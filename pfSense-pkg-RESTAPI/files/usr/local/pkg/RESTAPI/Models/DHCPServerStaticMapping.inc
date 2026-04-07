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
use RESTAPI\Validators\HostnameValidator;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\MACAddressValidator;

/**
 * Defines a Model that interacts with static DHCP mappings for the DHCP server.
 */
class DHCPServerStaticMapping extends Model {
    public StringField $mac;
    public StringField $ipaddr;
    public StringField $cid;
    public StringField $hostname;
    public StringField $domain;
    public StringField $domainsearchlist;
    public IntegerField $defaultleasetime;
    public IntegerField $maxleasetime;
    public StringField $gateway;
    public StringField $dnsserver;
    public StringField $winsserver;
    public StringField $ntpserver;
    public BooleanField $arp_table_static_entry;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->parent_model_class = 'DHCPServer';
        $this->config_path = 'staticmap';
        $this->subsystem = 'dhcpd';
        $this->many = true;
        $this->sort_by = ['ipaddr'];
        $this->sort_flags = SORT_NATURAL;

        # Define model Fields
        $this->mac = new StringField(
            required: true,
            unique: true,
            validators: [new MACAddressValidator()],
            verbose_name: 'MAC',
            help_text: 'The MAC address of the client this mapping is for.',
        );
        $this->ipaddr = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'IP Address',
            help_text: 'The IP address to assign this client via DHCP.',
        );
        $this->cid = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Cid',
            help_text: 'The client identifier of the client this mapping is for.',
        );
        $this->hostname = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            validators: [new HostnameValidator()],
            verbose_name: 'Hostname',
            help_text: 'The hostname to assign this client via DHCP.',
        );
        $this->domain = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 255,
            verbose_name: 'Domain',
            help_text: 'The domain to be assigned via DHCP.',
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
            help_text: 'The maximum DHCP lease validity period (in seconds) this client can request.',
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
            many_maximum: 3,
            maximum_length: 256,
            delimiter: null,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false, allow_fqdn: true)],
            verbose_name: 'NTP Servers',
            help_text: 'The NTP servers to provide via DHCP.',
        );
        $this->arp_table_static_entry = new BooleanField(
            default: false,
            verbose_name: 'ARP Table Static Entry',
            help_text: 'Assign a static ARP entry for this static mapping.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 255,
            verbose_name: 'Description',
            help_text: 'The description of this static mapping.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Provides extra validation for the `ipaddr` field.
     * @param string $ipaddr The incoming value to validate.
     * @return string $ipaddr The validated value to be assigned.
     * @throws ConflictError When the $ipaddr value is in use or overlaps another object such as virtual IPs,
     * or DHCP address pools.
     */
    public function validate_ipaddr(string $ipaddr): string {
        # Do not allow this $ipaddr to overlap other model objects
        $overlapped_model = $this->get_ip_overlap($ipaddr);
        if ($overlapped_model) {
            throw new ConflictError(
                message: "Field `ipaddr` overlaps existing $overlapped_model->verbose_name with ID " .
                    "`$overlapped_model->id`.",
                response_id: 'DHCP_SERVER_STATIC_MAPPING_IPADDR_OVERLAPS_EXISTING_OBJECT',
            );
        }

        # Do not allow this $ipaddr to be outside the interface's subnet
        if ($ipaddr and !$this->parent_model->interface->get_related_model()->is_ipv4_in_cidr($ipaddr)) {
            throw new ValidationError(
                message: 'Field `ipaddr` must be within the interface\'s subnet.',
                response_id: 'DHCP_SERVER_STATIC_MAPPING_IPADDR_NOT_WITHIN_SUBNET',
            );
        }

        return $ipaddr;
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
                response_id: 'DHCP_SERVER_STATIC_MAPPING_MAX_LEASE_TIME_LESS_THAN_DEFAULT',
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
        if (
            $gateway and
            !ip_in_subnet($gateway, $interface->get_current_ipv4() . '/' . $interface->get_current_subnetv4())
        ) {
            throw new ValidationError(
                message: "DHCP server `gateway` must be within interface's subnet.",
                response_id: 'DHCP_SERVER_STATIC_MAPPING_GATEWAY_NOT_WITHIN_SUBNET',
            );
        }

        return $gateway;
    }

    /**
     * Checks if a given IP is already reserved by an existing Model object such as address pools or virtual IPs
     * @param string $ipaddr The IP address to check for existing reservation.
     * @return Model|null Returns the Model object that has this
     * $ipaddr reserved if found, returns null if this $ipaddr is not reserved.
     */
    private function get_ip_overlap(string $ipaddr): ?Model {
        # Ensure this IP address is not reserved by the primary DHCPServer address pool
        if (is_inrange_v4($ipaddr, $this->parent_model->range_from->value, $this->parent_model->range_to->value)) {
            return $this;
        }

        # Ensure IP is not reserved by any existing DHCPServerAddressPools
        $pools = DHCPServerAddressPool::query(parent_id: $this->parent_id);
        foreach ($pools->model_objects as $pool) {
            if (is_inrange_v4($ipaddr, $pool->range_from->value, $pool->range_to->value)) {
                return $pool;
            }
        }

        # Ensure IP is not reserved by any existing VirtualIP
        $vips = VirtualIP::read_all();
        foreach ($vips->model_objects as $vip) {
            if ($ipaddr === $vip->subnet->value) {
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
