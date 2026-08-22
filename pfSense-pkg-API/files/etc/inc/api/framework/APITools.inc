<?php
//   Copyright 2023 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

namespace APITools;
require_once("config.inc");
require_once("util.inc");
require_once("interfaces.inc");
require_once("interfaces_fast.inc");
require_once("priv.defs.inc");
require_once("priv.inc");
require_once("service-utils.inc");
require_once("filter.inc");
require_once("shaper.inc");
require_once("auth.inc");
require_once("functions.inc");
require_once("openvpn.inc");
require_once("certs.inc");
require_once("pkg-utils.inc");
require_once("firewall_virtual_ip.inc");
require_once("firebase/php-jwt/src/JWT.php");
require_once("firebase/php-jwt/src/JWK.php");
require_once("firebase/php-jwt/src/Key.php");
require_once("firebase/php-jwt/src/CachedKeySet.php");
require_once("firebase/php-jwt/src/JWTExceptionWithPayloadInterface.php");
require_once("firebase/php-jwt/src/ExpiredException.php");
require_once("firebase/php-jwt/src/SignatureInvalidException.php");
require_once("firebase/php-jwt/src/BeforeValidException.php");
use Firebase\JWT\JWT;
use Firebase\JWT\Key;

# Checks our content type header and parses the content accordingly
function get_request_data() {
    # TODO: x-www-form-urlencoded still attempts to use json if no parameters were passed in. This was intentionally done to
    # TODO: support user scripts that didn't specify a static content type (before it was supported) remove this in a future
    # TODO: release. It is preferred that content uses a specified content type.
    $_SERVER["HTTP_CONTENT_TYPE"] = (empty($_SERVER["HTTP_CONTENT_TYPE"])) ? "application/x-www-form-urlencoded" : $_SERVER["HTTP_CONTENT_TYPE"];
    $content_types = [
        "application/json" => json_decode(file_get_contents('php://input'), true),
        "application/x-www-form-urlencoded" => (empty($_GET)) ? json_decode(file_get_contents('php://input'), true) : form_decode($_GET)
    ];

    # Check if content type is supported, if so return corresponding parsed request data
    if (array_key_exists($_SERVER["HTTP_CONTENT_TYPE"], $content_types)) {
        return $content_types[$_SERVER["HTTP_CONTENT_TYPE"]];
    }
    # Return false if the content type is unknown or unsupported
    else {
        return false;
    }
}

# Decode query string into their appropriate data types
function form_decode($array) {
    # Loop through each query string item and check for expected data types
    foreach ($array as $key=>$value) {
        # Check for boolean type query strings
        if ($value === "true") {
            $array[$key] = true;
        }
        elseif ($value === "false") {
            $array[$key] = false;
        }

        # Check for integer type query strings
        if (is_numeric($value)) {
            $array[$key] = intval($value);
        }
    }

    return $array;
}

# Check our local pfSense version
function get_pfsense_version() {
    # VARIABLES
    $ver_path = "/etc/version";
    $ver_patch_path = "/etc/version.patch";
    $ver_bt_path = "/etc/version.buildtime";
    $ver_lc_path = "/etc/version.lastcommit";
    $ver_data = array();

    # Ensure each file exists, then read it's contents and save it to our version data.
    if (file_exists($ver_path)) {
        # Read full release version
        $ver_file = fopen($ver_path, "r");
        $ver = str_replace(PHP_EOL, "", fread($ver_file, filesize($ver_path)));
        $ver_data["version"] = $ver;

        # Discard everything but the version tag (e.g. 2.6.0)
        $ver = explode("-", $ver)[0];

        # Separate each version category into an array and piece together the major and minor categories as the base
        $ver = explode(".", $ver);
        $major_ver = $ver[0];
        $minor_ver = $ver[1];
        $ver_data["base"] = $major_ver.".".$minor_ver;
    }
    if (file_exists($ver_patch_path)) {
        $ver_patch_file = fopen($ver_patch_path, "r");
        $ver_patch = str_replace(PHP_EOL, "", fread($ver_patch_file, filesize($ver_patch_path)));
        $ver_data["patch"] = $ver_patch;
    }
    if (file_exists($ver_bt_path)) {
        $ver_bt_file = fopen($ver_bt_path, "r");
        $ver_bt = str_replace(PHP_EOL, "", fread($ver_bt_file, filesize($ver_bt_path)));
        $ver_data["buildtime"] = $ver_bt;
    }
    if (file_exists($ver_lc_path)) {
        $ver_lc_file = fopen($ver_lc_path, "r");
        $ver_lc = str_replace(PHP_EOL, "", fread($ver_lc_file, filesize($ver_lc_path)));
        $ver_data["lastcommit"] = $ver_lc;
    }

    # Format the version into a comparable numeric value
    $ver_data["program"] = floatval(str_replace(".", "", explode("-", $ver_data["version"])[0]).".".$ver_patch);

    return $ver_data;
}

# Locates our API configuration from pfSense's XML configuration. Returns
function get_api_config() {
    $api_pkg_name = "API";
    $pkg_conf = config_get_path("installedpackages/package");
    // Check that our configuration is an array
    if (is_array($pkg_conf)) {
        // Loop through our packages and find our API package config
        foreach ($pkg_conf as $id => $pkg) {
            if ($pkg["name"] === $api_pkg_name) {
                return array($id, $pkg["conf"]);
            }
        }
    }
}

# Checks if a specified user is disabled
function is_user_disabled($username) {
    $users = index_users();
    $user = $users[$username];
    if (config_path_enabled("system/user/{$user}", "disabled")) {
        return true;
    }
    return false;
}

