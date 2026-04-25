<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\SubnetValidator;

/**
 * Defines a Model that represents OpenVPN clients.
 */
class OpenVPNClient extends Model {
    const INTERFACE_PREFIX = 'ovpnc';
    const STATIC_KEY_PREFIX = '-----BEGIN OpenVPN Static key V1-----';
    const STATIC_KEY_SUFFIX = '-----END OpenVPN Static key V1-----';

    public IntegerField $vpnid;
    public StringField $vpnif;
    public StringField $description;
    public BooleanField $disable;
    public StringField $mode;
    public StringField $dev_mode;
    public StringField $protocol;
    public InterfaceField $interface;
    public StringField $server_addr;
    public PortField $server_port;
    public PortField $local_port;
    public StringField $proxy_addr;
    public PortField $proxy_port;
    public StringField $proxy_authtype;
    public StringField $proxy_user;
    public StringField $proxy_passwd;
    public StringField $auth_user;
    public StringField $auth_pass;
    public BooleanField $auth_retry_none;
    public Base64Field $tls;
    public StringField $tls_type;
    public StringField $tlsauth_keydir;
    public ForeignModelField $caref;
    public ForeignModelField $certref;
    public StringField $data_ciphers;
    public StringField $data_ciphers_fallback;
    public StringField $digest;
    public BooleanField $remote_cert_tls;
    public StringField $tunnel_network;
    public StringField $tunnel_networkv6;
    public StringField $remote_network;
    public StringField $remote_networkv6;
    public IntegerField $use_shaper;
    public StringField $allow_compression;
    public BooleanField $passtos;
    public BooleanField $route_no_pull;
    public BooleanField $route_no_exec;
    public BooleanField $dns_add;
    public StringField $topology;
    public IntegerField $inactive_seconds;
    public StringField $ping_method;
    public IntegerField $keepalive_interval;
    public IntegerField $keepalive_timeout;
    public IntegerField $ping_seconds;
    public StringField $ping_action;
    public IntegerField $ping_action_seconds;
    public StringField $custom_options;
    public BooleanField $udp_fast_io;
    public StringField $exit_notify;
    public IntegerField $sndrcvbuf;
    public StringField $create_gw;
    public IntegerField $verbosity_level;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = true;
        $this->config_path = 'openvpn/openvpn-client';
        $this->always_apply = true;
        $this->verbose_name = 'OpenVPN Client';

