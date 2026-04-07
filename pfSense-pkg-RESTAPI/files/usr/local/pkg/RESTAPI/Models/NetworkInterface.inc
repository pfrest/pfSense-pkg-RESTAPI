<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\InterfaceApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\FilterNameValidator;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\MACAddressValidator;
use RESTAPI\Validators\SubnetValidator;
use RESTAPI\Validators\UniqueFromForeignModelValidator;

/**
 * Defines a Model that interacts with network interfaces. Note: PHP does not allow us to name a
 * class `Interface` so we must use `NetworkInterface` even though this isn't inconsistent.
 */
class NetworkInterface extends Model {
    public StringField $if;
    public BooleanField $enable;
    public StringField $descr;
    public StringField $spoofmac;
    public IntegerField $mtu;
    public IntegerField $mss;
    public StringField $media;
    public StringField $mediaopt;
    public BooleanField $blockpriv;
    public BooleanField $blockbogons;
    public StringField $typev4;
    public StringField $ipaddr;
    public IntegerField $subnet;
    public ForeignModelField $gateway;
    public StringField $dhcphostname;
    public StringField $alias_address;
    public IntegerField $alias_subnet;
    public StringField $dhcprejectfrom;
    public BooleanField $adv_dhcp_config_advanced;
    public StringField $adv_dhcp_pt_values;
    public IntegerField $adv_dhcp_pt_timeout;
    public IntegerField $adv_dhcp_pt_retry;
    public IntegerField $adv_dhcp_pt_select_timeout;
    public IntegerField $adv_dhcp_pt_reboot;
    public IntegerField $adv_dhcp_pt_backoff_cutoff;
    public IntegerField $adv_dhcp_pt_initial_interval;
    public StringField $adv_dhcp_send_options;
    public StringField $adv_dhcp_request_options;
    public StringField $adv_dhcp_required_options;
    public StringField $adv_dhcp_option_modifiers;
    public BooleanField $adv_dhcp_config_file_override;
    public StringField $adv_dhcp_config_file_override_path;
    public StringField $typev6;
    public StringField $ipaddrv6;
    public IntegerField $subnetv6;
    public ForeignModelField $gatewayv6;
    public BooleanField $ipv6usev4iface;
    public BooleanField $slaacusev4iface;
    public StringField $prefix_6rd;
    public StringField $gateway_6rd;
    public IntegerField $prefix_6rd_v4plen;
    public InterfaceField $track6_interface;
    public StringField $track6_prefix_id_hex;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set the configuration path for these objects and allow many to be stored.
        $this->config_path = 'interfaces';
        $this->many = true;
        $this->subsystem = 'interfaces';
        $this->update_strategy = 'replace';
        $this->id_type = 'string';