# Creates JWT server key if one does not exist, or optionally allows rotation of the JWT server key
function create_jwt_server_key($rotate=false) {
    $pkg_index = get_api_config()[0];    // Save our current API configs pkg index
    $api_config = get_api_config()[1];    // Save our current API config
    # Create a new server key if one is not set
    if (empty($api_config["server_key"]) or $rotate === true) {
        config_set_path("installedpackages/package/{$pkg_index}/conf/server_key", bin2hex(random_bytes(32)));
        write_config("API server key created");
    }
}

# Creates a JWT to use for JWT authentication
function create_jwt($data) {
    # Pull the API configuration and extract the jwt expiration value, then ensure a server key and payload exists.
    $api_config = get_api_config()[1];
    $token_exp = $api_config["jwt_exp"];
    create_jwt_server_key();
    $payload = array(
        "iss" => config_get_path("system/hostname"),
        "aud" => config_get_path("system/hostname"),
        "exp" => time() + $token_exp,
        "nbf" => time(),
        "data" => $data
    );

    # Return the encoded JWT
    return JWT::encode($payload, $api_config["server_key"], "HS256");
}

# Decodes a JWT using our store server key
function decode_jwt($token) {
    $token = (is_string($token)) ? $token : "";
    $key = get_api_config()[1]["server_key"];
    try {
        $decoded = (array) JWT::decode($token, new Key($key, "HS256"));
    } catch (\Exception $e) {
        $decoded = false;
    }
    return $decoded;
}

# Get our API tokens for a given username
function get_existing_tokens($username) {
    // Local variables
    $api_config = get_api_config()[1];
    $key_user = bin2hex($username);    // Save our user's dedicated API client-ID
    $user_keys = [];

    # Only loop through if we have configured keys
    if (is_array($api_config["keys"]) and is_array($api_config["keys"]["key"])) {
        foreach ($api_config["keys"]["key"] as $id => $key) {
            if ($key["client_id"] === $key_user) {
                $user_keys[$id] = array("client_token" => $key["client_token"], "algo" => $key["algo"]);
            }
        }
    }

    return $user_keys;
}

# Authenticate using an API token
function authenticate_token($cid, $ctoken) {
    $authenticated = false;
    $hex_to_user = pack("H*", $cid);

    // First check if our hex decoded user exists
    if (array_key_exists($hex_to_user, index_users())) {
        // Loop through each of our users API tokens and check if key matches
        foreach (get_existing_tokens($hex_to_user) as $id => $data) {
            $hash_input_key = hash($data["algo"], $ctoken);    // Hash our key using our configured algos
            if ($hash_input_key === $data["client_token"]) {
                $authenticated = true;
                break;
            }
        }
    }
    return $authenticated;
}

// Generate new API tokens for token auth mode
function generate_token($username) {
    // Local variables
    $pkg_index = get_api_config()[0];    // Save our current API configs pkg index
    $api_config = get_api_config()[1];    // Save our current API config
    $key_hash_algo = $api_config["keyhash"];    // Pull our configured key hash algorithm
    $key_bit_strength = $api_config["keybytes"];    // Pull our configured key bit strength
    $key_user = bin2hex($username);    // Save our user's dedicated API client-ID
    $key_new = bin2hex(random_bytes(intval($key_bit_strength)));    // Generate a new key
    $key_hash = hash($key_hash_algo, $key_new);    // Hash our key using our configured algos

    // Loop through our existing keys to see
    $api_config["keys"] = !is_array($api_config["keys"]) ? array("key" => []) : $api_config["keys"];
    $api_config["keys"]["key"][] = array("client_id" => $key_user, "client_token" => $key_hash, "algo" => $key_hash_algo);

    // Write our changes
    config_set_path("installedpackages/package/{$pkg_index}/conf", $api_config);
    write_config(sprintf(gettext(" Generated API key")));    // Apply our configuration change
    return $key_new;
}

// Check if CARP is enabled for disabled
function is_carp_enabled() {
    // Check current CARP status
    $status = get_single_sysctl('net.inet.carp.allow');
    $enabled = boolval(intval($status) > 0);
    return $enabled;
}

// Check each CARP interface's status
function get_carp_if_status() {
    // Local variables
    $carp_if_stats = [];
    $carp_enabled = is_carp_enabled();
    foreach (config_get_path('virtualip/vip', []) as $carp) {
        if ($carp['mode'] == "carp") {
            $carp_if_ent = [];
            $carp_if_ent["interface"] = $carp["interface"];
            $carp_if_ent["vhid"] = $carp['vhid'];
            $carp_if_ent["subnet"] = $carp['subnet'];
            $carp_if_ent["subnet_bits"] = $carp['subnet_bits'];
            $status = get_carp_interface_status("_vip{$carp['uniqid']}");
            if ($carp_enabled == false) {
                $carp_if_ent["status"] = "disabled";
            } else {
                if ($status == "MASTER") {
                    $carp_if_ent["status"] = "master";
                } else if ($status == "BACKUP") {
                    $carp_if_ent["status"] = "backup";
                } else if ($status == "INIT") {
                    $carp_if_ent["status"] = "init";
                }
            }
            // Add config to our array
            $carp_if_stats[] = $carp_if_ent;
        }
    }
    // Return our status
    return $carp_if_stats;
}

