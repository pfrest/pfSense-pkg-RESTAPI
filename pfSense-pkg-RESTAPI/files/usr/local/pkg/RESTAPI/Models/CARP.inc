<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interacting with the system's CARP settings.
 */
class CARP extends Model {
    public BooleanField $enable;
    public BooleanField $maintenance_mode;
    public StringField $interfaces;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_carp_internal';
        $this->always_apply = true;

        # Set model Fields
        $this->enable = new BooleanField(
            required: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Enable',
            help_text: 'Enables or disables CARP on this system.',
        );
        $this->maintenance_mode = new BooleanField(
            required: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Maintenance Mode',
            help_text: 'Enables or disables CARP maintenance mode on this system.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Defines how to update this object.
     */
    public function _update() {
        # Set the maintenance mode value in config if we are enabling maintenance mode
        if ($this->maintenance_mode->value === true) {
            $this->set_config('virtualip_carp_maintenancemode', '');
        }
        # Remove the maintenance mode value if we are disabling maintenance mode
        else {
            $this->del_config('virtualip_carp_maintenancemode');
        }

        # Write the maintenance mode changes to config
        $this->write_config('Updated CARP setttings via API');
    }

    /**
     * Applies changes to CARP on this system.
     */
    public function apply() {
        interfaces_carp_set_maintenancemode($this->maintenance_mode->value);
        $this->setup_carp();
    }

    /**
     * Obtains the internal representation for this object's Fields. This method is the internally callable for this
     * Model class.
     * @return array An array containing the internal values for this Model's Fields.
     */
    public function get_carp_internal(): array {
        return [
            'enable' => $this->is_carp_enabled(),
            'maintenance_mode' => $this->is_config_enabled('', 'virtualip_carp_maintenancemode'),
        ];
    }

    /**
     * Checks if CARP is enabled on this system.
     * @return bool Returns true if CARP is enabled on this system or false if CARP is not enabled on this system.
     */
    public static function is_carp_enabled(): bool {
        return (bool) get_single_sysctl('net.inet.carp.allow');
    }

    /**
     * Enables or disables CARP on the current system.
     */
    public function setup_carp() {
        # Don't change anything if the CARP enabled value hasn't changed
        if ($this->is_carp_enabled() === $this->enable->value) {
            return;
        }

        # Loop through each virtual IP and configure it according to the new CARP settings
        foreach (VirtualIP::read_all()->model_objects as $vip) {
            # Configure CARP virtual IPs if we are enabling CARP
            if ($this->enable->value and $vip->mode->value === 'carp') {
                interface_carp_configure($vip->to_internal());
                continue;
            }
            # Configure IP Alias virtual IPs with CARP interfaces if we are enabling CARP
            if (
                $this->enable->value and
                $vip->mode->value === 'ipalias' and
                str_starts_with($vip->interface->value, '_vip')
            ) {
                interface_ipalias_configure($vip->to_internal());
                continue;
            }
            # Bring down CARP virtual IPs if we are disabling CARP
            if (!$this->enable->value and $vip->mode->value === 'carp') {
                interface_vip_bring_down($vip->to_internal());
                continue;
            }
            # Bring down IP alias virtual IPs with CARP interfaces if we are disabling CARP
            if (
                !$this->enable->value and
                $vip->mode->value === 'ipalias' and
                str_starts_with($vip->interface->value, '_vip')
            ) {
                interface_vip_bring_down($vip->to_internal());
            }
        }

        # Enable CARP system-wide if we are enabling CARP
        if ($this->enable->value) {
            interfaces_sync_setup();
            set_single_sysctl('net.inet.carp.allow', '1');
        }
        # Disable CARP system-wide if we are disabling CARP
        else {
            set_single_sysctl('net.inet.carp.allow', '0');
        }
    }
}
