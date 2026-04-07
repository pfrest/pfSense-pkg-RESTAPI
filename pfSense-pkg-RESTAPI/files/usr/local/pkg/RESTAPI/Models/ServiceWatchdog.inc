<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the Service Watchdog configuration on this system.
 */
class ServiceWatchdog extends Model {
    public ForeignModelField $name;
    public StringField $description;
    public BooleanField $notify;
    public BooleanField $enabled;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/servicewatchdog/item';
        $this->packages = ['pfSense-pkg-Service_Watchdog'];
        $this->package_includes = ['servicewatchdog.inc'];
        $this->always_apply = true;
        $this->many = true;

        # Set model fields
        $this->name = new ForeignModelField(
            model_name: 'Service',
            model_field: 'name',
            required: true,
            unique: true,
            verbose_name: 'Name',
            help_text: 'The name of the service to be watched.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            read_only: true,
            verbose_name: 'Description',
            help_text: 'The description for the service being watched.',
        );
        $this->notify = new BooleanField(
            default: false,
            verbose_name: 'Notify',
            help_text: 'Enable or disable notifications being sent when Service Watchdogs finds this service stopped.',
        );
        $this->enabled = new BooleanField(
            default: true,
            read_only: true,
            verbose_name: 'Enabled',
            help_text: 'Indicates if this Service Watchdog is enabled or disabled. This value is unused.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation for the `description` field. This method force updates the `description` field's value
     * to match the description of the service specified in the `name` value.
     * @param string $description The incoming value to be validated.
     * @return string The validated value to be assigned.
     */
    public function validate_description(string $description): string {
        # Force this value to always be set to the description of the Service object referenced in the `name` field.
        return $this->name->get_related_model()->description->value;
    }

    /**
     * Applies changes to the Service Watchdog configuration.
     */
    public function apply(): void {
        # Refresh the Service Watchdog cron job
        servicewatchdog_cron_job();
    }
}
