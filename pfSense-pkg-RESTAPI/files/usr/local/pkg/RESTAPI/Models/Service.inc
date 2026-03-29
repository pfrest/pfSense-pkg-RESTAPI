<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the status of services on this system.
 */
class Service extends Model {
    public StringField $name;
    public StringField $action;
    public StringField $description;
    public BooleanField $enabled;
    public BooleanField $status;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set Model attributes
        $this->internal_callable = 'get_services';
        $this->auto_create_id = false; // We don't create services with create(), we just control existing ones
        $this->many = true;

        # Set Model Fields
        $this->action = new StringField(
            required: true,
            choices: ['start', 'stop', 'restart'],
            write_only: true,
            help_text: 'The action to perform against this service.',
        );
        $this->name = new StringField(required: true, unique: true, help_text: 'The internal name of the service.');
        $this->description = new StringField(read_only: true, help_text: 'The full descriptive name of the service.');
        $this->enabled = new BooleanField(
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            help_text: 'Indicates if the service is enabled.',
        );
        $this->status = new BooleanField(
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            help_text: 'Indicates if the service is actively running.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains all available services and their operating status. This method is the internal callable that populates
     * the internal field values for this Model.
     * @return array An array of available pfSense services and their current statuses.
     */
    public static function get_services(): array {
        $services = get_services();

        # For each service, determine if it's really enabled and running
        foreach ($services as $key => $service) {
            $services[$key]['enabled'] = is_service_enabled($service['name']);
            $services[$key]['status'] = get_service_status($service);
        }

        return $services;
    }

    /**
     * Starts or stops a specified service.
     */
    public function _create(): void {
        # Obtain the extras for this service. Provides additional info required for some services to be controlled.
        $extras = $this->_format_extra_data($this->get_services()[$this->id]);

        # Trigger the requested service action
        switch ($this->action->value) {
            case 'start':
                service_control_start(name: $this->name->value, extras: $extras);
                break;
            case 'stop':
                service_control_stop(name: $this->name->value, extras: $extras);
                break;
            case 'restart':
                service_control_restart(name: $this->name->value, extras: $extras);
                break;
        }

        # Recheck the service status after the action has been performed to ensure the correct status is returned
        $this->from_internal();
    }

    /**
     * Formats the 'extra' service data required for some services to be restarted to the weird format pfSense
     * sometimes expects. In the webConfigurator, this process is obfuscated by AJAX calls and JavaScript so there
     * is no direct function to format this data. This function is a workaround to provide the correct data format.
     *
     * @param array $service The service data to format.
     * @return array The formatted service data suitable for the service control functions 'extras' parameter.
     */
    public function _format_extra_data(array $service): array {
        # Format OpenVPN service extra data for service control
        if ($service['name'] === 'openvpn') {
            # 'mode' changes to 'vpnmode'
            $service['vpnmode'] = $service['mode'];
            unset($service['mode']);

            # 'vpnid' changes to just 'id'
            $service['id'] = $service['vpnid'];
        }

        return $service;
    }
}
