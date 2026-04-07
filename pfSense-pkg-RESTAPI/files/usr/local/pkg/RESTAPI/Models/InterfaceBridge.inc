<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model for interacting with Interface Bridges.
 */
class InterfaceBridge extends Model {
    public InterfaceField $members;
    public StringField $descr;
    public StringField $bridgeif;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'bridges/bridged';
        $this->many = true;
        $this->always_apply = true;

        # Set model Fields
        $this->members = new InterfaceField(
            required: true,
            unique: true,
            many: true,
            many_minimum: 1,
            verbose_name: 'Members',
            help_text: 'The member interfaces to include in this bridge.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this interface bridge.',
        );
        $this->bridgeif = new StringField(
            default: $this->get_next_bridgeif(),
            allow_empty: true,
            read_only: true,
            verbose_name: 'Bridge Interface',
            help_text: 'The real interface name for this bridge interface.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `members` field.
     * @param string $members The incoming value to validate.
     * @returns string The validated value to be set.
     */
    public function validate_members(string $members): string {
        # Variables
        $member_internal = NetworkInterface::query([$this->members->represented_as => $members])->first()->id;
        $is_wireless = $this->get_config("interfaces/$member_internal/wireless");
        $is_wireless_hostap = $this->get_config("interfaces/$member_internal/wireless/mode") == 'hostap';

        # Throw an error if this is a wireless interface but it is not in `hostap` mode
        if ($is_wireless and !$is_wireless_hostap) {
            throw new ValidationError(
                message: "Bridge `members` cannot be wireless interface unless it's in `hostap` mode.",
                response_id: 'INTERFACE_BRIDGE_NO_NON_HOSTAP_WIRELESS_MEMBERS',
            );
        }

        # Throw an error if this bridge member is assigned to a captive port
        foreach ($this->get_config('captiveportal', []) as $cp) {
            if (in_array($member_internal, explode(',', $cp['interface']))) {
                throw new ValidationError(
                    message: 'Bridge `members` cannot be assigned to a captive portal',
                    response_id: 'INTERFACE_BRIDGE_MEMBER_IS_ASSIGNED_TO_CAPTIVE_PORTAL',
                );
            }
        }

        return $members;
    }

    /**
     * Applies this interface bridge on the backend.
     */
    public function apply() {
        interface_bridge_configure($this->to_internal());
    }

    /**
     * Applies the deletion of this interface bridge.
     */
    public function apply_delete() {
        pfSense_interface_destroy($this->bridgeif->value);
    }

    /**
     * Extends the default Model `_delete()` method to add custom validation for deletions.
     */
    public function _delete() {
        # Do not allow this object to be delete if it is assigned to an existing interface
        $if_query = NetworkInterface::query(if: $this->bridgeif->value);
        if ($if_query->exists()) {
            throw new ValidationError(
                message: 'Interface bridge cannot be deleted because it is in use by interface with ID ' .
                    "ID `{$if_query->first()->id}",
                response_id: 'INTERFACE_BRIDGE_IN_USE',
            );
        }

        # Proceed with the deletion as usual
        parent::_delete();
    }

    /**
     * Obtains the next available bridgeif value
     * @returns string The next available bridgeif value.
     */
    public function get_next_bridgeif(): string {
        # Local variables
        $counter = 0;
        $in_use = true;

        # Loop continuously until unique bridge if is found
        while ($in_use) {
            # Loop variables
            $next_bridge = 'bridge' . $counter;
            $in_use = false;

            # Loop through all existing bridges and see if this bridgeif is in use
            foreach ($this->get_config('bridges/bridged', []) as $bridge) {
                # Check if this value matches our potential next bridge
                if ($bridge['bridgeif'] === $next_bridge) {
                    $in_use = true;
                }
            }

            # Increase the counter
            $counter++;
        }

        return $next_bridge;
    }
}
