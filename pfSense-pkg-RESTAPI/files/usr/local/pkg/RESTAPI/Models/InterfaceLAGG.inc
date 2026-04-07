<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ValidationError;

/**
 * A Model class for interacting with LAGG interfaces.
 */
class InterfaceLAGG extends Model {
    public StringField $laggif;
    public StringField $descr;
    public StringField $members;
    public StringField $proto;
    public StringField $lacptimeout;
    public StringField $lagghash;
    public StringField $failovermaster;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'laggs/lagg';
        $this->many = true;
        $this->always_apply = true;

        # Set model fields
        $this->laggif = new StringField(
            read_only: true,
            maximum_length: 32,
            verbose_name: 'LAGG Interface',
            help_text: 'The real name of the LAGG interface.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description to help document the purpose of this LAGG interface.',
        );
        $this->members = new StringField(
            required: true,
            many: true,
            many_minimum: 1,
            verbose_name: 'Members',
            help_text: 'A list of member interfaces to include in the LAGG.',
        );
        $this->proto = new StringField(
            required: true,
            choices: ['lacp', 'failover', 'loadbalance', 'roundrobin', 'none'],
            verbose_name: 'Protocol',
            help_text: 'The LAGG protocol to use.',
        );
        $this->lacptimeout = new StringField(
            default: 'slow',
            choices: ['slow', 'fast'],
            conditions: ['proto' => 'lacp'],
            verbose_name: 'LACP Timeout',
            help_text: 'The LACP timeout mode to use.',
        );
        $this->lagghash = new StringField(
            default: 'l2,l3,l4',
            choices: ['l2', 'l3', 'l4', 'l2,l3', 'l2,l4', 'l3,l4', 'l2,l3,l4'],
            conditions: ['proto' => ['lacp', 'loadbalance']],
            verbose_name: 'LAGG Hash',
            help_text: 'The LAGG hash algorithm to use.',
        );
        $this->failovermaster = new StringField(
            default: 'auto',
            conditions: ['proto' => 'failover'],
            verbose_name: 'Failover Master',
            help_text: 'The failover master interface to use.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Add extra validation to the `members` field.
     * @param string $value The value to validate.
     * @return string The validated value to assign.
     * @throws ValidationError When the requested member interface does not exist.
     */
    public function validate_members(string $value): string {
        # Throw a validation error if the request member does not exist
        if (!does_interface_exist($value)) {
            throw new ValidationError(
                message: "Member interface `$value` does not exist or is invalid.",
                response_id: 'INTERFACE_LAGG_MEMBER_DOES_NOT_EXIST',
            );
        }

        return $value;
    }

    /**
     * Adds extra validation to the `failovermaster` field.
     * @param string $value The value to validate.
     * @return string The validated value to assign.
     * @throws ValidationError When the requested failover master interface is not assigned as a member.
     */
    public function validate_failovermaster(string $value): string {
        # Always allow the `auto` value
        if ($value === 'auto') {
            return $value;
        }

        # Throw a validation error if the requested failover master is not a member
        if (!in_array($value, $this->members->value)) {
            throw new ValidationError(
                message: "Failover master interface `$value` is not a member of the LAGG.",
                response_id: 'INTERFACE_LAGG_MASTER_NOT_MEMBER',
            );
        }

        return $value;
    }

    /**
     * Extends the default _create method to capture the LAGG interface name of the newly created LAGG.
     */
    public function _create(): void {
        $this->laggif->value = interface_lagg_configure($this->to_internal());
        parent::_create();
    }

    /**
     * Extends the default _update method to reconfigure the updated LAGG.
     */
    public function _update(): void {
        interface_lagg_configure($this->to_internal());
        parent::_update();
    }

    /**
     * Extends the default _delete method to prevent deletion of the LAGG interface if it is in use.
     */
    public function _delete(): void {
        # Query for an interface using this lagg
        $if_q = NetworkInterface::query(if: $this->laggif->value);
        if ($if_q->exists()) {
            throw new ConflictError(
                message: "Cannot delete LAGG interface `{$this->laggif->value}` because it is in use by interface " .
                    "with ID `{$if_q->first()->id}`.",
                response_id: 'INTERFACE_LAGG_CANNOT_BE_DELETED_WHILE_IN_USE',
            );
        }

        parent::_delete();
    }

    /**
     * Reconfigures VLANs and interfaces using this LAGG after successful creation or update.
     */
    public function apply(): void {
        # Check for interfaces using this LAGG and reconfigure them
        $if_q = NetworkInterface::query(if: $this->laggif->value);
        foreach ($if_q->model_objects as $if) {
            interface_configure($if->id);
        }

        # Check for VLANs using this LAGG and reconfigure them
        $vlan_q = InterfaceVLAN::query(if: $this->laggif->value);
        foreach ($vlan_q->model_objects as $vlan) {
            interface_vlan_configure($vlan->to_internal());
        }
    }

    /**
     * Applies the deletion of this LAGG interface.
     */
    public function apply_delete(): void {
        pfSense_interface_destroy($this->laggif->value);
    }
}
