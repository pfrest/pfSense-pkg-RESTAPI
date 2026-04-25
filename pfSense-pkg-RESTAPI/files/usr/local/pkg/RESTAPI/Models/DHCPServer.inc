<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Dispatchers\DHCPServerApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\MACAddressValidator;

/**
 * Defines a Model that interacts with the DHCP server for a given interface.
 */
class DHCPServer extends Model {
    public InterfaceField $interface;
    public BooleanField $enable;
    public StringField $range_from;
    public StringField $range_to;
    public StringField $domain;
    public StringField $failover_peerip;
    public StringField $mac_allow;
    public StringField $mac_deny;
    public StringField $domainsearchlist;
    public IntegerField $defaultleasetime;
    public IntegerField $maxleasetime;
    public StringField $gateway;
    public StringField $dnsserver;
    public StringField $winsserver;
    public StringField $ntpserver;
    public BooleanField $staticarp;
    public BooleanField $ignorebootp;
    public BooleanField $ignoreclientuids;
    public BooleanField $nonak;
    public BooleanField $disablepingcheck;
    public BooleanField $dhcpleaseinlocaltime;
    public BooleanField $statsgraph;
    public StringField $denyunknown;
    public NestedModelField $pool;
    public NestedModelField $numberoptions;
    public NestedModelField $staticmap;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define Model attributes
        $this->config_path = 'dhcpd';
        $this->id_type = 'string';
        $this->many = true;
        $this->subsystem = 'dhcpd';
        $this->update_strategy = 'merge';
        $this->verbose_name = 'DHCP Server';

