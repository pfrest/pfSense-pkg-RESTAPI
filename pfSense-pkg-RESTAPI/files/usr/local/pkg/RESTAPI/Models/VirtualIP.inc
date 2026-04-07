<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\VirtualIPApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UIDField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\UniqueFromForeignModelValidator;

/**
 * Defines a Model for interacting with the virtual IPs on this system.
 */
class VirtualIP extends Model {
    public UIDField $uniqid;
    public StringField $mode;
    public InterfaceField $interface;
    public StringField $type;
    public StringField $subnet;
    public IntegerField $subnet_bits;
    public StringField $descr;
    public BooleanField $noexpand;
    public IntegerField $vhid;
    public IntegerField $advbase;
    public IntegerField $advskew;
    public StringField $password;
    public StringField $carp_status;
    public StringField $carp_mode;
    public StringField $carp_peer;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->config_path = 'virtualip/vip';
        $this->subsystem = 'vip';
        $this->many = true;

        # Define model Fields
        $this->uniqid = new UIDField(verbose_name: 'Unique ID', help_text: 'The unique ID for this virtual IP.');
        $this->mode = new StringField(
            required: true,
            choices: ['ipalias', 'proxyarp', 'carp', 'other'],
            verbose_name: 'Mode',
            help_text: 'The virtual IP mode to use for this virtual IP.',
        );
        $this->interface = new InterfaceField(
            required: true,
            allow_carp_interface: true,
            verbose_name: 'Interface',
            help_text: 'The interface this virtual IP will apply to.',
        );
        $this->type = new StringField(
            default: 'single',
            choices: ['single', 'network'],
            verbose_name: 'Type',
            help_text: 'The virtual IP scope type. The `network` option is only applicable to the `proxyarp` and ' .
                '`other` virtual IP modes.',
        );
        $this->subnet = new StringField(
            required: true,
            unique: true,
            validators: [
                new IPAddressValidator(allow_ipv4: true, allow_ipv6: true),
                new UniqueFromForeignModelValidator(model: 'NetworkInterface', field: 'ipaddr'),
                new UniqueFromForeignModelValidator(model: 'NetworkInterface', field: 'ipaddrv6'),
            ],
            verbose_name: 'Subnet',
            help_text: 'The address for this virtual IP.',
        );
        $this->subnet_bits = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 128,
            verbose_name: 'Subnet Bits',
            help_text: 'The subnet bits for this virtual IP. For `proxyarp` and `other` virtual IPs, this value ' .
                'specifies a block of many IP address. For all other virtual IP modes, this specifies the subnet mask',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for administrative reference',
        );
        $this->noexpand = new BooleanField(
            default: false,
            indicates_true: '',
            indicates_false: null,
            conditions: ['mode' => ['proxyarp', 'other']],
            verbose_name: 'No Expand',
            help_text: 'Disable expansion of this entry into IPs on NAT lists (e.g. 192.168.1.0/24 expands to 256 entries.)',
        );
        $this->vhid = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 255,
            conditions: ['mode' => 'carp'],
            verbose_name: 'VHID',
            help_text: 'The VHID group that the machines will share.',
        );
        $this->advbase = new IntegerField(
            default: 1,
            minimum: 1,
            maximum: 254,
            conditions: ['mode' => 'carp'],
            verbose_name: 'Advertisement Base',
            help_text: 'The base frequency that this machine will advertise.',
        );
        $this->advskew = new IntegerField(
            default: 0,
            minimum: 0,
            maximum: 254,
            conditions: ['mode' => 'carp'],
            verbose_name: 'Advertisement Skew',
            help_text: 'The frequency skew that this machine will advertise.',
        );
        $this->password = new StringField(
            required: true,
            sensitive: true,
            conditions: ['mode' => 'carp'],
            verbose_name: 'Password',
            help_text: 'The VHID group password shared by all CARP members.',
        );
        $this->carp_status = new StringField(
            default: '',
            allow_empty: true,
            read_only: true,
            representation_only: true,
            conditions: ['mode' => 'carp'],
            verbose_name: 'CARP Status',
            help_text: 'The current CARP status of this virtual IP. This will display show whether this CARP node ' .
                'is the primary or backup peer.',
        );
        $this->carp_mode = new StringField(
            default: 'mcast',
            choices: ['mcast', 'ucast'],
            conditions: ['mode' => 'carp'],
            verbose_name: 'CARP Mode',
            help_text: 'The CARP mode to use for this virtual IP. Please note this field is exclusive to ' .
                'pfSense Plus and has no effect on CE.',
        );
        $this->carp_peer = new StringField(
            required: true,
            conditions: ['carp_mode' => 'ucast'],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'CARP Peer',
            help_text: 'The IP address of the CARP peer. Please note this field is exclusive to pfSense Plus and ' .
                'has no effect on CE.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation to the `type` field.
     * @param string $type The incoming `type` value to be validated.
     * @return string The validated `type` value to be set.
     * @throws ValidationError When the `type` value is not valid.
     */
    public function validate_type(string $type): string {
        # Do not allow `ipalias` and `carp` mode vips to use the `network` type.
        if (in_array($this->mode->value, ['ipalias', 'carp']) and $type === 'network') {
            throw new ValidationError(
                message: 'Virtual IPs `type` cannot be `network` when `mode` is `ipalias` or `carp`',
                response_id: 'VIRTUALIP_MODE_DOES_NOT_SUPPORT_NETWORK_TYPE',
            );
        }

        return $type;
    }

    /**
     * Adds custom validation to the `subnet_bits` field.
     * @param int $subnet_bits The incoming `subnet_bits` value to be validated.
     * @return int The validated `subnet_bits` value to be set.
     * @throws ValidationError When the `subnet_bits` value is not valid.
     */
    public function validate_subnet_bits(int $subnet_bits): int {
        # Ensure the `subnet_bits` does not exceed `32` if the current `subnet` is IPv4
        if ($this->subnet->has_label('is_ipaddrv4') and $subnet_bits > 32) {
            throw new ValidationError(
                message: 'Virtual IP `subnet_bits` cannot exceed 32 when the `subnet` is an IPv4 address',
                response_id: 'VIRTUALIP_IPV4_SUBNET_BITS_EXCEEDED',
            );
        }

        return $subnet_bits;
    }

    /**
     * Adds extra validation to the vhid field.
     * @param int $vhid The incoming `vhid` value to be validated.
     * @return int The validated `vhid` value to be set.
     * @throws ValidationError When the `vhid` value is already used by another CARP virtual IP on the same interface.
     */
    public function validate_vhid(int $vhid): int {
        # Check for an existing CARP virtual IP with the same VHID on this interface
        $vip_q = $this->query(id__except: $this->id, mode: 'carp', interface: $this->interface->value, vhid: $vhid);

        # Ensure no other CARP virtual IP on this interface is using the same VHID
        if ($vip_q->exists()) {
            $vip = $vip_q->first();
            throw new ConflictError(
                message: "Virtual IP with ID '$vip->id' is already using VHID '$vhid' on interface '{$this->interface->value}'",
                response_id: 'VIRTUALIP_VHID_ALREADY_IN_USE',
            );
        }
        return $vhid;
    }

    /**
     * Obtains the current internal CARP status of this object
     * @return string|null Returns a string that indicates the current CARP status of this virtual IP, or null
     * if this is not a virtual IP.
     */
    public function from_internal_carp_status(): string|null {
        # Do not try to obtain this internal value if this is not a CARP virtual IP
        if ($this->mode->value !== 'carp' or !$this->uniqid->value) {
            return null;
        }

        # Return `disabled` if CARP is disabled on this system
        if (!CARP::is_carp_enabled()) {
            return 'disabled';
        }

        # Get and return the active CARP status
        $carp_status = get_carp_interface_status('_vip' . $this->uniqid->value);
        return strtolower($carp_status);
    }

    /**
     * Serializes changes to virtual IPs before applying them.
     */
    private function serialize_changes(): void {
        global $g;

        # Retain existing serialized virtual IPs if present
        if (file_exists("{$g['tmp_path']}/.firewall_virtual_ip.apply")) {
            $vips_to_apply = unserialize(file_get_contents("{$g['tmp_path']}/.firewall_virtual_ip.apply"));
            $vips_to_apply[$this->id] = $this->get_config("$this->config_path/$this->id");
        } else {
            $vips_to_apply = [$this->id => $this->get_config("$this->config_path/$this->id")];
        }

        # Write the pending changes to file
        file_put_contents("{$g['tmp_path']}/.firewall_virtual_ip.apply", serialize($vips_to_apply));
    }

    /**
     * Applies pending virtual IP changes.
     */
    public function apply(): void {
        (new VirtualIPApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Serializes new virtual IPs as pending changes and saves them to configuration.     */
    public function _create(): void {
        $this->serialize_changes();
        parent::_create();
    }

    /**
     * Serializes updated virtual IPs as pending changes and saves them to configuration.
     */
    public function _update(): void {
        $this->serialize_changes();
        parent::_update();
    }

    /**
     * Brings down and removes this virtual IP from the configuration.
     */
    public function _delete(): void {
        # Call the parent _delete() method to remove the entry from config
        parent::_delete();

        # Safely remove and bring proxyarp virtual IPs
        if ($this->mode->value == 'proxyarp') {
            interface_proxyarp_configure($this->interface->value);
        }
        # Safely remove and bring down all other virtual IP types
        else {
            interface_vip_bring_down($this->to_internal());
        }
    }
}
