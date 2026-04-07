<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DHCPServerApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\MACAddressValidator;

/**
 * Defines a Model that configures additional address pools for a parent DHCPServer.
 */
class DHCPServerAddressPool extends Model {
    public StringField $range_from;
    public StringField $range_to;
    public StringField $domain;
    public StringField $mac_allow;
    public StringField $mac_deny;
    public StringField $domainsearchlist;
    public IntegerField $defaultleasetime;
    public IntegerField $maxleasetime;
    public StringField $gateway;
    public StringField $dnsserver;
    public StringField $winsserver;
    public StringField $ntpserver;
    public BooleanField $ignorebootp;
    public BooleanField $ignoreclientuids;
    public StringField $denyunknown;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define Model attributes
        $this->parent_model_class = 'DHCPServer';
        $this->config_path = 'pool';
        $this->subsystem = 'dhcpd';
        $this->many = true;

        $this->range_from = new StringField(
            required: true,
            maximum_length: 15,
            internal_name: 'from',
            internal_namespace: 'range',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Range From',
            help_text: 'The starting IP address for this address pool. This address must be less than or equal ' .
                'to the `range_to` field.',
        );
        $this->range_to = new StringField(
            required: true,
            maximum_length: 15,
            internal_name: 'to',
            internal_namespace: 'range',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Range To',
            help_text: 'The ending IP address for the this address pool. This address must be greater than or equal ' .
                'to the `range_to` field.',
        );
        $this->domain = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 255,
            verbose_name: 'Domain',
            help_text: 'The domain to be assigned via DHCP.',
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

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Provides extra validation to this entire Model object.
     */
    public function validate_extra(): void {
        # Check if this pool's range overlaps with another model object.
        $overlapped_model = $this->get_range_overlap();

        # Do not allow this DHCPServerAddressPool to overlap with other objects
        if ($overlapped_model) {
            throw new ConflictError(
                message: "This DHCP server address pool overlaps with $overlapped_model->verbose_name " .
                    "object with ID `$overlapped_model->id`. Adjust your `range_from` and `range_to` values.",
                response_id: 'DHCP_SERVER_ADDRESS_POOL_OVERLAPS_EXISTING_OBJECT',
            );
        }
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
        $interface = NetworkInterface::query(['id' => $this->parent_id])->first();

        # Do not allow this IP to be the network address
        if ($interface->get_network_ipv4() == $range_from) {
            throw new ValidationError(
                message: "DHCP server `range_from` cannot be the interface subnet's network address.",
                response_id: 'DHCP_SERVER_ADDRESS_POOL_RANGE_FROM_CANNOT_BE_NETWORK_ADDRESS',
            );
        }

        # Do not allow the `range_from` to be greater `range_to`
        if (ip_greater_than($range_from, $this->range_to->value)) {
            throw new ValidationError(
                message: 'DHCP server `range_from` cannot be greater than `range_to`.',
                response_id: 'DHCP_SERVER_ADDRESS_POOL_RANGE_FROM_CANNOT_BE_GREATER_THAN_RANGE_TO',
            );
        }

        # Do not allow the `range_from` IP to be outside the interface's configured subnet
        if (!is_inrange_v4($range_from, $interface->get_network_ipv4(), $interface->get_broadcast_ipv4())) {
            throw new ValidationError(
                message: "DHCP server `range_from` must lie within the interface's subnet.",
                response_id: 'DHCP_SERVER_ADDRESS_POOL_RANGE_FROM_OUTSIDE_OF_SUBNET',
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
        $interface = NetworkInterface::query(['id' => $this->parent_id])->first();

        # Do not allow this IP to be the network address
        if ($interface->get_broadcast_ipv4() === $range_to) {
            throw new ValidationError(
                message: "DHCP server `range_to` cannot be the interface subnet's broadcast address.",
                response_id: 'DHCP_SERVER_ADDRESS_POOL_RANGE_TO_CANNOT_BE_BROADCAST_ADDRESS',
            );
        }

        # Do not allow the `range_to` IP to be outside the interface's configured subnet
        if (!is_inrange_v4($range_to, $interface->get_network_ipv4(), $interface->get_broadcast_ipv4())) {
            throw new ValidationError(
                message: "DHCP server `range_to` must lie within the interface's subnet.",
                response_id: 'DHCP_SERVER_ADDRESS_POOL_RANGE_TO_OUTSIDE_OF_SUBNET',
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
                response_id: 'DHCP_SERVER_ADDRESS_POOL_MAX_LEASE_TIME_LESS_THAN_DEFAULT',
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
        $interface = NetworkInterface::query(['id' => $this->parent_id])->first();

        # Do not allow non-empty gateway values that are not within the interface's subnet
        if ($gateway and !$interface->is_ipv4_in_cidr($gateway)) {
            throw new ValidationError(
                message: "DHCP server `gateway` must be within interface's subnet.",
                response_id: 'DHCP_SERVER_ADDRESS_POOL_GATEWAY_NOT_WITHIN_SUBNET',
            );
        }

        return $gateway;
    }

    /**
     * Checks if this pool's address range overlaps with reserved IPs such as static mappings, pools and virtual IPs.
     * @return Model|null Returns the Model object that this range overlaps with, returns
     * null if the range did not overlap with an existing Model object.
     */
    private function get_range_overlap(): ?Model {
        # Variables
        $range_from = $this->range_from->value;
        $range_to = $this->range_to->value;
        $parent_range_from = $this->parent_model->range_from->value;
        $parent_range_to = $this->parent_model->range_to->value;

        # Ensure this IP address is not reserved by the primary DHCPServer address pool
        if (
            is_inrange_v4($parent_range_from, $range_from, $range_to) or
            is_inrange_v4($parent_range_to, $range_from, $range_to)
        ) {
            return $this;
        }

        # Ensure range does not overlap with existing static mappings
        $static_mappings = DHCPServerStaticMapping::query(parent_id: $this->parent_id);
        foreach ($static_mappings->model_objects as $static_mapping) {
            if (is_inrange_v4($static_mapping->ipaddr->value, $range_from, $range_to)) {
                return $static_mapping;
            }
        }

        # Ensure range does not overlap with existing address pools `range_from` or `range_to` addresses
        $pools = DHCPServerAddressPool::query(parent_id: $this->parent_id, id__except: $this->id);
        foreach ($pools->model_objects as $pool) {
            if (
                is_inrange_v4($pool->range_from->value, $range_from, $range_to) or
                is_inrange_v4($pool->range_to->value, $range_from, $range_to)
            ) {
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
     * Applies the pending DHCP server changes.
     */
    public function apply(): void {
        (new DHCPServerApplyDispatcher(async: $this->async))->spawn_process();
    }
}