        # Define Model Fields
        $this->interface = new InterfaceField(
            required: true,
            representation_only: true,
            verbose_name: 'Interface',
            help_text: 'The interface to configure the DHCP server for. This field is only necessary when you want' .
                'to change the interface (ID) of an existing DHCP server, or you are replacing all DHCP server objects ' .
                'with a new configuration. Note that specifying an interface in this field will update the ID of the ' .
                'DHCP server to match the interface specified here. Leaving this field empty will retain the ' .
                'existing interface.',
        );
        $this->enable = new BooleanField(
            default: false,
            verbose_name: 'Enable',
            help_text: 'Enable the DHCP server for this interface.',
        );
        $this->range_from = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 15,
            internal_name: 'from',
            internal_namespace: 'range',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Range From',
            help_text: 'The starting IP address for the primary DHCP pool. This address must be less than or equal ' .
                'to the `range_to` field.',
        );
        $this->range_to = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 15,
            internal_name: 'to',
            internal_namespace: 'range',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Range To',
            help_text: 'The ending IP address for the primary DHCP pool. This address must be greater than or equal ' .
                'to the `range_to` field.',
        );
        $this->domain = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 255,
            verbose_name: 'Domain',
            help_text: 'The domain to be assigned via DHCP.',
        );
        $this->failover_peerip = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 255,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Failover Peer IP',
            help_text: 'The interface IP address of the other firewall (failover peer) in this subnet. Leave ' .
                'empty to disable failover peering.',
        );
        $this->mac_allow = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            maximum_length: 17,
            validators: [new MACAddressValidator()],
            verbose_name: 'MAC Allow',
            help_text: 'MAC addresses this DHCP server is allowed to provide leases for.',
        );
        $this->mac_deny = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            maximum_length: 17,
            validators: [new MACAddressValidator()],
            verbose_name: 'MAC Deny',
            help_text: 'MAC addresses this DHCP server is not allowed to provide leases for.',
        );
        $this->domainsearchlist = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            maximum_length: 255,
            delimiter: ';',
            validators: [new IPAddressValidator(allow_ipv4: false, allow_ipv6: false, allow_fqdn: true)],
            verbose_name: 'Domain Search List',
            help_text: 'The domain search list to provide via DHCP.',
        );
        $this->defaultleasetime = new IntegerField(
            default: 7200,
            allow_null: true,
            minimum: 60,
            verbose_name: 'Default Lease Time',
            help_text: 'The default DHCP lease validity period (in seconds). This is used for clients that do not ask ' .
                'for a specific expiration time.',
        );
        $this->maxleasetime = new IntegerField(
            default: 86400,
            allow_null: true,
            minimum: 60,
            verbose_name: 'Max Lease Time',
            help_text: 'The maximum DHCP lease validity period (in seconds) a client can request.',
        );
        $this->gateway = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 15,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false, allow_keywords: ['none'])],
            verbose_name: 'Gateway',
            help_text: 'The gateway IPv4 address to provide via DHCP. This is only necessary if you are not using ' .
                "the interface's IP as the gateway. Specify `none` for no gateway assignment.",
        );
        $this->dnsserver = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            many_maximum: 4,
            maximum_length: 15,
            delimiter: null,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'DNS Servers',
            help_text: 'The DNS servers to provide via DHCP. Leave empty to default to system nameservers.',
        );
        $this->winsserver = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            many_maximum: 2,
            maximum_length: 15,
            delimiter: null,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'WINS Servers',
            help_text: 'The WINS servers to provide via DHCP.',
        );
        $this->ntpserver = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            many_maximum: 4,
            maximum_length: 256,
            delimiter: null,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false, allow_fqdn: true)],
            verbose_name: 'NTP Servers',
            help_text: 'The NTP servers to provide via DHCP.',
        );
        $this->staticarp = new BooleanField(
            default: false,
            verbose_name: 'Static ARP',
            help_text: 'Assign static ARP entries for DHCP leases provided by this server.',
        );
        $this->ignorebootp = new BooleanField(
            default: false,
            verbose_name: 'Ignore BOOTP',
            help_text: 'Force this DHCP server to ignore BOOTP queries.',
        );
        $this->ignoreclientuids = new BooleanField(
            default: false,
            verbose_name: 'Ignore Client UIDs',
            help_text: 'Prevent recording a unique identifier (UID) in client lease data if present in the client ' .
                'DHCP request. This option may be useful when a client can dual boot using different client ' .
                'identifiers but the same hardware (MAC) address. Note that the resulting server behavior violates ' .
                'the official DHCP specification.',
        );
        $this->nonak = new BooleanField(
            default: false,
            verbose_name: 'No NAK',
            help_text: 'Ignore denied clients rather than reject. This option is not compatible with failover and ' .
                'cannot be enabled when a Failover Peer IP address is configured.',
        );
        $this->disablepingcheck = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Disable Ping Check',
            help_text: 'Prevent the DHCP server from sending a ping to the address being assigned, where if no ' .
                'response has been heard, it assigns the address.',
        );
        $this->dhcpleaseinlocaltime = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'DHCP Lease in Local Time',
            help_text: 'Display the DHCP lease times in local time instead of UTC.',
        );
        $this->statsgraph = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Statistics Graph',
            help_text: 'Enable adding DHCP lease statistics to the pfSense Monitoring graphs.',
        );
        $this->denyunknown = new StringField(
            default: null,
            choices: ['enabled', 'class'],
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Deny Unknown Clients',
            help_text: 'Define how to handle unknown clients requesting DHCP leases. When set to `null`, any DHCP ' .
                'client will get an IP address within this scope/range on this interface. If set to `enabled`, ' .
                'any DHCP client with a MAC address listed in a static mapping on any scope(s)/interface(s) will ' .
                'get an IP address. If set to `class`, only MAC addresses listed in static mappings on this interface ' .
                'will get an IP address within this scope/range.',
        );
        $this->pool = new NestedModelField(
            model_class: 'DHCPServerAddressPool',
            default: [],
            allow_empty: true,
            verbose_name: 'Pool',
            help_text: 'Additional address pools applied to this DHCP server.',
        );
        $this->numberoptions = new NestedModelField(
            model_class: 'DHCPServerCustomOption',
            default: [],
            allow_empty: true,
            verbose_name: 'Custom DHCP Options',
            help_text: 'The custom DHCP options to apply to this DHCP server.',
        );
        $this->staticmap = new NestedModelField(
            model_class: 'DHCPServerStaticMapping',
            default: [],
            allow_empty: true,
            verbose_name: 'Static Mappings',
            help_text: 'Static mappings applied to this DHCP server.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal `interface` field value since it is not stored in the config.
     * @return string The internal interface ID of the DHCPServer.
     */
    public function from_internal_interface(): string {
        return $this->interface->_from_internal($this->id);
    }

    /**
     * Provides extra validation for this entire Model object.
     */
    public function validate_extra(): void {
        # Do not allow this DHCPServers primary address pool to overlap with other objects
        $overlapped_model = $this->get_range_overlap($this->range_from->value, $this->range_to->value);
        if ($overlapped_model) {
            throw new ConflictError(
                message: "This DHCP server's primary address pool overlaps with $overlapped_model->verbose_name " .
                    "object with ID `$overlapped_model->id`. Adjust your `range_from` and `range_to` values.",
                response_id: 'DHCP_SERVER_PRIMARY_POOL_OVERLAPS_EXISTING_OBJECT',
            );
        }
    }

    /**
     * Provides extra validation for the `enable` field.
     * @param bool $enable The incoming `enable` field value to validate.
     * @return bool The validated `enable` field value to be assigned.
     * @throws ValidationError When `enable` is `true` but interface associated with `id` does not have a static IP.
     * @throws ValidationError When `enable` is `true`, but there is a DHCP relay running on this interface.
     */
    public function validate_enable(bool $enable): bool {
        # Get the interface associated with this DHCP server
        $interface = NetworkInterface::query(['id' => $this->id])->first();

        # Do not allow the DHCP server to be enabled if the interface does not have a static IP address assigned
        if ($enable and $interface->typev4->value !== 'static') {
            throw new ValidationError(
                message: "DHCP server cannot be enabled because interface `{$this->id}` does not have a static IPv4 " .
                    'address assigned.',
                response_id: 'DHCP_SERVER_CANNOT_ENABLE_WITHOUT_STATIC_IPV4',
            );
        }

        # Do not allow the DHCP server to be enabled if any interface is running a DHCP relay
        $dhcp_relay = new DHCPRelay();
        if ($dhcp_relay->enable->value) {
            throw new ValidationError(
                message: 'DHCP server cannot be enabled while the DHCP Relay is enabled.',
                response_id: 'DHCP_SERVER_CANNOT_BE_ENABLED_WITH_DHCP_RELAY',
            );
        }

        return $enable;
    }

    /**
     * Provides extra validation for the `range_from` field.
     * @param string $range_from The incoming `range_from` field value to validate.
     * @return string The validated `range_from` field value to be assigned.
     * @throws ValidationError When `range_from` is the interface's network address.
     * @throws ValidationError When `range_from` is not in the interface's subnet.
     * @throws ValidationError When `range_from` is greater than the `range_to` IP.
     */
    public function validate_range_from(string $range_from): string {
        # Get the parent interface for this DHCP server
        $interface = NetworkInterface::query(['id' => $this->id])->first();

        # Do not allow this IP to be the network address
        if ($interface->get_network_ipv4() == $range_from) {
            throw new ValidationError(
                message: "DHCP server `range_from` cannot be the interface subnet's network address.",
                response_id: 'DHCP_SERVER_RANGE_FROM_CANNOT_BE_NETWORK_ADDRESS',
            );
        }

        # Do not allow the `range_from` to be greater `range_to`
        if (ip_greater_than($range_from, $this->range_to->value)) {
            throw new ValidationError(
                message: 'DHCP server `range_from` cannot be greater than `range_to`.',
                response_id: 'DHCP_SERVER_RANGE_FROM_CANNOT_BE_GREATER_THAN_RANGE_TO',
            );
        }

        # Do not allow the `range_from` IP to be outside the interface's configured subnet
        if (!$interface->is_ipv4_in_cidr($range_from)) {
            throw new ValidationError(
                message: "DHCP server `range_from` must lie within the interface's subnet.",
                response_id: 'DHCP_SERVER_RANGE_FROM_OUTSIDE_OF_SUBNET',
            );
        }

        return $range_from;
    }

    /**
     * Provides extra validation for the `range_from` field.
     * @param string $range_to The incoming `range_from` field value to validate.
     * @return string The validated `range_from` field value to be assigned.
     * @throws ValidationError When `range_from` is the interface's network address.
     * @throws ValidationError When `range_from` is not in the interface's subnet.
     * @throws ValidationError When `range_from` is greater than the `range_to` IP.
     */
    public function validate_range_to(string $range_to): string {
        # Get the parent interface for this DHCP server
        $interface = NetworkInterface::query(['id' => $this->id])->first();

        # Do not allow this IP to be the network address
        if ($interface->get_broadcast_ipv4() === $range_to) {
            throw new ValidationError(
                message: "DHCP server `range_to` cannot be the interface subnet's broadcast address.",
                response_id: 'DHCP_SERVER_RANGE_FROM_CANNOT_BE_BROADCAST_ADDRESS',
            );
        }

        # Do not allow the `range_to` IP to be outside the interface's configured subnet
        if (!$interface->is_ipv4_in_cidr($range_to)) {
            throw new ValidationError(
                message: "DHCP server `range_to` must lie within the interface's subnet.",
                response_id: 'DHCP_SERVER_RANGE_TO_OUTSIDE_OF_SUBNET',
            );
        }

        return $range_to;
    }

    /**
     * Provides extra validation for the `maxleasetime` field.
     * @param int $maxleasetime The incoming `maxleasetime` field value to validate.
     * @return int The validated `maxleasetime` field value to be assigned.
     * @throws ValidationError When `maxleasetime` is less than the `defaultleasetime`
     */
    public function validate_maxleasetime(int $maxleasetime): int {
        if ($maxleasetime < $this->defaultleasetime->value) {
            throw new ValidationError(
                message: 'DHCP server `maxleasetime` cannot be less than the `defaultleasetime`',
                response_id: 'DHCP_SERVER_MAX_LEASE_TIME_LESS_THAN_DEFAULT',
            );
        }

        return $maxleasetime;
    }

    /**
     * Provides extra validation for the `gateway` field.
     * @param string $gateway The incoming `gateway` field value to validate.
     * @return string The validated `gateway` field value to be assigned.
     * @throws ValidationError When `gateway` is not an IP within the interface's subnet
     */
    public function validate_gateway(string $gateway): string {
        # Get the parent interface for this DHCP server
        $interface = NetworkInterface::query(['id' => $this->id])->first();

        # Do not allow non-empty gateway values that are not within the interface's subnet
        if ($gateway and !$interface->is_ipv4_in_cidr($gateway)) {
            throw new ValidationError(
                message: "DHCP server `gateway` must be within interface's subnet.",
                response_id: 'DHCP_SERVER_GATEWAY_NOT_WITHIN_SUBNET',
            );
        }

        return $gateway;
    }

    /**
     * Provides extra validation for the `nonak` field.
     * @param bool $nonak The incoming `nonak` field value to validate.
     * @return bool The validated `nonak` field value to be assigned.
     * @throws ValidationError When `nonak` is `true` and a `failover_peerip` is assigned.
     */
    public function validate_nonak(bool $nonak): bool {
        # Do not allow `nonak` to be enabled if a `failover_peerip` is set.
        if ($nonak and $this->failover_peerip->value) {
            throw new ValidationError(
                'DHCP server `nonak` cannot be enabled while a `failover_peerip` is assigned.',
                response_id: 'DHCP_SERVER_NONAK_WITH_FAILOVER_PEERIP_NOT_ALLOWED',
            );
        }

        return $nonak;
    }

    /**
     * Provides extra validation for the `staticarp` field.
     * @param bool $staticarp The incoming `staticarp` field value to validate.
     * @return bool The validated `staticarp` field value to be assigned.
     * @throws ValidationError When `staticarp` is `true` but there are configured static mappings without IPs.
     */
    public function validate_staticarp(bool $staticarp): bool {
        # Do not allow `staticarp` to be enabled if there are any configured static mappings without IPs
        $static_mappings = DHCPServerStaticMapping::query(parent_id: $this->id);
        foreach ($static_mappings->model_objects as $static_mapping) {
            if ($staticarp and !$static_mapping->ipaddr->value) {
                throw new ValidationError(
                    'DHCP server `staticarp` cannot be enabled while there are DHCP server static mappings ' .
                        'configured without an assigned IP.',
                    response_id: 'DHCP_SERVER_STATICARP_WITH_NO_IP_STATIC_MAPPINGS',
                );
            }
        }

        return $staticarp;
    }

    /**
     * Checks if a given address range overlaps with reserved IPs such as static mappings, pools and virtual IPs.
     * @param string $range_from The start address of the range.
     * @param string $range_to The end address of the range.
     * @return Model|null Returns the Model object that this range overlaps with, returns
     * null if the range did not overlap with an existing Model object.
     */
    private function get_range_overlap(string $range_from, string $range_to): ?Model {
        # Ensure range does not overlap with existing static mappings
        $static_mappings = DHCPServerStaticMapping::query(parent_id: $this->id);
        foreach ($static_mappings->model_objects as $static_mapping) {
            if (is_inrange_v4($static_mapping->ipaddr->value, $range_from, $range_to)) {
                return $static_mapping;
            }
        }

        # Ensure range does not overlap with existing address pools `range_from` or `range_to` addresses
        $pools = DHCPServerAddressPool::query(parent_id: $this->id);
        foreach ($pools->model_objects as $pool) {
            if (is_inrange_v4($pool->range_from->value, $range_from, $range_to)) {
                return $pool;
            }
            if (is_inrange_v4($pool->range_to->value, $range_from, $range_to)) {
                return $pool;
            }
        }

        # Ensure range does not overlap with existing virtual IPs
        $vips = VirtualIP::read_all();
        foreach ($vips->model_objects as $vip) {
            if (is_inrange_v4($vip->subnet->value, $range_from, $range_to)) {
                return $vip;
            }
        }

        return null;
    }

    /**
     * Obtains the next available ID for a new DHCP server object.
     * @return string The interface ID associated with `interface` field value.
     * @note This is not the conventional use case for this method, but it is necessary for the DHCPServer::replace_all()
     * method to associate DHCP server objects with the requested interface.
     */
    public function get_next_id(): string {
        return $this->interface->_to_internal($this->interface->value) ?: 'unknown';
    }

    /**
     * Applies DHCP server changes after replacement of all existing DHCP server objects.
     */
    public function apply_replace_all(ModelSet $initial_objects, ModelSet $new_objects): void {
        # Always configure RRD after replacing all DHCP server objects
        enable_rrd_graphing();

        # Always reset the dhcpd db files after replacing all DHCP server objects
        mwexec('/bin/rm -rf /var/dhcpd/var/db/*');

        (new DHCPServerApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Applies the pending DHCP server changes.
     */
    public function apply(): void {
        # Enable RRD graphing for DHCP if `statsgraph` changed
        if ($this->initial_object->statsgraph->value !== $this->statsgraph->value) {
            enable_rrd_graphing();
        }

        # Remove the existing dhcpd db files if the `failover_peerip` changed
        if ($this->initial_object->failover_peerip->value !== $this->failover_peerip->value) {
            mwexec('/bin/rm -rf /var/dhcpd/var/db/*');
        }

        (new DHCPServerApplyDispatcher(async: $this->async))->spawn_process();
    }
}