// Enables CARP interfaces
function enable_carp($enable) {
    // Local variables
    $vip_arr = config_get_path('virtualip/vip');
    $no_action = (is_carp_enabled() === $enable) ? true : false;    // Check if a change is even requried
    // Disable if $enable is false, enable if $enable is true
    if (!$no_action and $enable === false) {
        set_single_sysctl('net.inet.carp.allow', '0');
        foreach ($vip_arr as $vip) {
            if ($vip['mode'] != "carp" && $vip['mode'] != "ipalias")
                continue;
            if ($vip['mode'] == "ipalias" && substr($vip['interface'], 0, 4) != "_vip")
                continue;
            interface_vip_bring_down($vip);
        }
    } elseif (!$no_action and $enable === true) {
        foreach ($vip_arr as $vip) {
            switch ($vip['mode']) {
                case "carp":
                    interface_carp_configure($vip);
                    break;
                case 'ipalias':
                    if (substr($vip['interface'], 0, 4) == "_vip") {
                        interface_ipalias_configure($vip);
                    }
                    break;
            }
        }
        interfaces_sync_setup();
        set_single_sysctl('net.inet.carp.allow', '1');
    }
}

# Sorts NAT rules by specified criteria
function sort_nat_rules($top=false, $data=null, $field=null) {
    // Variables
    $sort_arr = [];

    # Set appropriate config location depending on specified field
    switch($field) {
        case "onetoone":
            $acl =& config_get_path("nat/onetoone", []);
            break;
        case "outbound":
            $acl =& config_get_path("nat/outbound/rule", []);
            break;
        default:
            $acl =& config_get_path("nat/rule", []);
    }

    foreach ($acl as $idx => $fre) {
        # Check if top mode is enabled, if so add this item to the start of the array
        if ($top == true and $idx === $data) {
            array_unshift($sort_arr, $fre);
        } else {
            $sort_arr[] = $fre;
        }
    }


    switch($field) {
        case "onetoone":
            config_set_path("nat/onetoone", $sort_arr);
            break;
        case "outbound":
            config_set_path("nat/outbound/rule", $sort_arr);
            break;
        default:
            config_set_path("nat/rule", $sort_arr);
    }

    if (!empty($sort_arr)) {
        $acl = $sort_arr;
    } else {
        unset($acl);
    }
}

# Input a physical interface ID, or a descriptive interface name, and return the pfSense interface ID (lan,wan,optx)
function get_pfsense_if_id($interface, $include_carp=false, $include_ifgroup=false, $include_ipsec=false, $include_openvpn=false) {
    # Variables
    # Loop through our config and check each interface for a physical ID match
    foreach (config_get_path("interfaces", []) as $if_id => $if_ent) {
        # First check if the requested interface matches this physical interface ID
        if (array_key_exists("if", $if_ent)) {
            // Check if the interface id matches
            if ($interface === $if_ent["if"]) {
                return $if_id;
            }
        }
        # Next, check if the requested interface matches this pfSense interface description
        if (array_key_exists("descr", $if_ent)) {
            // Check if the interface descr matches
            if ($interface === $if_ent["descr"]) {
                return $if_id;
            }
        }
        # Alternatively, check if the interface does not have a description. If so, check it's default description.
        elseif (strtoupper($if_id) === $interface) {
            return $if_id;
        }
        # Next, check if the requested interface matches this pfSense interface ID (wan, lan, opt1, opt2, etc.)
        if ($interface === $if_id) {
            return $if_id;
        }
    }

    # Only include CARP interfaces if they are explicitly requested
    if ($include_carp and is_vip($interface, "carp")) {
        return $interface;
    }

    # Only include interface groups if they are explicitly requested
    if($include_ifgroup and is_ifgroup($interface)) {
        return $interface;
    }
    
    # Only include the default IPsec interface (enc0) if explicitly requested
    if ($include_ipsec and $interface === "enc0") {
        return $interface;
    }

    # Only include the OpenVPN interface if explicitly requested and OpenVPN is configured
    if ($include_openvpn and $interface === "openvpn" and config_get_path("openvpn", false)) {
        return $interface;
    }
}

# Check if a string is a virtual IPs unique ID
function is_vip($value, $vip_type="all") {
    # Loop through each configured virtual IP
    foreach (config_get_path('virtualip/vip', []) as $vip) {
        if ($vip['mode'] == $vip_type or $vip_type === "all") {
            if ($value === '_vip'.$vip['uniqid']) {
                return $value;
            }
        }
    }
}

# Checks if a string is an existing certificate's ref ID
function is_certificate($refid) {
    # Loop through each configured certificate and check if this value matches its refid
    foreach (config_get_path("cert", []) as $cert) {
        if ($cert["refid"] === $refid) {
            return true;
        }
    }
    return false;
}

# Checks if a string is an existing ca's ref ID
function is_ca($refid) {
    # Loop through each configured ca and check if this value matches its refid
    foreach (config_get_path("ca", []) as $ca) {
        if ($ca["refid"] === $refid) {
            return true;
        }
    }
    return false;
}

