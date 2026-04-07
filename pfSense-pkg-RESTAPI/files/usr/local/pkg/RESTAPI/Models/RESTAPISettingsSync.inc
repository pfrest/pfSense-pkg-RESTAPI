<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Endpoints\SystemRESTAPISettingsSyncEndpoint;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use function RESTAPI\Core\Tools\cprint;

/**
 * Defines a Model that interacts with the REST API settings sync feature. This Model is not intended for public use.
 */
class RESTAPISettingsSync extends Model {
    public StringField $sync_data;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'void';
        $this->many = false;
        $this->verbose_name = 'REST API Settings Sync';

        # Set model fields
        $this->sync_data = new StringField(
            required: true,
            representation_only: true,
            maximum_length: 65535,
            verbose_name: 'Sync Data',
            help_text: 'The serialized REST API settings data to be synced.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Returns an empty array as the internall callable
     * @returns array An array containing nothing
     */
    protected function void(): array {
        return [];
    }

    /**
     * Adds extra validation to the `sync_data` field.
     * @param string $sync_data The incoming value to be validated.
     * @returns string The validated value to be assigned
     * @throws ValidationError When `sync_data` cannot be unserialized into an array of REST API settings
     */
    public function validate_sync_data(string $sync_data): string {
        # Attempt to unserialize the sync data
        $unserialized_sync_data = unserialize($this->sync_data->value);

        # Throw a validation error if $sync_data could not be unserialized
        if (!$unserialized_sync_data) {
            throw new ValidationError(
                message: 'Field `sync_data` value could not be unserialized.',
                response_id: 'RESTAPI_SETTINGS_SYNC_SYNC_DATA_COULD_NOT_BE_UNSERIALIZED',
            );
        }

        return $sync_data;
    }

    /**
     * Writes the synced changes to configuration.
     */
    public function _create(): void {
        # Unserialize the sync data
        $unserialized_sync_data = unserialize($this->sync_data->value);

        # Disable HA sync on this system since it is being managed by a remote peer.
        $unserialized_sync_data['ha_sync'] = 'disabled';
        $unserialized_sync_data['ha_sync_hosts'] = '';
        $unserialized_sync_data['ha_sync_username'] = '';
        $unserialized_sync_data['ha_sync_password'] = '';

        # Write the synced REST API settings over this hosts REST API settings
        $pkg_id = RESTAPISettings::get_pkg_id();
        $this->set_config("installedpackages/package/$pkg_id/conf", $unserialized_sync_data);
        $this->write_config(change_note: 'Synced REST API settings from ' . $this->client->ip_address);
    }

    /**
     * Syncs this host's REST API settings to HA peers if configured.
     * @param bool $print_status Set to true to print the progress of the sync to the console. Set to false run the
     * sync quietly.
     */
    public static function sync(bool $print_status = false): void {
        # Variables
        $settings = new RESTAPISettings();
        $endpoint_url = (new SystemRESTAPISettingsSyncEndpoint())->url;
        # TODO: Replace these with a WebGUI Model call once developed.
        $wc_protocol = self::get_config(path: 'system/webgui/protocol', default: 'https');
        $wc_port = self::get_config(path: 'system/webgui/port', default: '443');
        cprint(message: 'Starting REST API settings sync... ', condition: $print_status);

        # Don't perform sync if HA sync is disabled
        if (!$settings->ha_sync->value) {
            cprint(message: 'not configured.' . PHP_EOL, condition: $print_status);
            return;
        }

        cprint(PHP_EOL, $print_status);

        # Sync our REST API settings to each configured HA sync host
        foreach ($settings->ha_sync_hosts->value as $ha_sync_host) {
            # Make an API request to the sync endpoint on the HA peer
            cprint("Syncing REST API settings to $ha_sync_host... ", $print_status);
            $response = \RESTAPI\Core\Tools\http_request(
                url: "$wc_protocol://$ha_sync_host:$wc_port$endpoint_url",
                method: 'POST',
                data: ['sync_data' => serialize($settings->get_pkg_config())],
                username: $settings->ha_sync_username->value,
                password: $settings->ha_sync_password->value,
                validate_certs: false,
            );

            # Decode the response from JSON
            $response_json = json_decode($response, associative: true);

            # Log an error if the HTTP failed
            if (!$response) {
                cprint(message: 'failed.' . PHP_EOL, condition: $print_status);
                self::log(
                    level: LOG_ERR,
                    message: "Failed to sync REST API settings to $ha_sync_host: no response received.",
                );
            } elseif (!$response_json) {
                cprint(message: 'failed.' . PHP_EOL, condition: $print_status);
                self::log(
                    level: LOG_ERR,
                    message: "Failed to sync REST API settings to $ha_sync_host: received unexpected response.",
                );
            } elseif ($response_json['code'] !== 200) {
                cprint(message: 'failed.' . PHP_EOL, condition: $print_status);
                self::log(
                    level: LOG_ERR,
                    message: "Failed to sync REST API settings to $ha_sync_host: {$response_json['message']}",
                );
            } else {
                cprint(message: 'done.' . PHP_EOL, condition: $print_status);
            }
        }
    }
}