        # Model Fields
        $this->if = new StringField(
            required: true,
            unique: true,
            choices_callable: 'get_interface_ports',
            maximum_length: 128,
            verbose_name: 'Interface Name',
            help_text: 'The real interface this configuration will be applied to.',
        );
        $this->enable = new BooleanField(
            required: false,
            default: false,
            verbose_name: 'Enable',
            help_text: 'Enable or disable this interface.',
        );
        $this->descr = new StringField(
            required: true,
            unique: true,
            maximum_length: 128,
            validators: [
                new FilterNameValidator(),
                new UniqueFromForeignModelValidator(model: 'FirewallAlias', field: 'name'),
            ],
            verbose_name: 'Description',
            help_text: 'The descriptive name for this interface.',
        );
        $this->spoofmac = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            maximum_length: 17,
            validators: [new MACAddressValidator()],
            verbose_name: 'Spoof MAC Address',
            help_text: "Assigns (spoofs) the MAC address for this interface instead of using the interface's real MAC.",
        );
        $this->mtu = new IntegerField(
            required: false,
            allow_null: true,
            minimum: 1280,
            maximum: 8192,
            verbose_name: 'MTU',
            help_text: 'Sets the MTU for this interface. Assumes default MTU if value is `null`.',
        );
        $this->mss = new IntegerField(
            required: false,
            allow_null: true,
            minimum: 576,
            maximum: 65535,
            verbose_name: 'MSS',
            help_text: 'Sets the MSS for this interface. Assumes default MSS if value is `null`.',
        );
        $this->media = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            internal_name: 'media',
            verbose_name: 'Media',
            help_text: 'Sets the link speed for this interface. In most situations this can be left as the default.',
        );
        $this->mediaopt = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            internal_name: 'mediaopt',
            verbose_name: 'Media Options',
            help_text: 'Sets the link duplex for this interface. In most situations this can be left as the default.',
        );
        $this->blockpriv = new BooleanField(
            required: false,
            default: false,
            verbose_name: 'Block Private Networks',
            help_text: 'Enable or disable automatically blocking RFC 1918 private networks on this interface.',
        );
        $this->blockbogons = new BooleanField(
            required: false,
            default: false,
            verbose_name: 'Block Bogon Networks',
            help_text: 'Enable or disable automatically blocking bogon networks on this interface.',
        );
        $this->typev4 = new StringField(
            required: true,
            choices: ['static', 'dhcp', 'none'],
            representation_only: true,
            verbose_name: 'Typev4',
            help_text: 'Selects the IPv4 address type to assign this interface.',
        );
        $this->ipaddr = new StringField(
            required: true,
            conditions: ['typev4' => ['static', 'dhcp']],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false, allow_keywords: ['dhcp'])],
            verbose_name: 'IP Address',
            help_text: 'Sets the IPv4 address to assign to this interface.',
        );
        $this->subnet = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 32,
            conditions: ['typev4' => 'static'],
            verbose_name: 'Subnet',
            help_text: 'Sets the subnet bit count to assign this interface.',
        );
        $this->gateway = new ForeignModelField(
            model_name: ['RoutingGateway', 'RoutingGatewayGroup'],
            model_field: 'name',
            model_query: ['ipprotocol' => 'inet'],
            allow_null: true,
            conditions: ['typev4' => 'static'],
            verbose_name: 'Gateway',
            help_text: 'Sets the upstream gateway this interface will use. This is only applicable for WAN-type interfaces.',
        );
        $this->dhcphostname = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['typev4' => 'dhcp'],
            validators: [new IPAddressValidator(allow_ipv4: false, allow_ipv6: false, allow_fqdn: true)],
            verbose_name: 'DHCP Hostname',
            help_text: 'Sets the DHCP hostname this interface will advertise via DHCP.',
        );
        $this->alias_address = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'alias-address',
            conditions: ['typev4' => 'dhcp'],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false, allow_fqdn: false)],
            verbose_name: 'Alias Address',
            help_text: 'Sets the value used as a fixed alias IPv4 address by the DHCP client.',
        );
        $this->alias_subnet = new IntegerField(
            default: 32,
            allow_null: true,
            minimum: 0,
            maximum: 32,
            internal_name: 'alias-subnet',
            conditions: ['typev4' => 'dhcp'],
            verbose_name: 'Alias Subnet',
            help_text: "Sets the value used as the fixed alias IPv4 address's subnet bit count by the DHCP client.",
        );
        $this->dhcprejectfrom = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: ',',
            conditions: ['typev4' => 'dhcp'],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false, allow_fqdn: false)],
            verbose_name: 'DHCP Reject From',
            help_text: 'Sets a list of IPv4 DHCP server addresses to reject DHCP offers for on this interface.',
        );
        $this->adv_dhcp_config_advanced = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['typev4' => 'dhcp'],
            verbose_name: 'Advanced DHCP Config',
            help_text: 'Enables or disables the advanced DHCP settings on this interface.',
        );
        $this->adv_dhcp_pt_values = new StringField(
            default: 'SavedCfg',
            choices: ['SavedCfg'],
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Timing Values',
            help_text: 'Selects the advanced DHCP timing preset.',
        );
        $this->adv_dhcp_pt_timeout = new IntegerField(
            allow_null: true,
            minimum: 1,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Timeout',
            help_text: 'Manually sets the timeout timing value used when requested DHCP leases on this interface.',
        );
        $this->adv_dhcp_pt_retry = new IntegerField(
            allow_null: true,
            minimum: 1,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Retry',
            help_text: 'Manually sets the retry timing value used when requested DHCP leases on this interface.',
        );
        $this->adv_dhcp_pt_select_timeout = new IntegerField(
            allow_null: true,
            minimum: 0,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Select Timeout',
            help_text: 'Manually sets the select timing value used when requested DHCP leases on this interface.',
        );
        $this->adv_dhcp_pt_reboot = new IntegerField(
            allow_null: true,
            minimum: 1,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Reboot',
            help_text: 'Manually sets the reboot timing value used when requested DHCP leases on this interface.',
        );
        $this->adv_dhcp_pt_backoff_cutoff = new IntegerField(
            allow_null: true,
            minimum: 1,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Backoff Cutoff',
            help_text: 'Manually sets the backoff cutoff timing value used when requested DHCP leases on this interface.',
        );
        $this->adv_dhcp_pt_initial_interval = new IntegerField(
            allow_null: true,
            minimum: 1,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Initial Interval',
            help_text: 'Manually sets the initial interval timing value used when requested DHCP leases on this interface.',
        );
        $this->adv_dhcp_send_options = new StringField(
            allow_null: true,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Send Options',
            help_text: 'Sets DHCP options to be sent when requesting a DHCP lease for this interface.',
        );
        $this->adv_dhcp_request_options = new StringField(
            allow_null: true,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Request Options',
            help_text: 'Sets DHCP option 55 values to be sent when requesting a DHCP lease for this interface.',
        );
        $this->adv_dhcp_required_options = new StringField(
            allow_null: true,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Required Options',
            help_text: 'Sets DHCP options required by the client when requesting a DHCP lease for this interface.',
        );
        $this->adv_dhcp_option_modifiers = new StringField(
            allow_null: true,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_advanced' => true],
            verbose_name: 'DHCP Option Modifiers',
            help_text: 'Sets DHCP option modifiers applied to the obtained DHCP lease.',
        );
        $this->adv_dhcp_config_file_override = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['typev4' => 'dhcp'],
            verbose_name: 'DHCP Config File Override',
            help_text: 'Enables or disables overriding the entire DHCP configuration file for this interface.',
        );
        $this->adv_dhcp_config_file_override_path = new StringField(
            allow_null: true,
            conditions: ['typev4' => 'dhcp', 'adv_dhcp_config_file_override' => true],
            verbose_name: 'DHCP Config Override Path',
            help_text: 'Sets the local file path of the custom DHCP configuration file.',
        );
        $this->typev6 = new StringField(
            choices: ['staticv6', 'dhcp6', 'slaac', '6rd', 'track6', '6to4', 'none'],
            allow_null: true,
            representation_only: true,
            verbose_name: 'Typev6',
            help_text: 'Selects the IPv6 address type to assign this interface.',
        );
        $this->ipaddrv6 = new StringField(
            required: true,
            conditions: ['typev6' => ['staticv6', 'dhcp6', 'slaac', '6rd', 'track6', '6to4']],
            validators: [
                new IPAddressValidator(
                    allow_ipv4: false,
                    allow_ipv6: true,
                    allow_ipv6_link_local: false,
                    allow_keywords: ['dhcp6', 'slaac', '6rd', 'track6', '6to4'],
                ),
            ],
            verbose_name: 'IPv6 Address',
            help_text: 'Sets the IPv6 address to assign to this interface.',
        );
        $this->subnetv6 = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 128,
            conditions: ['typev6' => 'staticv6'],
            verbose_name: 'IPv6 Subnet',
            help_text: 'Sets the subnet bit count to assign this interface.',
        );
        $this->gatewayv6 = new ForeignModelField(
            model_name: ['RoutingGateway', 'RoutingGatewayGroup'],
            model_field: 'name',
            model_query: ['ipprotocol' => 'inet6'],
            allow_null: true,
            conditions: ['typev6' => 'staticv6'],
            verbose_name: 'IPv6 Gateway',
            help_text: 'Sets the upstream IPv6 gateway this interface will use. This is only applicable for WAN-type interfaces.',
        );
        $this->ipv6usev4iface = new BooleanField(
            default: false,
            conditions: ['typev6' => 'staticv6'],
            verbose_name: 'IPv6 Use IPv4 Interface',
            help_text: 'Enable or disable IPv6 using the IPv4 connectivity link (PPPoE).',
        );
        $this->slaacusev4iface = new BooleanField(
            default: false,
            conditions: ['typev6' => 'slaac'],
            verbose_name: 'SLAAC Use IPv4 Interface',
            help_text: 'Enable or disable IPv6 using the IPv4 connectivity link (PPPoE).',
        );
        $this->prefix_6rd = new StringField(
            required: true,
            unique: true,
            internal_name: 'prefix-6rd',
            conditions: ['typev6' => '6rd'],
            validators: [new SubnetValidator(allow_ipv4: false, allow_ipv6: true)],
            verbose_name: 'Prefix 6RD',
            help_text: 'Sets the 6RD IPv6 prefix assigned by the ISP for this interface.',
        );
        $this->gateway_6rd = new StringField(
            required: true,
            internal_name: 'gateway-6rd',
            conditions: ['typev6' => '6rd'],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Gateway 6RD',
            help_text: 'Sets the 6RD IPv4 gateway address assigned by the ISP for this interface.',
        );
        $this->prefix_6rd_v4plen = new IntegerField(
            required: true,
            minimum: 0,
            maximum: 32,
            internal_name: 'prefix-6rd-v4plen',
            conditions: ['typev6' => '6rd'],
            verbose_name: 'Prefix 6RD V4plen',
            help_text: 'Sets the 6RD IPv4 prefix length. Normally specified by the ISP. A value of 0 means embed the' .
                'entire IPv4 address in the 6RD prefix.',
        );
        $this->track6_interface = new InterfaceField(
            required: true,
            interface_query: ['id__except' => &$this->id, 'typev6__except' => 'none'],
            internal_name: 'track6-interface',
            conditions: ['typev6' => 'track6'],
            verbose_name: 'Track6 Interface',
            help_text: 'Sets the dynamic IPv6 WAN interface to track for configuration.',
        );
        $this->track6_prefix_id_hex = new StringField(
            default: '0',
            internal_name: 'track6-prefix-id--hex',
            conditions: ['typev6' => 'track6'],
            verbose_name: 'Track6 Prefix ID Hex',
            help_text: 'Sets the hexadecimal IPv6 prefix ID. This determines the configurable network ID based on the ' .
                'dynamic IPv6 connection.',
        );

        # Needs to be constructed after Fields are defined
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Populates the `descr` field as the interface's ID in uppercase if no value exists. This is necessary because
     * some default pfSense interfaces are created without a description assigned.
     */
    public function from_internal_descr(mixed $descr): string {
        # Check if descr has no value, if so, assign it this object's ID in uppercase
        if (empty($descr)) {
            return strtoupper($this->id);
        }

        # Otherwise, just return the existing value
        return $descr;
    }

    /**
     * Obtains an array of the (real) interfaces available to this system. This populates the `choices` available to
     * the `if` field.
     * @returns array An array of the (real) interfaces available to this system where the array key is the real
     * interface ID/name and the value is an array of interface attributes
     */
    public function get_interface_ports(): array {
        return self::get_all_avail_interfaces();
    }

    /**
     * Adds custom validation to the `descr` field.
     * @param string $descr The descr value to validate.
     * @returns string The validated $descr to set.
     * @throws ValidationError When the `$descr` value is invalid.     */
    public function validate_descr(string $descr): string {
        # Ensure OpenVPN interfaces are 22 characters or less
        if (str_starts_with($this->if->value, 'ovpn') and strlen($descr) > 22) {
            throw new ValidationError(
                message: "Field 'descr' must be 22 characters or less when 'if' is an OpenVPN interface",
                response_id: 'NETWORK_INTERFACE_DESCR_OPENVPN_MAX_LENGTH_EXCEEDED',
            );
        }

        # Ensure IPsec interfaces are 22 characters or less
        if (str_starts_with($this->if->value, 'ipsec') and strlen($descr) > 22) {
            throw new ValidationError(
                message: "Field 'descr' must be 22 characters or less when 'if' is an IPsec interface",
                response_id: 'NETWORK_INTERFACE_DESCR_IPSEC_MAX_LENGTH_EXCEEDED',
            );
        }

        # Ensure GIF interfaces are 22 characters or less
        if (str_starts_with($this->if->value, 'gif') and strlen($descr) > 22) {
            throw new ValidationError(
                message: "Field 'descr' must be 22 characters or less when 'if' is a GIF interface",
                response_id: 'NETWORK_INTERFACE_DESCR_GIF_MAX_LENGTH_EXCEEDED',
            );
        }

        # Ensure GRE interfaces are 22 characters or less
        if (str_starts_with($this->if->value, 'gre') and strlen($descr) > 22) {
            throw new ValidationError(
                message: "Field 'descr' must be 22 characters or less when 'if' is a GRE interface",
                response_id: 'NETWORK_INTERFACE_DESCR_GRE_MAX_LENGTH_EXCEEDED',
            );
        }

        return $descr;
    }

    /**
     * Adds custom validation to the `mtu` field.
     * @param int $mtu The MTU value to validate.
     * @returns string The validated $mtu to set.
     * @throws ValidationError When the `if` value is invalid.
     */
    public function validate_mtu(int $mtu): int {
        # Check if this interface is a VLAN interface
        $vlan_query = InterfaceVLAN::query(['vlanif' => $this->if->value]);

        # If this interface is a VLAN interface, ensure the MTU is less than the parent interface
        if ($vlan_query->exists()) {
            $vlan = $vlan_query->model_objects[0];

            # Throw an error if the parent interface's MTU is lower than this interface's MTU
            if (get_interface_mtu($vlan->if->value) < $mtu) {
                throw new ValidationError(
                    message: "MTU cannot be higher than parent interface's MTU",
                    response_id: 'INTERFACE_WITH_MTU_HIGHER_THAN_PARENT_MTU',
                );
            }
        }

        return $mtu;
    }

    /**
     * Adds custom validation to the `media` field.
     * @param string $media The media value to validate (e.g. 1000baseT)
     * @returns string The validated $media to set.
     * @throws ValidationError When the `$media` value is invalid.
     */
    public function validate_media(string $media): string {
        # Variables
        $media_types = array_keys($this->get_supported_media());
        $media_types_str = implode(',', $media_types);

        # Throw an error if the provided media is not supported
        if (!in_array($media, $media_types)) {
            throw new ValidationError(
                message: "Interface does not support media type `$media`, supported options are: [$media_types_str]",
                response_id: 'NETWORK_INTERFACE_WITH_UNSUPPORTED_MEDIA_TYPE',
            );
        }

        return $media;
    }

    /**
     * Adds custom validation to the `mediaopt` field.
     * @param string $mediaopt The mediaopt value to validate (e.g. full-duplex)
     * @returns string The validated $mediaopt to set.
     * @throws ValidationError When the `mediaopt` value is invalid.
     */
    public function validate_mediaopt(string $mediaopt): string {
        # Variables
        $media_options = $this->get_supported_media()[$this->media->value];
        $media_options_str = implode(',', $media_options);

        # Throw an error if the provided media is not supported
        if (!in_array($mediaopt, $media_options)) {
            throw new ValidationError(
                message: "Interface does not support media option `$mediaopt`, supported options are: [$media_options_str]",
                response_id: 'NETWORK_INTERFACE_WITH_UNSUPPORTED_MEDIA_OPTION',
            );
        }

        return $mediaopt;
    }

    /**
     * Adds custom validation to the `blockbogons` field.
     * @param bool $blockbogons The blockbogons value to validate
     * @returns bool The validated $blockbogons to set.
     * @throws ValidationError When the `blockbogons` value is invalid.
     */
    public function validate_blockbogons(bool $blockbogons): bool {
        # Gather applicable configuration values
        $ipv6_allowed = $this->is_config_enabled(path: 'system', enable_key: 'ipv6allow');
        $max_table_entries = $this->get_config(path: 'system/maximumtableentries', default: 0);
        $min_bogonv6_entries = g_get('minimumtableentries_bogonsv6');

        # Do not allow block bogons if IPv6 is enabled, but maximum table entries are less than the bogon minimum
        if ($blockbogons and $ipv6_allowed and $max_table_entries < $min_bogonv6_entries) {
            throw new ValidationError(
                message: "Field 'blockbogons' cannot be enabled until the maximum table entries value is increased " .
                    "to at least $min_bogonv6_entries",
                response_id: 'NETWORK_INTERFACE_BLOCKBOGONS_ENABLED_WITH_INSUFFICIENT_TABLE_ENTRY_LIMIT',
            );
        }

        return $blockbogons;
    }

    /**
     * Populates the `typev4` field if a value is not present
     * @param string $typev4 The typev4 value to override
     * @returns string The overridden $typev4 value to set.
     * @throws ValidationError When the `$typev4` value is invalid.
     */
    public function from_internal_typev4(mixed $typev4): string {
        # Only populate this value if a value was not already set
        if (is_null($typev4)) {
            # Set the type to static if an IP address is set
            if (is_ipaddrv4($this->ipaddr->value)) {
                return 'static';
            } elseif ($this->ipaddr->value === 'dhcp') {
                return 'dhcp';
            } else {
                return 'none';
            }
        }

        return $typev4;
    }

    /**
     * Adds custom validation to the `typev4` field.
     * @param string $typev4 The ipaddr value to validate
     * @returns string The validated $typev4 to set.
     * @throws ValidationError When the `typev4` value is invalid.
     */
    public function validate_typev4(string $typev4): string {
        # Do not allow the type to change from 'static' if the DHCP server is enabled on this interface
        if ($typev4 !== 'static' and $this->is_config_enabled(path: "dhcpd/$this->id")) {
            throw new ValidationError(
                message: "Field 'typev4' cannot be changed from 'static' while a DHCP server is enabled on this " .
                    "interface. Disable the DHCP server on this interface before changing 'typev4'.",
                response_id: 'NETWORK_INTERFACE_CANNOT_CHANGE_TYPEV4_WHILE_DHCP_SERVER_ENABLED',
            );
        }

        # Do not allow typev4 to be set to 'none' if there are existing virtual IPs on this interface
        # TODO: Change this to a query when the VirtualIP model is developed
        if ($typev4 === 'none') {
            foreach ($this->get_config('virtualip/vip', []) as $vip) {
                if (is_ipaddrv4($vip['subnet']) and $vip['interface'] === $this->id) {
                    throw new ValidationError(
                        message: "Field 'typev4' cannot be changed to 'none' because there are existing virtual IPs " .
                            'on this interface.',
                        response_id: 'NETWORK_INTERFACE_NO_TYPEV4_NONE_WITH_EXISTING_VIRTUAL_IPS',
                    );
                }
            }
        }

        return $typev4;
    }

    /**
     * Adds custom validation to the `ipaddr` field.
     * @param string $ipaddr The ipaddr value to validate
     * @returns string The validated $ipaddr to set.
     * @throws ValidationError When the `ipaddr` value is invalid.
     */
    public function validate_ipaddr(string $ipaddr): string {
        # For non static values, force `ipaddr` to match `typev4`
        if ($this->typev4->value !== 'static') {
            $ipaddr = $this->typev4->value;
        }

        # Do not allow static values to use keywords
        if (!is_ipaddrv4($ipaddr) and $this->typev4->value === 'static') {
            throw new ValidationError(
                message: 'Interface `ipaddr` must be an IPv4 address when `typev4` is set to `static`',
                response_id: 'INTERFACE_IPADDR_NOT_IPV4_WHEN_STATIC',
            );
        }

        # Ensure this IP address is not in use anywhere else
        if (where_is_ipaddr_configured($ipaddr, $this->id, true, true, $this->subnet->value)) {
            throw new ValidationError(
                message: "Interface `ipaddr` cannot use '$ipaddr' because it is currently in use",
                response_id: 'INTERFACE_IPADDR_ALREADY_IN_USE',
            );
        }

        # When this interfaces 'subnet' is less than 31, ensure the IP is not the network or broadcast address
        if ($this->subnet->value < 31) {
            if ($ipaddr === gen_subnet($ipaddr, $this->subnet->value)) {
                throw new ValidationError(
                    message: "Interface 'ipaddr' cannot use '$ipaddr' because it is the network address",
                    response_id: 'NETWORK_INTERFACE_IPADDR_CANNOT_USE_NETWORK_ADDRESS',
                );
            }
            if ($ipaddr === gen_subnet_max($ipaddr, $this->subnet->value)) {
                throw new ValidationError(
                    message: "Interface 'ipaddr' cannot use '$ipaddr' because it is the broadcast address",
                    response_id: 'NETWORK_INTERFACE_IPADDR_CANNOT_USE_BROADCAST_ADDRESS',
                );
            }
        }

        # Do not allow this IP if it conflicts with an existing static route
        # TODO: replace this with a query when a StaticRoute model is developed
        foreach ($this->get_config(path: 'staticroutes/route', default: []) as $static_route_id => $static_route) {
            $if_cidr = gen_subnet($ipaddr, $this->subnet->value) . '/' . $this->subnet->value;
            if ($static_route['network'] === $if_cidr) {
                throw new ValidationError(
                    message: "Interface 'ipaddr' cannot be used because it conflicts with a static route " .
                        "with ID '$static_route_id'",
                    response_id: 'NETWORK_INTERFACE_IPADDR_CONFLICTS_WITH_STATIC_ROUTE',
                );
            }
        }

        return $ipaddr;
    }

    /**
     * Adds custom validation to the `subnet` field.
     * @param int $subnet The subnet value to validate
     * @returns string The validated $subnet to set.
     * @throws ValidationError When the `subnet` value is invalid.
     */
    public function validate_subnet(int $subnet): int {
        # Require the subnet to be 30 or less when a DHCP server is enabled on this interface
        if ($subnet > 30 and $this->is_config_enabled(path: "dhcpd/$this->id")) {
            throw new ValidationError(
                message: "Field 'subnet' cannot be greater than 30 while a DHCP server is enabled on this interface'. " .
                    "Disable the DHCP server on this interface before changing the 'subnet' value.",
                response_id: 'NETWORK_INTERFACE_SUBNET_MAXIMUM_EXCEEDED_WHILE_DHCP_SERVER_ENABLED',
            );
        }

        return $subnet;
    }

    /**
     * Populates the `typev6` field if a value is not present
     * @param string $typev6 The typev6 value to override
     * @returns string The overridden $typev6 value to set.
     * @throws ValidationError When the `$typev6` value is invalid.
     */
    public function from_internal_typev6(mixed $typev6): string {
        # Only populate this value if a value was not already set
        if (is_null($typev6)) {
            # Set the type to static if an IP address is set
            if (is_ipaddrv6($this->ipaddrv6->value)) {
                return 'staticv6';
            } elseif ($this->ipaddrv6->value === 'dhcp6') {
                return 'dhcp6';
            } elseif ($this->ipaddrv6->value === 'slaac') {
                return 'slaac';
            } elseif ($this->ipaddrv6->value === '6rd') {
                return '6rd';
            } elseif ($this->ipaddrv6->value === '6to4') {
                return '6to4';
            } elseif ($this->ipaddrv6->value === 'track6') {
                return 'track6';
            } else {
                return 'none';
            }
        }

        return $typev6;
    }

    /**
     * Adds custom validation to the `typev6` field.
     * @param string $typev6 The ipaddr value to validate
     * @returns string The validated $typev6 to set.
     * @throws ValidationError When the `typev6` value is invalid.
     */
    public function validate_typev6(string $typev6): string {
        # Do not allow the type to change from 'staticv6' or 'track6' if the DHCPv6 server is enabled on this interface
        if (!in_array($typev6, ['staticv6', 'track6'])) {
            # TODO: Change this to use a query when the DHCPServer6 model is developed
            if ($this->is_config_enabled(path: "dhcpdv6/$this->id")) {
                throw new ValidationError(
                    message: "Field 'typev6' cannot be changed while a DHCPv6 server is enabled on this " .
                        "interface. Disable the DHCPv6 server on this interface before changing 'typev6'.",
                    response_id: 'NETWORK_INTERFACE_CANNOT_CHANGE_TYPEV6_WHILE_DHCP_SERVER_ENABLED',
                );
            }

            # Do not allow the type to change from 'staticv6' or 'track6' if the DHCPv6 RA server is enabled
            # TODO: Change this to use a query when the DHCPServer6 model is developed
            if ($this->get_config(path: "dhcpdv6/$this->id/ramode", default: 'disabled') !== 'disabled') {
                throw new ValidationError(
                    message: "Field 'typev6' cannot be changed while DHCPv6 RAs are enabled on this " .
                        "interface. Disable the DHCPv6 RAs on this interface before changing 'typev6'.",
                    response_id: 'NETWORK_INTERFACE_CANNOT_CHANGE_TYPEV6_WHILE_DHCP_RAS_ENABLED',
                );
            }
        }

        # Only allow one interface's typev6 value to be 6to4
        $if_6to4_query = $this->query(['id__except' => $this->id, 'typev6' => '6to4']);
        if ($if_6to4_query->exists()) {
            throw new ValidationError(
                message: "Field 'typev6' cannot use '6to4' because it is already in use by " .
                    "interface with ID '{$if_6to4_query->first()->id}'",
                response_id: 'NETWORK_INTERFACE_TYPEV6_6TO4_ALREADY_IN_USE',
            );
        }

        # Do not allow typev6 to be set to 'none' if there are existing virtual IPs on this interface
        # TODO: Change this to a query when the VirtualIP model is developed
        if ($typev6 === 'none') {
            foreach ($this->get_config('virtualip/vip', []) as $vip) {
                if (is_ipaddrv6($vip['subnet']) and $vip['interface'] === $this->id) {
                    throw new ValidationError(
                        message: "Field 'typev6' cannot be changed to 'none' because there are existing virtual IPs " .
                            'on this interface.',
                        response_id: 'NETWORK_INTERFACE_NO_TYPEV6_NONE_WITH_EXISTING_VIRTUAL_IPS',
                    );
                }
            }
        }

        return $typev6;
    }

    /**
     * Adds custom validation to the `ipaddrv6` field.
     * @param string $ipaddrv6 The ipaddr value to validate
     * @returns string The validated $ipaddrv6 to set.
     * @throws ValidationError When the `ipaddrv6` value is invalid.
     */
    public function validate_ipaddrv6(string $ipaddrv6): string {
        # Only allow `dhcp6` as a value if the type is also `dhcp6`
        if ($ipaddrv6 === 'dhcp6' and $this->typev6->value !== 'dhcp6') {
            throw new ValidationError(
                message: 'Interface `ipaddrv6` cannot be `dhcp6` unless `typev6` is also `dhcp`',
                response_id: 'INTERFACE_IPADDRV6_IS_DHCP6_WITH_NON_DHCP6_TYPEV6',
            );
        }
        # Only allow `slaac` as a value if the type is also `slaac`
        if ($ipaddrv6 === 'slaac' and $this->typev6->value !== 'slaac') {
            throw new ValidationError(
                message: 'Interface `ipaddrv6` cannot be `slaac` unless `typev6` is also `slaac`',
                response_id: 'INTERFACE_IPADDRV6_IS_SLAAC_WITH_NON_SLAAC_TYPEV6',
            );
        }
        # Only allow `6rd` as a value if the type is also `6rd`
        if ($ipaddrv6 === '6rd' and $this->typev6->value !== '6rd') {
            throw new ValidationError(
                message: 'Interface `ipaddrv6` cannot be `6rd` unless `typev6` is also `6rd`',
                response_id: 'INTERFACE_IPADDRV6_IS_6RD_WITH_NON_6RD_TYPEV6',
            );
        }
        # Only allow `6to4` as a value if the type is also `6to4`
        if ($ipaddrv6 === '6to4' and $this->typev6->value !== '6to4') {
            throw new ValidationError(
                message: 'Interface `ipaddrv6` cannot be `6to4` unless `typev6` is also `6to4`',
                response_id: 'INTERFACE_IPADDRV6_IS_6TO4_WITH_NON_6TO4_TYPEV6',
            );
        }
        # Only allow `track6` as a value if the type is also `track6`
        if ($ipaddrv6 === 'track6' and $this->typev6->value !== 'track6') {
            throw new ValidationError(
                message: 'Interface `ipaddrv6` cannot be `track6` unless `typev6` is also `track6`',
                response_id: 'INTERFACE_IPADDRV6_IS_TRACK6_WITH_NON_TRACK6_TYPEV6',
            );
        }

        # Ensure this IPv6 address is not in use anywhere else
        if (where_is_ipaddr_configured($ipaddrv6, $this->id, true, true, $this->subnetv6->value)) {
            throw new ValidationError(
                message: "Interface `ipaddrv6` cannot use '$ipaddrv6' because it is currently in use",
                response_id: 'INTERFACE_IPADDRV6_ALREADY_IN_USE',
            );
        }

        # Do not allow this IP if it conflicts with an existing static route
        # TODO: replace this with a query when a StaticRoute model is developed
        foreach ($this->get_config(path: 'staticroutes/route', default: []) as $static_route_id => $static_route) {
            $if_cidr = gen_subnetv6($ipaddrv6, $this->subnetv6->value) . '/' . $this->subnetv6->value;
            if ($static_route['network'] === $if_cidr) {
                throw new ValidationError(
                    message: "Interface 'ipaddrv6' cannot be used because it conflicts with a static route " .
                        "with ID '$static_route_id'",
                    response_id: 'NETWORK_INTERFACE_IPADDRV6_CONFLICTS_WITH_STATIC_ROUTE',
                );
            }
        }

        return $ipaddrv6;
    }

    /**
     * Overwrites the default get_next_id() method to return the next pfSense interface ID (e.g. 'wan', 'lan', 'opt1').
     * @return string The next available pfSense interface ID
     */
    public function get_next_id(): string {
        # Use `lan` if it doesn't already exist
        if (!$this->query(['id' => 'lan'])->exists()) {
            return 'lan';
        }

        # Otherwise, loop until we find an available opt interface ID
        foreach (range(1, 65535) as $count) {
            # Use this opt if it doesn't already exist
            if (!$this->query(['id' => "opt$count"])->exists()) {
                return "opt$count";
            }
        }

        throw new ServerError(
            message: 'Could not determine the next available interface ID',
            response_id: 'COULD_NOT_FIND_NEXT_INTERFACE_ID',
        );
    }

    /**
     * Determines all supported media options for this interface.
     * @return array An array where the key is the speed (e.g. 1000baseT) and the value is an array of duplex options
     */
    public function get_supported_media(): array {
        # Variables
        $media_options = [];
        $if = $this->if->value;

        # Run ifconfig to determine what media and media options are supported
        exec("/sbin/ifconfig -m $if | grep \"media \"", $media_options_output);

        # Loop through each supported media from the ifconfig output
        foreach ($media_options_output as $media_option) {
            # Locate lines beginning with `media `
            preg_match('/media (.*)/', $media_option, $media_line);

            # Check if there is speed AND duplex
            if (preg_match('/(.*) mediaopt (.*)/', $media_line[1], $matches)) {
                $media_options[$matches[1]][] = $matches[2];
            }
            # Otherwise only include the speed
            else {
                $media_options[$media_line[1]][] = '';
            }
        }

        # Always allow an empty string to assume the default speed and/or duplex
        $media_options[''] = [''];

        return $media_options;
    }

    /**
     * Serializes this interface configuration and places it in the pending interface
     * changes file at /tmp/.interfaces.apply
     */
    public function serialize_changes() {
        # Local variables
        global $g;
        $pending_ifs = [];

        # Check for any existing pending interface changes
        if (file_exists($g['tmp_path'] . '/.interfaces.apply')) {
            $pending_ifs = unserialize(file_get_contents($g['tmp_path'] . '/.interfaces.apply'));
        }

        # Set the current validated interface as a pending interface change
        $pending_ifs[$this->id] = [];

        # When we are updating an existing interface, store the original interface
        if ($this->initial_object) {
            $pending_ifs[$this->id]['ifcfg'] = $this->initial_object->to_internal();
            $pending_ifs[$this->id]['ifcfg']['realif'] = $this->initial_object->if->value;
        }
        # Otherwise, for new interface creations, store this interface
        else {
            $pending_ifs[$this->id]['ifcfg'] = $this->to_internal();
            $pending_ifs[$this->id]['ifcfg']['realif'] = $this->if->value;
        }

        file_put_contents($g['tmp_path'] . '/.interfaces.apply', serialize($pending_ifs));
        configure_cron();
    }

    /**
     * Ensures pending interfaces are serialized and stored in the pending interfaces file before they are applied.
     * This method is automatically called after interface creations but before they are applied.
     */
    public function pre_apply_create(): void {
        $this->serialize_changes();
    }

    /**
     * Defines how interfaces should be applied after they are create. This method is automatically called after
     * interfaces are created if immediate application is requested.
     */
    public function apply_create(): void {
        (new InterfaceApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Ensures pending interfaces are serialized and stored in the pending interfaces file before they are applied.
     * This method is automatically called after interface updates but before they are applied.
     */
    public function pre_apply_update(): void {
        $this->serialize_changes();
    }

    /**
     * Defines how interfaces should be applied after they are updated. This method is automatically called after
     * interfaces are updated if immediate application is requested.
     */
    public function apply_update(): void {
        (new InterfaceApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Defines how interfaces should be deleted. This method should not be called directly, instead it should be
     * called using the delete() method.
     */
    public function _delete(): void {
        # Do not allow deletion of WAN interface
        if ($this->id === 'wan') {
            throw new ValidationError(
                message: 'Interface `wan` cannot be deleted.',
                response_id: 'NETWORK_INTERFACE_WAN_CANNOT_BE_DELETED',
            );
        }
        # Do not allow deletion if interface is in group
        # TODO: replace this with a Model query when the InterfaceGroup Model is developed
        elseif (link_interface_to_group($this->id)) {
            throw new ValidationError(
                message: "Interface `$this->id` cannot be deleted because it is in use by an interface group.",
                response_id: 'NETWORK_INTERFACE_IN_USE_BY_INTERFACE_GROUP',
            );
        }
        # Do not allow deletion if interface is in bridge
        # TODO: replace this with a Model query when the InterfaceBridge Model is developed
        elseif (link_interface_to_bridge($this->id)) {
            throw new ValidationError(
                message: "Interface `$this->id` cannot be deleted because it is in use by an interface bridge.",
                response_id: 'NETWORK_INTERFACE_IN_USE_BY_INTERFACE_BRIDGE',
            );
        }
        # Do not allow interface to be deleted if it is assigned GRE interface
        # TODO: replace this with a Model query when the InterfaceGRE Model is developed
        elseif (link_interface_to_tunnelif($this->id, 'gre')) {
            throw new ValidationError(
                message: "Interface `$this->id` cannot be deleted because it is in use by an GRE tunnel.",
                response_id: 'NETWORK_INTERFACE_IN_USE_BY_INTERFACE_GRE',
            );
        }
        # Do not allow interface to be deleted if it is assigned GIF interface
        # TODO: replace this with a Model query when the InterfaceGIF Model is developed
        elseif (link_interface_to_tunnelif($this->id, 'gif')) {
            throw new ValidationError(
                message: "Interface `$this->id` cannot be deleted because it is in use by an GIF interface.",
                response_id: 'NETWORK_INTERFACE_IN_USE_BY_INTERFACE_GIF',
            );
        }
        # Do not allow interface to be deleted if it has a traffic queue assigned
        # TODO: replace this with a Model query when the TrafficShaperQueue Model is developed
        elseif (interface_has_queue($this->id)) {
            throw new ValidationError(
                message: "Interface `$this->id` cannot be deleted because it is in use by a traffic shaper queue.",
                response_id: 'NETWORK_INTERFACE_IN_USE_BY_TRAFFIC_SHAPER_QUEUE',
            );
        }
        # Otherwise, run through the steps to delete the interface
        else {
            # Bring down this interface and remove it from config
            $real_interface = get_real_interface($this->id);
            interface_bring_down($this->id);
            $this->del_config("$this->config_path/$this->id");

            # Remove DHCP config for interface
            if (is_array($this->get_config("dhcpd/{$this->id}"))) {
                $this->del_config("dhcpd/{$this->id}");
                services_dhcpd_configure('inet');
            }
            # Remove interface config for dhcp6
            if (is_array($this->get_config("dhcpdv6/{$this->id}"))) {
                $this->del_config("dhcpdv6/{$this->id}");
                services_dhcpd_configure('inet6');
            }
            # Remove ACL for interface
            foreach ($this->get_config('filter/rule', []) as $x => $rule) {
                if ($rule['interface'] == $this->id) {
                    $this->del_config("filter/rule/{$x}");
                }
            }
            # Remove NAT config for interface
            foreach ($this->get_config('nat/rule', []) as $x => $rule) {
                if ($rule['interface'] == $this->id) {
                    $this->del_config("nat/rule/{$x}");
                }
            }

            # Disable DHCP if last interface
            if (count($this->get_config('interfaces', [])) <= 1 && $this->get_config('dhcpd/wan')) {
                $this->del_config('dhcpd/wan');
            }

            # Update VLAN assignments
            link_interface_to_vlans($real_interface, 'update');

            # Write the changes to config
            $this->write_config("Deleted $this->verbose_name via API");
        }
    }

    /**
     * Gathers information about all real interfaces and returns them in an array.
     * @return array a list of all interfaces along with their attributes
     */
    public static function get_all_avail_interfaces(): array {
        return AvailableInterface::get_all_avail_interfaces();
    }

    /**
     * Checks the current IPv4 address of this interface. This works for both static and dynimcally assigned IPs.
     */
    public function get_current_ipv4(): string|null {
        return get_interface_ip($this->id);
    }

    /**
     * Checks the current subnet (v4) of this interface. This works for both static and dynimcally assigned IPs.
     */
    public function get_current_subnetv4(): int|null {
        return get_interface_subnet($this->id);
    }

    /**
     * Checks the current IPv6 address of this interface. This works for both static and dynimcally assigned IPs.
     */
    public function get_current_ipv6(): string|null {
        return get_interface_ipv6($this->id);
    }

    /**
     * Checks the current subnet (v6) of this interface. This works for both static and dynamically assigned IPs.
     */
    public function get_current_subnetv6(): int|null {
        return get_interface_subnetv6($this->id);
    }

    /**
     * Obtains the current network address of the subnet assigned to this interface.
     */
    public function get_network_ipv4(): string|null {
        $ip = $this->get_current_ipv4();
        $subnet = $this->get_current_subnetv4();
        return gen_subnetv4($ip, $subnet);
    }

    /**
     * Obtains the current broadcast address of the subnet assigned to this interface.
     */
    public function get_broadcast_ipv4(): string|null {
        $ip = $this->get_current_ipv4();
        $subnet = $this->get_current_subnetv4();
        return gen_subnetv4_max($ip, $subnet);
    }

    /**
     * Checks if a given IPv4 address is within this interface's configured network CIDR.
     * @param string $ipaddr The IPv4 address to check.
     * @return bool Returns true if the $ipaddr is an address within this interface's configured IPv4 CIDR, returns
     * false otherwise.
     */
    public function is_ipv4_in_cidr(string $ipaddr): bool {
        return (bool) ip_in_subnet($ipaddr, $this->get_current_ipv4() . '/' . $this->get_current_subnetv4());
    }

    /**
     * Obtains the string representation for this interface as specified in the REST API settings
     * 'represent_interfaces_as' field.
     */
    public function represented_as(): string {
        $api_settings = new RESTAPISettings();
        return match ($api_settings->represent_interfaces_as->value) {
            'descr' => $this->descr->value,
            'if' => $this->if->value,
            default => $this->id,
        };
    }
}