# Check if input is valid for rule source and destination
# TODO: this function is messy, clean it up
function is_valid_rule_addr($addr, $direction) {
    // Variables
    $addr_types = array("any", "pppoe", "l2tp", "(self)");   // Array of special src/dst types
    $ret_val = array("valid" => true, "data" => array());
    // Check if our source values are valid
    if (is_string($addr)) {
        // Check src/dst is negated
        if (str_starts_with("!", $addr)) {
            $addr_not = true;
            $addr = str_replace("!", "", $addr);
        }

        // Check if our data is valid
        $addr_if = str_replace("ip", "", $addr);

        if (is_ipaddr($addr) or is_subnet($addr)) {
            $ret_val["data"] = array($direction => array("address" => $addr));
        } elseif (is_alias($addr)) {
            $ret_val["data"] = array($direction => array("address" => $addr));
        } elseif (get_pfsense_if_id($addr_if)) {
            $addr_pfif = get_pfsense_if_id($addr_if);    // Save our interface pfid

            // If source was interface address (ending in ip), otherwise assume entire subnet
            if (str_replace($addr_if, "", $addr) === "ip") {
                $ret_val["data"] = array($direction => array("network" => $addr_pfif . "ip"));
            } else {
                $ret_val["data"] = array($direction => array("network" => $addr_pfif));
            }
        } elseif (in_array($addr, $addr_types)) {
            # Format config for any address
            if ($addr === "any") {
                $ret_val["data"] = array($direction => array("any" => ""));
            }
            # Do not allow (self) address if direction is source
            elseif ($addr === "(self)" and $direction === "source") {
                $ret_val["valid"] = false;
            }
            # Otherwise, Format config as network
            else {
                $ret_val["data"] = array($direction => array("network" => $addr));
            }
        } else {
            $ret_val["valid"] = false;
        }
        // If source is negated, add not to our array
        if ($addr_not) {
            $ret_val["data"][$direction]["not"] = "";
        }
    } else {
        $ret_val["valid"] = false;
    }
    return $ret_val;
}

// Checks if a given string starts with another given string
function str_starts_with($needle, $haystack) {
    $length = strlen($needle);
    return (substr($haystack, 0, $length) === $needle);
}

function str_ends_with($needle, $haystack) {
    # Variables
    $length = strlen($needle);

    # If the needle is empty, match everything.
    if( !$length ) {
        return true;
    }

    # Check if the end of the string matches the needle
    return substr($haystack, -$length) === $needle;
}

// Find an available virtual IP vhid
function next_vhid() {
    $vhid_config = config_get_path("virtualip/vip", []);
    $ret_vhid = null;
    # Loop through our range of valid VHID ids
    foreach (range(1, 255) as $idx) {
        $vhid_exists = false;
        # Loop through our virutal IPs and check if this VHID already exists
        foreach ($vhid_config as $vip) {
            if (intval($vip["vhid"]) === intval($idx)) {
                $vhid_exists = true;
            }
        }
        // Check if our VHID was already used
        if (!$vhid_exists) {
            $ret_vhid = $idx;
            break;
        }
    }
    return $ret_vhid;
}

// Checks if an IP is in use elsewhere in our configuration
function is_ip_in_use($ip) {
    $vip_conf = config_get_path("virtualip/vip", []);
    $if_conf = config_get_path("interfaces", []);
    // Check if IP is used in our virtual IP configuration
    foreach ($vip_conf as $vip) {
        if ($ip === $vip["subnet"]) {
            return true;
        }
    }
    // Check if IP is used in our interface configuration
    foreach ($if_conf as $iface) {
        if ($ip === $iface["ipaddr"]) {
            return true;
        } elseif ($ip === $iface["ipaddrv6"]) {
            return true;
        }
    }
    return false;
}

# Checks if routing gateway exists
function is_gateway($gw, $ret_protocol=false) {
    # Local variables
    $gw_config = return_gateways_array();
    $gw_group_config = return_gateway_groups_array();

    # Loop through each gateway/gateway group and check if our gw matches the name
    if (is_array($gw_config)) {
        # Loop through each gateway and see if name matches
        foreach ($gw_config as $gw_name => $gw_item) {
            if ($gw === $gw_name) {
                return ($ret_protocol) ? $gw_item["ipprotocol"] : true;
            }
        }
        # Loop through each gateway and see if name matches
        foreach ($gw_group_config as $gwg_name => $gwg_item) {
            if ($gw === $gwg_name) {
                return ($ret_protocol) ? $gwg_item["ipprotocol"] : true;
            }
        }
    }
    return false;
}

# Checks if a specified gateway ID is in use
function is_gateway_in_use($id) {
    # Local variables
    $gws = config_get_path("gateways/gateway_item", []);

    # Only check if our ID is numeric
    if (is_numeric($id)) {
        # Check if gateways groups are using this gateway
        foreach (config_get_path('gateways/gateway_group', []) as $group) {
            foreach ($group['item'] as $item) {
                $items = explode("|", $item);
                if ($items[0] == $gws[$id]['name']) {
                    return true;
                }
            }
        }

        # Check if static routes are using this gateway
        foreach (config_get_path('staticroutes/route', []) as $route) {
            if ($route['gateway'] == $gws[$id]['name']) {
                return true;
            }
        }

        # Check if system DNS servers are using this gateway
        foreach (config_get_path('system/dnsserver', []) as $id_dnsgw => $dnsserver) {
            if (config_path_enabled("system", "dns{$id_dnsgw}gw")) {
                if ($gws[$id]['name'] == config_get_path("system/dns{$id_dnsgw}gw")) {
                    return true;
                }
            }
        }
    }

    return false;
}

// Duplicate function from /firewall_aliases.php: accept input and check if alias exists
function alias_find_references($section, $field, $origname, &$is_alias_referenced, &$referenced_by) {
    if (!$origname || $is_alias_referenced) {
        return;
    }
    $sectionref = config_get_path("/");
    foreach ($section as $sectionname) {
        if (is_array($sectionref) && isset($sectionref[$sectionname])) {
            $sectionref = &$sectionref[$sectionname];
        } else {
            return;
        }
    }
    if (is_array($sectionref)) {
        foreach ($sectionref as $itemkey => $item) {
            $fieldfound = true;
            $fieldref = &$sectionref[$itemkey];
            foreach ($field as $fieldname) {
                if (is_array($fieldref) && isset($fieldref[$fieldname])) {
                    $fieldref = &$fieldref[$fieldname];
                } else {
                    $fieldfound = false;
                    break;
                }
            }
            if ($fieldfound && $fieldref == $origname) {
                $is_alias_referenced = true;
                if (is_array($item)) {
                    $referenced_by = $item['descr'];
                }
                break;
            }
        }
    }
}