        # Set model fields
        $this->vpnid = new IntegerField(
            unique: true,
            default_callable: 'get_next_vpn_id',
            editable: false,
            read_only: true,
            verbose_name: 'VPN ID',
            help_text: 'The unique ID for this OpenVPN client. This value is assigned by the ' .
                'system and cannot be changed.',
        );
        $this->vpnif = new StringField(
            unique: true,
            default: null, // populated by from_internal_vpnif
            editable: false,
            read_only: true,
            representation_only: true,
            verbose_name: 'VPN Interface',
            help_text: 'The VPN interface name for this OpenVPN client. This value is assigned by the system and ' .
                'cannot be changed.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The description for this OpenVPN client.',
        );
        $this->disable = new BooleanField(
            default: false,
            verbose_name: 'Disable',
            help_text: 'Disables this OpenVPN client.',
        );
        $this->mode = new StringField(
            required: true,
            choices: ['p2p_tls'],
            verbose_name: 'Mode',
            help_text: 'The OpenVPN client mode.',
        );
        $this->dev_mode = new StringField(
            required: true,
            choices: ['tun', 'tap'],
            verbose_name: 'Dev Mode',
            help_text: 'The carrier mode for this OpenVPN client. `tun` mode carries IPv4 and IPv6 (layer 3) and ' .
                'is the most common and compatible mode across all platforms. `tap` mode is capable of carrying ' .
                '802.3 (layer 2).',
        );
        $this->protocol = new StringField(
            required: true,
            choices: ['UDP4', 'UDP6', 'UDP', 'TCP4', 'TCP6', 'TCP'],
            verbose_name: 'Protocol',
            help_text: 'The protocol used by this OpenVPN client.',
        );
        $this->interface = new InterfaceField(
            required: true,
            allow_carp_interface: true,
            allow_localhost_interface: true,
            conditions: ['!protocol' => ['UDP', 'TCP']],
            verbose_name: 'Interface',
            help_text: 'The interface used by the firewall to originate this OpenVPN client connection.',
        );
        $this->server_addr = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Server Addr',
            help_text: 'The IP address or hostname of the OpenVPN server this client will connect to.',
        );
        $this->server_port = new PortField(
            required: true,
            allow_alias: false,
            allow_range: false,
            allow_null: false,
            verbose_name: 'Server Port',
            help_text: 'The port used by the server to receive client connections.',
        );
        $this->local_port = new PortField(
            default: null,
            allow_alias: false,
            allow_range: false,
            allow_null: true,
            verbose_name: 'Local Port',
            help_text: 'The port binding used by OpenVPN for client connections.',
        );
        $this->proxy_addr = new StringField(
            default: null,
            allow_null: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Proxy Addr',
            help_text: 'The address for an HTTP Proxy this client can use to connect to a remote server.',
        );
        $this->proxy_port = new PortField(
            default: null,
            allow_null: true,
            verbose_name: 'Proxy Port',
            help_text: 'The port used by the HTTP Proxy.',
        );
        $this->proxy_authtype = new StringField(
            default: 'none',
            choices: ['none', 'basic', 'ntlm'],
            verbose_name: 'Proxy Authtype',
            help_text: 'The type of authentication used by the proxy server.',
        );
        $this->proxy_user = new StringField(
            required: true,
            conditions: ['!proxy_authtype' => 'none'],
            verbose_name: 'Proxy User',
            help_text: 'The username to use for authentication to the remote proxy.',
        );
        $this->proxy_passwd = new StringField(
            required: true,
            sensitive: true,
            conditions: ['!proxy_authtype' => 'none'],
            verbose_name: 'Proxy Passwd',
            help_text: 'The username to use for authentication to the remote proxy.',
        );
        $this->auth_user = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'Auth User',
            help_text: 'The username used to authenticate with the OpenVPN server.',
        );
        $this->auth_pass = new StringField(
            default: null,
            allow_null: true,
            conditions: ['!auth_user' => null],
            verbose_name: 'Auth Pass',
            help_text: 'The password used to authenticate with the OpenVPN server.',
        );
        $this->auth_retry_none = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'auth-retry-none',
            verbose_name: 'Auth Retry None',
            help_text: 'Disables retrying authentication if an authentication failed error is received from the server',
        );
        $this->tls = new Base64Field(
            default: null,
            allow_null: true,
            verbose_name: 'TLS',
            help_text: 'The TLS key this OpenVPN client will use to sign control channel packets with an HMAC ' .
                'signature for authentication when establishing the tunnel.',
        );
        $this->tls_type = new StringField(
            required: true,
            choices: ['auth', 'crypt'],
            conditions: ['!tls' => null],
            verbose_name: 'TLS Type',
            help_text: 'The TLS key usage type. In `auth` mode, the TLS key is used only as HMAC authentication for ' .
                'the control channel, protecting the peers from unauthorized connections. The `crypt` mode encrypts ' .
                'the control channel communication in addition to providing authentication, providing more privacy ' .
                'and traffic control channel obfuscation.',
        );
        $this->tlsauth_keydir = new StringField(
            default: 'default',
            choices: ['default', '0', '1', '2'],
            conditions: ['!tls' => null],
            verbose_name: 'Tlsauth Keydir',
            help_text: 'The TLS key direction. This must be set to complementary values on the client and client. ' .
                'For example, if the client is set to 0, the client must be set to 1. Both may be set to omit the ' .
                'direction, in which case the TLS Key will be used bidirectionally.',
        );
        $this->caref = new ForeignModelField(
            model_name: 'CertificateAuthority',
            model_field: 'refid',
            required: true,
            verbose_name: 'CA Reference',
            help_text: 'The `refid` of the CA object to assume as the peer CA.',
        );
        $this->certref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            default: null,
            allow_null: true,
            verbose_name: 'Certificate Reference',
            help_text: 'The `refid` of the certificate object to assume as the OpenVPN client certificate.',
        );
        $this->data_ciphers = new StringField(
            required: true,
            choices_callable: 'get_available_cipher_choices',
            many: true,
            many_minimum: 1,
            verbose_name: 'Data Ciphers',
            help_text: 'The encryption algorithms/ciphers allowed by this OpenVPN client.',
        );
        $this->data_ciphers_fallback = new StringField(
            required: true,
            choices_callable: 'get_available_cipher_choices',
            verbose_name: 'Data Ciphers Fallback',
            help_text: 'The fallback encryption algorithm/cipher used for data channel packets when communicating ' .
                'with clients that do not support data encryption algorithm negotiation (e.g. Shared Key).',
        );
        $this->digest = new StringField(
            required: true,
            choices_callable: 'get_available_digest_choices',
            verbose_name: 'Digest',
            help_text: 'The algorithm used to authenticate data channel packets, and control channel packets ' .
                'if a TLS Key is present.',
        );
        $this->remote_cert_tls = new BooleanField(
            default: true,
            verbose_name: 'Remote Certificate TLS',
            help_text: 'Enables or disables requiring hosts to have a client certificate to connect.',
        );
        $this->tunnel_network = new StringField(
            default: '',
            allow_empty: true,
            validators: [new SubnetValidator(allow_ipv4: true, allow_ipv6: false, allow_alias: false)],
            verbose_name: 'Tunnel Network',
            help_text: 'The IPv4 virtual network used for private communications between this client and client hosts.',
        );
        $this->tunnel_networkv6 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new SubnetValidator(allow_ipv4: false, allow_ipv6: true, allow_alias: false)],
            verbose_name: 'IPv6 Tunnel Network',
            help_text: 'The IPv6 virtual network used for private communications between this client and client hosts.',
        );
        $this->remote_network = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            validators: [new SubnetValidator(allow_ipv4: true, allow_ipv6: false, allow_alias: true)],
            verbose_name: 'Remote Network',
            help_text: 'IPv4 networks that will be routed through the tunnel, so that a site-to-site VPN can be ' .
                'established without manually changing the routing tables. Expressed as a list of ' .
                'one or more CIDR ranges or host/network type aliases. If this is a site-to-site VPN, enter the ' .
                'remote LAN/s here. May be left empty for non site-to-site VPN.',
        );
        $this->remote_networkv6 = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            validators: [new SubnetValidator(allow_ipv4: false, allow_ipv6: true, allow_alias: true)],
            verbose_name: 'IPv6 Remote Network',
            help_text: 'IPv6 networks that will be routed through the tunnel, so that a site-to-site VPN can be ' .
                'established without manually changing the routing tables. Expressed as a list of ' .
                'one or more CIDR ranges or host/network type aliases. If this is a site-to-site VPN, enter the ' .
                'remote LAN/s here. May be left empty for non site-to-site VPN.',
        );
        $this->use_shaper = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 100,
            maximum: 100000000,
            verbose_name: 'Use Shaper',
            help_text: 'Maximum outgoing bandwidth (in bytes per second) for this tunnel. Use `null` no limit.',
        );
        $this->allow_compression = new StringField(
            default: 'no',
            choices: ['no', 'yes', 'asym'],
            verbose_name: 'Allow Compression',
            help_text: 'The compression mode allowed by this OpenVPN client. Compression can potentially increase ' .
                'throughput but may allow an attacker to extract secrets if they can control compressed plaintext ' .
                'traversing the VPN (e.g. HTTP)',
        );
        $this->topology = new StringField(
            default: 'subnet',
            choices: ['subnet', 'net30'],
            conditions: ['dev_mode' => 'tun'],
            verbose_name: 'Topology',
            help_text: 'The method used to supply a virtual adapter IP address to clients when using TUN mode on IPv4.',
        );
        $this->passtos = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'Pass TOS',
            help_text: 'Enables or disables setting the TOS IP header value of tunnel packets to match the ' .
                'encapsulated packet value.',
        );
        $this->route_no_pull = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'Route No Pull',
            help_text: "Enables or disables the servers ability to add routes to the client's routing table.",
        );
        $this->route_no_exec = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'Route No Exec',
            help_text: 'Enables or disables adding/removing routes automatically.',
        );
        $this->dns_add = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'DNS Add',
            help_text: 'Enables or disables using the DNS server(s) provided by the OpenVPN server.',
        );
        $this->inactive_seconds = new IntegerField(
            default: 300,
            verbose_name: 'Inactive Seconds',
            help_text: 'The amount of time (in seconds) until a client connection is closed for inactivity.',
        );
        $this->ping_method = new StringField(
            default: 'keepalive',
            choices: ['keepalive', 'ping'],
            verbose_name: 'Ping Method',
            help_text: 'The method used to define ping configuration.',
        );
        $this->keepalive_interval = new IntegerField(
            default: 10,
            conditions: ['ping_method' => 'keepalive'],
            verbose_name: 'Keepalive Interval',
            help_text: 'The keepalive interval parameter.',
        );
        $this->keepalive_timeout = new IntegerField(
            default: 60,
            conditions: ['ping_method' => 'keepalive'],
            verbose_name: 'Keepalive Timeout',
            help_text: 'The keepalive timeout parameter.',
        );
        $this->ping_seconds = new IntegerField(
            default: 10,
            conditions: ['ping_method' => 'ping'],
            verbose_name: 'Ping Seconds',
            help_text: 'The number of seconds to accept no packets before sending a ping to the ' .
                'remote peer over the TCP/UDP control channel.',
        );
        $this->ping_action = new StringField(
            default: 'ping_restart',
            choices: ['ping_restart', 'ping_exit'],
            conditions: ['ping_method' => 'ping'],
            verbose_name: 'Ping Action',
            help_text: 'The action to take after a ping to the remote peer times-out.',
        );
        $this->ping_action_seconds = new IntegerField(
            default: 60,
            conditions: ['ping_method' => 'ping'],
            verbose_name: 'Ping Action Seconds',
            help_text: 'The number of seconds that must elapse before the ping is considered a timeout and the ' .
                'configured `ping_action` is performed.',
        );
        $this->custom_options = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: ';',
            verbose_name: 'Custom Options',
            help_text: 'Additional options to add to the OpenVPN client configuration.',
        );
        $this->udp_fast_io = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'UDP Fast I/O',
            help_text: 'Enables or disables fast I/O operations with UDP writes to tun/tap (Experimental).',
        );
        $this->exit_notify = new StringField(
            default: 'none',
            choices: ['1', '2', '3', '4', '5', 'none'],
            verbose_name: 'Exit Notify',
            help_text: 'The number of times this client will attempt to send an exit notifications.',
        );
        $this->sndrcvbuf = new IntegerField(
            default: null,
            choices: [65536, 131072, 262144, 524288, 1048576, 2097152],
            allow_null: true,
            verbose_name: 'Send/Receive Buffer',
            help_text: 'The send and receive buffer size for OpenVPN. Set to null to use the system default.',
        );
        $this->create_gw = new StringField(
            default: 'both',
            choices: ['both', 'v4only', 'v6only'],
            verbose_name: 'Create Gw',
            help_text: 'The gateway type(s) that will be created when a virtual interface is assigned ' .
                'to this OpenVPN server',
        );
        $this->verbosity_level = new IntegerField(
            default: 1,
            choices: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11],
            verbose_name: 'Verbosity Level',
            help_text: 'The OpenVPN logging verbosity level.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to this entire Model object.
     * @throws ValidationError When the sum of all items assigned to the `data_ciphers` field exceeds 254 characters.
     */
    public function validate_extra(): void {
        # Do not allow the entirety of the data-ciphers options string to exceed 254 characters
        $data_ciphers_opt = 'data-ciphers ' . $this->data_ciphers->to_internal();
        if (strlen($data_ciphers_opt) > 254) {
            throw new ValidationError(
                message: 'Field `data_ciphers` has too many items. The sum of all item characters exceeds OpenVPN ' .
                    'character limits.',
                response_id: 'OPENVPN_CLIENT_DATA_CIPHERS_CONTAINS_TOO_MANY_CHARS',
            );
        }

        # Ensure the vpnif field value is populated
        $this->vpnif->value = self::INTERFACE_PREFIX . $this->vpnid->value;
    }

    /**
     * Populate the `vpnif` field value from internal since this is not a stored value.
     * @return string The `vpnif` value's internal value
     */
    public function from_internal_vpnif(): string {
        return self::INTERFACE_PREFIX . $this->vpnid->value;
    }

    /**
     * Adds extra validation to the `disable` field.
     * @param bool $disable The incoming value to validate.
     * @return bool The validated value to be assigned.
     * @throws ConflictError When $disable is true, but this OpenVPNClient is in use by an existing interface.
     */
    public function validate_disable(bool $disable): bool {
        # Do not allow this OpenVPN client to be disabled if it's in use by an interface
        if ($disable and openvpn_inuse($this->vpnid->value, 'client')) {
            throw new ConflictError(
                message: 'Field `disable` cannot be set to `true` while this OpenVPNClient is in use.',
                response_id: 'OPENVPN_CLIENT_CANNOT_BE_DISABLED_WHILE_IN_USE',
            );
        }

        return $disable;
    }

    /**
     * Adds extra validation to the `interface` field.
     * @param string $interface The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When the `protocol` field is an IPv4 protocol, but the $interface has no IPv4 address
     * @throws ValidationError When the `protocol` field is an IPv6 protocol, but the $interface has no IPv6 address
     *
     */
    public function validate_interface(string $interface): string {
        # Shortname variable for related NetworkInterface object
        $if_obj = $this->interface->get_related_model();

        # Require the interface to have an IPv4 address if `protocol` is set to an IPv4 protocol
        if (in_array($this->protocol->value, ['UDP4', 'TCP4']) and !$if_obj->get_current_ipv4()) {
            throw new ValidationError(
                message: 'Field `interface` must be an interface with an IPv4 address while ' .
                    '`protocol` is `UDP4` or `TCP4`.',
                response_id: 'OPENVPN_CLIENT_INTERFACE_HAS_NO_IPV4_ASSIGNMENT',
            );
        }

        # Require the interface to have an IPv6 address if `protocol` is set to an IPv6 protocol
        if (in_array($this->protocol->value, ['UDP6', 'TCP6']) and !$if_obj->get_current_ipv4()) {
            throw new ValidationError(
                message: 'Field `interface` must be an interface with an IPv6 address while ' .
                    '`protocol` is `UDP6` or `TCP6`.',
                response_id: 'OPENVPN_CLIENT_INTERFACE_HAS_NO_IPV6_ASSIGNMENT',
            );
        }

        return $interface;
    }

    /**
     * Adds extra validation to the `tls` field.
     * @param string $tls The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $tls is not a valid OpenVPN static key.
     */
    public function validate_tls(string $tls): string {
        # Ensure this TLS key begins with the OpenVPN key prefix and suffix
        if (!str_contains($tls, self::STATIC_KEY_PREFIX) and !str_contains($tls, self::STATIC_KEY_SUFFIX)) {
            throw new ValidationError(
                message: 'Field `tls` must be a valid OpenVPN TLS key.',
                response_id: 'OPENVPN_CLIENT_TLS_INVALID_KEY',
            );
        }

        return $tls;
    }

    /**
     * Adds extra validation to the `tunnel_network` field.
     * @param string $tunnel_network The incoming value to validate.
     * @return string The validated value to be assigned.
     */
    public function validate_tunnel_network(string $tunnel_network): string {
        # Force the subnet to use the full network address CIDR value (i.e. 10.1.2.3/24 becomes 10.1.2.0/24)
        return openvpn_tunnel_network_fix($tunnel_network);
    }

    /**
     * Adds extra validation to the `tunnel_networkv6` field.
     * @param string $tunnel_networkv6 The incoming value to validate.
     * @return string The validated value to be assigned.
     */
    public function validate_tunnel_networkv6(string $tunnel_networkv6): string {
        # Force the subnet to use the full network address CIDR value (i.e. 1234::2/24 becomes 1234::/24)
        return openvpn_tunnel_network_fix($tunnel_networkv6);
    }

    /**
     * Obtains the next available VPN ID.
     * @return integer The next available ID number.
     */
    public function get_next_vpn_id(): int {
        return openvpn_vpnid_next();
    }

    /**
     * Obtains the available cipher options for this system.
     * @return array An array of available cipher options for this system.
     */
    public function get_available_cipher_choices(): array {
        return openvpn_get_cipherlist();
    }

    /**
     * Obtains the available `digest` options for this system.
     * @return array An array of available digest algorithms for this system.
     */
    public function get_available_digest_choices(): array {
        return openvpn_get_digestlist();
    }

    /**
     * Overrides the parent _delete() method to ensure this OpenVPNClient is not in use before deletion.
     */
    public function _delete(): void {
        # Do not allow this OpenVPN client to be deleted if it's in use by an interface
        if (openvpn_inuse($this->vpnid->value, 'client')) {
            throw new ConflictError(
                message: 'OpenVPNClient cannot be deleted while it is in use.',
                response_id: 'OPENVPN_CLIENT_CANNOT_BE_DELETED_WHILE_IN_USE',
            );
        }
        parent::_delete();
    }

    /**
     * Applies changes to this OpenVPN Server.
     */
    public function apply(): void {
        # Delete the old OpenVPN interface if the device mode changed
        if ($this->initial_object and $this->initial_object->dev_mode->value !== $this->dev_mode->value) {
            openvpn_delete('client', $this->initial_object->to_internal());
        }

        # Apply changes to this OpenVPN client and restart services.
        openvpn_resync('client', $this->to_internal());
        openvpn_resync_csc_all();
        services_unbound_configure(false);
    }

    /**
     * Applies the deletion of this OpenVPN client
     */
    public function apply_delete(): void {
        openvpn_delete('client', $this->to_internal());
    }
}