// Input an alias name and check if the alias exists
function alias_in_use($alias_name) {
    $is_alias_referenced = false;
    $referenced_by = false;
    // Firewall rules
    alias_find_references(array('filter', 'rule'), array('source', 'address'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('filter', 'rule'), array('destination', 'address'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('filter', 'rule'), array('source', 'port'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('filter', 'rule'), array('destination', 'port'), $alias_name, $is_alias_referenced, $referenced_by);
    // NAT Rules
    alias_find_references(array('nat', 'rule'), array('source', 'address'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'rule'), array('source', 'port'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'rule'), array('destination', 'address'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'rule'), array('destination', 'port'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'rule'), array('target'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'rule'), array('local-port'), $alias_name, $is_alias_referenced, $referenced_by);
    // NAT 1:1 Rules
    alias_find_references(array('nat', 'onetoone'), array('destination', 'address'), $alias_name, $is_alias_referenced, $referenced_by);
    // NAT Outbound Rules
    alias_find_references(array('nat', 'outbound', 'rule'), array('source', 'network'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'outbound', 'rule'), array('sourceport'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'outbound', 'rule'), array('destination', 'address'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'outbound', 'rule'), array('dstport'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('nat', 'outbound', 'rule'), array('target'), $alias_name, $is_alias_referenced, $referenced_by);
    // Alias in an alias
    alias_find_references(array('aliases', 'alias'), array('address'), $alias_name, $is_alias_referenced, $referenced_by);
    // Load Balancer
    alias_find_references(array('load_balancer', 'lbpool'), array('port'), $alias_name, $is_alias_referenced, $referenced_by);
    alias_find_references(array('load_balancer', 'virtual_server'), array('port'), $alias_name, $is_alias_referenced, $referenced_by);
    // Static routes
    alias_find_references(array('staticroutes', 'route'), array('network'), $alias_name, $is_alias_referenced, $referenced_by);
    return $is_alias_referenced;
}

// Strip special characters and replace whitespace with underscore
function sanitize_str($string) {
    $string = str_replace(' ', '_', $string);    // Replace whitespace with underscore
    $string = preg_replace('/[^A-Za-z0-9\-_.]/', '', $string);    // Remove special characters
    return $string;
}

// Restarts the pfSense webConfigurator
function restart_webconfigurator() {
    ob_flush();
    flush();
    log_error(gettext("webConfigurator configuration has changed. Restarting webConfigurator."));
    send_event("service restart webgui");
}

// Parse our ARP table into an array
function get_arp_table() {
    // Local variables
    $arp_cmd = "arp -an";    // Assign the command which reads our ARP table
    exec($arp_cmd, $arp_data);    // Output our ARP table into a string
    $arp_table = array();    // Init our ARP table array
    // Loop through each line of our ARP data and parse into our array
    foreach ($arp_data as $arp_line) {
        $elements = explode(' ', $arp_line, 7);
        $arp_entry = array();
        $arp_entry['ip'] = trim(str_replace(array('(', ')'), '', $elements[1]));
        $arp_entry['mac'] = trim($elements[3]);
        $arp_entry['interface'] = trim($elements[5]);
        $arp_entry['status'] = trim(substr($elements[6], 0, strrpos($elements[6], ' ')));
        $arp_entry['linktype'] = trim(str_replace(array('[', ']'), '', strrchr($elements[6], ' ')));
        $arp_table[] = $arp_entry;
    }
    return $arp_table;
}

// Pull a single ARP entry value from our ARP table
function get_arp_entry($search, $value) {
    // Local variables
    $arp_table = get_arp_table();   // Pull our ARP table
    $arp_match = [];    // Init our match array
    // Loop through ARP table and look for matches
    foreach ($arp_table as $arp_ent) {
        if ($arp_ent[$search] === $value) {
            $arp_match = $arp_ent;
        }
    }
    return $arp_match;
}

// Checks if an authentication server exists by name
function is_authentication_server($name) {
    foreach (config_get_path("system/authserver", []) as $as) {
        $reserved_names = [$as["name"], "Local_Database", "local", "LOCAL", "Local"];
        if (in_array($name, $reserved_names)) {
            return true;
        }
    }
    return false;
}

// Reload our unbound configuration, restart associated services and clear config locks
function unbound_reload_config() {
    $reload_unbound = 0;
    $reload_unbound |= services_unbound_configure();
    // Check if application was successful
    if ($reload_unbound === 0) {
        system_resolvconf_generate();    // Update resolveconf
        system_dhcpleases_configure();    // Update DHCPD
        clear_subsystem_dirty("unbound");
        return true;
    }
}


// Get a complete config list of ALL interfaces. Based off interfaces_assign.php
function get_all_avail_interfaces() {
    // Local variables
    global $ipsec_descrs;
    $base_ifs = get_interface_list();
    
    # Include wireless clone interfaces 
    if (config_get_path('wireless/clone') && is_array(config_get_path('wireless/clone')) && count(config_get_path('wireless/clone'))) {
        foreach (config_get_path('wireless/clone', []) as $clone) {
            $base_ifs[$clone['cloneif']] = $clone;
            $base_ifs[$clone['cloneif']]['iswlclone'] = true;
        }
    }

    # Include VLAN interfaces 
    if (config_get_path('vlans/vlan') && is_array(config_get_path('vlans/vlan')) && count(config_get_path('vlans/vlan'))) {
        foreach (config_get_path('vlans/vlan', []) as $vlan) {
            $base_ifs[$vlan['vlanif']] = $vlan;
            $base_ifs[$vlan['vlanif']]['isvlan'] = true;
        }
    }

    # Include Bridge interfaces 
    if (config_get_path('bridges/bridged') && is_array(config_get_path('bridges/bridged')) && count(config_get_path('bridges/bridged'))) {
        foreach (config_get_path('bridges/bridged', []) as $bridge) {
            $base_ifs[$bridge['bridgeif']] = $bridge;
            $base_ifs[$bridge['bridgeif']]['isbridge'] = true;
        }
    }

    # Include GIF interfaces 
    if (config_get_path('gifs/gif') && is_array(config_get_path('gifs/gif')) && count(config_get_path('gifs/gif'))) {
        foreach (config_get_path('gifs/gif', []) as $gif) {
            $base_ifs[$gif['gifif']] = $gif;
            $base_ifs[$gif['gifif']]['isgif'] = true;
        }
    }

    # Include GRE interfaces 
    if (config_get_path('gres/gre') && is_array(config_get_path('gres/gre')) && count(config_get_path('gres/gre'))) {
        foreach (config_get_path('gres/gre', []) as $gre) {
            $base_ifs[$gre['greif']] = $gre;
            $base_ifs[$gre['greif']]['isgre'] = true;
        }
    }

    # Include LAGG interfaces 
    if (config_get_path('laggs/lagg') && is_array(config_get_path('laggs/lagg')) && count(config_get_path('laggs/lagg'))) {
        foreach (config_get_path('laggs/lagg', []) as $lagg) {
            $base_ifs[$lagg['laggif']] = $lagg;
            $base_ifs[$lagg['laggif']]['islagg'] = true;
            # LAGG members cannot be assigned 
            $lagifs = explode(',', $lagg['members']);
            foreach ($lagifs as $lagif) {
                if (isset($base_ifs[$lagif])) {
                    unset($base_ifs[$lagif]);
                }
            }
        }
    }

    # Include QinQ interfaces 
    if (config_get_path('qinqs/qinqentry') && is_array(config_get_path('qinqs/qinqentry')) && count(config_get_path('qinqs/qinqentry'))) {
        foreach (config_get_path('qinqs/qinqentry', []) as $qinq) {
            $base_ifs["{$qinq['vlanif']}"]['descr'] = "VLAN {$qinq['tag']} on {$qinq['if']}";
            $base_ifs["{$qinq['vlanif']}"]['isqinq'] = true;
            # QinQ members 
            $qinqifs = explode(' ', $qinq['members']);
            foreach ($qinqifs as $qinqif) {
                $base_ifs["{$qinq['vlanif']}.{$qinqif}"]['descr'] = "QinQ {$qinqif} on VLAN {$qinq['tag']} on {$qinq['if']}";
                $base_ifs["{$qinq['vlanif']}.{$qinqif}"]['isqinq'] = true;
            }
        }
    }

    # Include PPP interfaces 
    if (config_get_path('ppps/ppp') && is_array(config_get_path('ppps/ppp')) && count(config_get_path('ppps/ppp'))) {
        foreach (config_get_path('ppps/ppp', []) as $pppid => $ppp) {
            $portname = $ppp['if'];
            $base_ifs[$portname] = $ppp;
            $base_ifs[$portname]['isppp'] = true;
            $ports_base = basename($ppp['ports']);
            if (isset($ppp['descr'])) {
                $base_ifs[$portname]['descr'] = strtoupper($ppp['if']). "({$ports_base}) - {$ppp['descr']}";
            } else if (isset($ppp['username'])) {
                $base_ifs[$portname]['descr'] = strtoupper($ppp['if']). "({$ports_base}) - {$ppp['username']}";
            } else {
                $base_ifs[$portname]['descr'] = strtoupper($ppp['if']). "({$ports_base})";
            }
        }
    }

    # Include OpenVPN interfaces
    $ovpn_descrs = array();
    if (config_get_path('openvpn') && is_array(config_get_path('openvpn'))) {
        if (config_get_path('openvpn/openvpn-server') && is_array(config_get_path('openvpn/openvpn-server'))) {
            foreach (config_get_path('openvpn/openvpn-server', []) as $s) {
                $portname = "ovpns{$s['vpnid']}";
                $base_ifs[$portname] = $s;
                $ovpn_descrs[$s['vpnid']] = $s['description'];
            }
        }
        if (config_get_path('openvpn/openvpn-client') && is_array(config_get_path('openvpn/openvpn-client'))) {
            foreach (config_get_path('openvpn/openvpn-client', []) as $c) {
                $portname = "ovpnc{$c['vpnid']}";
                $base_ifs[$portname] = $c;
                $ovpn_descrs[$c['vpnid']] = $c['description'];
            }
        }
    }

    # Include IPsec interfaces
    $ipsec_descrs = interface_ipsec_vti_list_all();
    foreach ($ipsec_descrs as $ifname => $ifdescr) {
        $base_ifs[$ifname] = array('descr' => $ifdescr);
    }

    # Loop through our array and check if interface is in use
    foreach ($base_ifs as $pid => $conf) {
        # Check if there is a configuration for this interface
        $pf_id = get_pfsense_if_id($pid);

        # Check if the pfSense interface ID was found
        if (get_pfsense_if_id($pid)) {
            $base_ifs[$pid]["in_use"] = $pf_id;
        }
    }

    return $base_ifs;
}

// Get available media options for a given interface. Modified from interfaces.php
function get_if_media_options($interface, $physical_if=false) {
    // Local variables
    $interface = ($physical_if !== true) ? get_pfsense_if_id($interface) : $interface;
    $interface = ($physical_if !== true) ? config_get_path("interfaces/{$interface}/if") : $interface;
    $mediaopts_list = array();
    exec("/sbin/ifconfig -m $interface | grep \"media \"", $mediaopts);
    foreach ($mediaopts as $mediaopt) {
        preg_match("/media (.*)/", $mediaopt, $matches);
        if (preg_match("/(.*) mediaopt (.*)/", $matches[1], $matches1)) {
            // there is media + mediaopt like "media 1000baseT mediaopt full-duplex"
            array_push($mediaopts_list, $matches1[1] . " " . $matches1[2]);
        } else {
            // there is only media like "media 1000baseT"
            array_push($mediaopts_list, $matches[1]);
        }
    }
    return $mediaopts_list;
}

// Get our next available pfSense interface ID
function get_next_pfsense_if_id() {
    // Local variables
    $curr_ifs = config_get_path("interfaces", []);
    // Check if we have our `wan` or `lan` pf IDs in use, if so, find the next OPTx id
    if (!array_key_exists("wan", $curr_ifs)) {
        return "wan";
    } elseif (!array_key_exists("lan", $curr_ifs)) {
        return "lan";
    } else {
        // Loop until we find an unused OPTx interface
        foreach (range(1, 2000) as $count) {
            // Check if this OPTx ID exists
            $optx = "opt".strval($count);
            if (!array_key_exists($optx, $curr_ifs)) {
                return $optx;
            }
        }
    }
}

// Returns a list of dynamically configured IPv6 interfaces. Modified from interfaces.php function.
function get_ipv6_if_list() {
    $interfaces = get_configured_interface_with_descr(true);
    $dyn_v6_ifs = array();
    foreach ($interfaces as $iface => $ifacename) {
        switch (config_get_path("interfaces/{$iface}/ipaddrv6")) {
            case "6to4":
            case "6rd":
            case "dhcp6":
                $dyn_v6_ifs[$iface] = array(
                    'name' => $ifacename,
                    'ipv6_num_prefix_ids' => pow(2, (int) calculate_ipv6_delegation_length($iface)) - 1);
                break;
            default:
                continue 2;
        }
    }
    return($dyn_v6_ifs);
}

# Converts an integer-of-100 percent to a decimal percentage
function float_percent($value) {
    # Only return converted value if it is numeric
    if (is_numeric($value)) {
        # Add a leading 0 if number is single digit
        if (floatval($value) > 0 and floatval($value) < 10) {
            $value = "0" . strval($value);
        }
        return floatval("0." . $value);
    }
}

# Checks if an interface group with a specified name already exists. Returns true or false.
function is_ifgroup($name) {
    foreach (config_get_path('ifgroups/ifgroupentry', []) as $if_group_ent) {
        if ($if_group_ent["ifname"] === $name) {
            return true;
        }
    }
    return false;
}

# Checks if an interface has DHCPD enabled. Returns true or false.
function is_dhcpd_enabled($if) {
    # Ensure our interface is in it's pfSense ID format
    $if = get_pfsense_if_id($if);

    # Check if this interface is enabled
    if (config_path_enabled("dhcpd/{$if}")) {
        return true;
    }

    return false;
}

# Checks if an interface has DHCPDv6 enabled. Returns true or false.
function is_dhcpdv6_enabled($if) {
    # Ensure our interface is in it's pfSense ID format
    $if = get_pfsense_if_id($if);

    # Check if this interface is enabled
    if (config_path_enabled("dhcpdv6/{$if}")) {
        return true;
    }

    return false;
}

# Checks if a value is an IPv4 address, subnet, or alias
function is_ip_subnet_or_alias($value) {
    if (is_subnet($value) === 4) {
        return "ipv4_subnet";
    } elseif (is_ipaddrv4($value)) {
        return "ipv4_addr";
    } elseif (is_ipaddroralias($value)) {
        return "alias";
    } else {
        return false;
    }
}

# Checks if a given IPv4 address is within a given CIDR
function is_ipv4_in_cidr($ip, $cidr) {
    # Use IP2Long to determine if a specified IP is within a specified cidr
    $subnet = explode('/', $cidr)[0];
    $subnet = ip2long($subnet);
    $bits = (!is_null(explode('/', $cidr)[1])) ? explode('/', $cidr)[1] : 32;
    $ip = ip2long($ip);
    $mask = -1 << (32 - $bits);
    $subnet &= $mask;
    return ($ip & $mask) == $subnet;
}

# Checks if a given IPv6 address is within a given CIDR
# Note: the CIDR must use the CIDRs network address to work
function is_ipv6_in_cidr($ip, $cidr) {
    # Local variables
    $subnet = inet_pton(explode('/', $cidr)[0]);
    $mask = (!is_null(explode('/', $cidr)[1])) ? explode('/', $cidr)[1] : 128;
    $mask_bits = str_repeat("f", $mask / 4);

    # Calculate the addr based on the mask
    if ($mask % 4 == 1) {
        $mask_bits .= "8";
    }
    elseif ($mask % 4 == 2) {
        $mask_bits .= "c";
    }
    elseif ($mask % 4 == 3) {
        $mask_bits .= "e";
    }

    # Format our address and check if it is within our CIDR
    $mask_bits = str_pad($mask_bits, 32, '0');
    $mask_bits = pack("H*" , $mask_bits);
    return (inet_pton($ip) & $mask_bits) == $subnet;
}

# Check if a given IPv4 or IPv6 address is within a given CIDR
function is_ip_in_cidr($ip, $cidr) {
    # Run the CIDR function for the corresponding IP version
    if (is_ipaddrv4($ip) and is_subnetv4($cidr)) {
        return is_ipv4_in_cidr($ip, $cidr);
    }
    if (is_ipaddrv6($ip) and is_subnetv6($cidr)) {
        return is_ipv6_in_cidr($ip, $cidr);
    }

    # Default to false if no condition was met previously
    return false;
}

# Check if a specified IP is a configured virtual IP
function is_ip_vip($ip, $if=null) {
    # Loop through each configured virtual IP
    foreach (config_get_path("virtualip/vip", []) as $vip) {
        # Check if this VIPs interface matches the requested interface, or any if none was specified
        if ($vip["interface"] === $if or $if === null) {
            # Check if the specified IP is contained in this VIP
            if (is_ip_in_cidr($ip, $vip["subnet"]."/".$vip["subnet_bits"])) {
                return true;
            }
        }
    }
    return false;
}

# Gets the config ID of a IPsec phase 1 entry
function get_phase1_config_id_from_ikeid($ikeid) {
    # Loop through each IPsec phase1 entry and check for a match
    foreach (config_get_path("ipsec/phase1", []) as $index=>$p1) {
        # Check if this IKE ID matches
        if ((int)$ikeid === (int)$p1["ikeid"]) {
            return $index;
        }
    }

    return false;
}

# Gets the config ID of IPsec phase2 entry by it's uniqid
function get_phase2_config_id_from_uniqid($uniqid) {
    # Loop through each IPsec phase2 entry and check for a match
    foreach (config_get_path("ipsec/phase2", []) as $index=>$p2) {
        # Check if this IKE ID matches
        if ($uniqid === $p2["uniqid"]) {
            return $index;
        }
    }

    return false;
}

# Checks if an array is a associative array. Returns true if assoc, false if numeric, or null if not array
function is_assoc_array($array, $strict_seq=false) {
    # Local variables
    $i = 0;

    # First check if our array is actually an array. Return null otherwise.
    if (is_array($array)) {
        # Loop through each array value and check it's key
        foreach ($array as $key => $value) {
            # Check if key is numeric
            if (!is_numeric($key) or ($strict_seq and $i !== intval($key))) {
               return true;
            }
            $i++;
        }
    } else {
        return null;
    }

    return false;
}

function api_request($url, $method, $data=[], $headers=[], $username="", $password="") {
    # Format data and headers
    $data = json_encode($data);

    # Ensure headers is always an array
    if (!is_array($headers)) {
        $headers = [];
    }
    $headers["Content-Type"] = "application/json";
    $headers["Content-Length"] = strlen($data);

    # Setup cURL options
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_SSL_VERIFYHOST, 0);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
    curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, 10);
    curl_setopt($ch, CURLOPT_TIMEOUT, 10);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, 1);
    curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
    curl_setopt($ch, CURLOPT_CUSTOMREQUEST, strtoupper($method));
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data);

    # Only include basic authentication if username and password are set
    if ($username and $password) {
        curl_setopt($ch, CURLOPT_USERPWD, $username . ":" . $password);
    }

    # Make our request and return the response
    $result = curl_exec($ch);
    curl_close($ch);
    return $result;
}

function sync() {
    # Local variables
    $pkg_conf = get_api_config()[1];
    $protocol = config_get_path("system/webgui/protocol", "https");
    $port = config_get_path("system/webgui/port", "443");

    # Only perform sync when HA Sync is enabled for the API
    if (isset($pkg_conf["hasync"])) {
        # Loop through each HA sync node and try to sync the configuration
        foreach (explode(" ", $pkg_conf["hasync_hosts"]) as $node) {
            # Make the API request to sync configuration
            echo "Syncing API configuration to node ".$node."... ";
            $resp = api_request(
                $protocol."://".$node.":".$port."/api/v1/system/api/sync",
                "PUT",
                $pkg_conf,
                [],
                $pkg_conf["hasync_username"],
                $pkg_conf["hasync_password"]
            );

            # Ensure the sync was successful
            if ($resp) {
                $resp = json_decode($resp, true);
                # Check if our response was decoded correctly
                if ($resp) {
                    if ($resp["return"] === 0) {
                        echo "done." . PHP_EOL;
                    } elseif ($resp["return"] === 3) {
                        log_error("API sync failure (".$node."): Authentication failed");
                        echo "failed. Authentication failure.".PHP_EOL;
                    } elseif ($resp["return"] === 4) {
                        log_error("API sync failure (".$node."): Authorization failed");
                        echo "failed. Authorization failure.".PHP_EOL;
                    } else {
                        log_error("API sync failure (".$node."): Sync received return code ".strval($resp["return"]));
                        echo "failed. Received return code ".strval($resp["return"]).".".PHP_EOL;
                    }
                } else {
                    log_error("API sync failure (".$node."): Sync received unexpected response");
                    echo "failed. Received unexpected response.".PHP_EOL;
                }
            } else {
                log_error("API sync failure (".$node."): No response received from node");
                echo "failed. No response received from node.".PHP_EOL;
            }
        }
    } else {
        echo "Syncing API configuration... not configured.".PHP_EOL;
    }
}
