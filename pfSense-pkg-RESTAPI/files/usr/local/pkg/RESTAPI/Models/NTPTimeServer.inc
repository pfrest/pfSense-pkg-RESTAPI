<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with NTP time server settings.
 */
class NTPTimeServer extends Model {
    const AUTO_POOL_SUFFIX = 'pool.ntp.org';

    public StringField $timeserver;
    public StringField $type;
    public BooleanField $prefer;
    public BooleanField $noselect;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many_minimum = 1; // Require at least one timeserver at all times
        $this->many = true;
        $this->always_apply = true;
        $this->internal_callable = 'get_timeservers_from_internal';

        # Set model fields
        $this->timeserver = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Time Server',
            help_text: 'The IP or hostname of the remote NTP time server, pool or peer.',
        );
        $this->type = new StringField(
            default: 'server',
            choices: ['server', 'pool', 'peer'],
            verbose_name: 'Type',
            help_text: 'The type of this timeserver. Use `server` is `timeserver` is a standalone NTP server, use ' .
                '`pool` if `timeserver` represents an NTP pool, or `peer` if `timeserver` is an NTP peer. Note: If ' .
                'the `timeserver` value ends with the `' .
                self::AUTO_POOL_SUFFIX .
                '` suffix, this field will be forced ' .
                'to use `pool`.',
        );
        $this->prefer = new BooleanField(
            default: false,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Prefer',
            help_text: 'Enable NTP favoring the use of this server more than all others.',
        );
        $this->noselect = new BooleanField(
            default: false,
            indicates_true: true,
            indicates_false: false,
            conditions: ['!type' => 'pool'],
            verbose_name: 'No Select',
            help_text: 'Prevent NTP from using this timeserver, but continue collecting stats.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Provides extra validation to the `type` field's value.
     * @param string $type The incoming value to be validated
     * @return string The validated value to be assigned.
     */
    public function validate_type(string $type): string {
        # Force $type to be `pool` if it ends with the auto pool suffix (pool.ntp.org)
        if (str_ends_with($this->timeserver->value, self::AUTO_POOL_SUFFIX)) {
            $type = 'pool';
        }

        return $type;
    }

    /**
     * Obtains the internal values for this model. An internal callable is required here due to the timeserver values
     * being stored in various places in the config. This callable allows us to gather all the required values and
     * represent them in a more structured format.
     * @return array The internal values for this Model object obtained from the pfSense configuration.
     */
    public function get_timeservers_from_internal(): array {
        # Variables
        $internal_objects = [];
        $timeservers = explode(' ', $this->get_config(path: 'system/timeservers', default: ''));
        $ntp_config = $this->get_config(path: 'ntpd', default: []);
        $pools = explode(' ', $ntp_config['ispool']);
        $peers = explode(' ', $ntp_config['ispeer']);
        $preferred = explode(' ', $ntp_config['prefer']);
        $noselects = explode(' ', $ntp_config['noselect']);

        # Loop through each system timeserver and check its NTP configuration
        foreach ($timeservers as $id => $timeserver) {
            # Assign an internal object for this timeserver
            $internal_objects[$id] = [
                'timeserver' => $timeserver,
                'type' => 'server',
                'prefer' => false,
                'noselect' => false,
            ];

            # If this timeserver is a pool, assign its type accordingly
            if (in_array($timeserver, $pools)) {
                $internal_objects[$id]['type'] = 'pool';
            }
            # If this timeserver is a peer, assign its type accordingly
            elseif (in_array($timeserver, $peers)) {
                $internal_objects[$id]['type'] = 'peer';
            }

            # If this timeserver is preferred, assign it as such
            if (in_array($timeserver, $preferred)) {
                $internal_objects[$id]['prefer'] = true;
            }

            # If this timeserver is a no select and it is not a pool, assign it as such
            if (in_array($timeserver, $noselects) and $internal_objects[$id]['type'] !== 'pool') {
                $internal_objects[$id]['noselect'] = true;
            }
        }

        return $internal_objects;
    }

    /**
     * Obtains the next available ID for this model.
     */
    public function get_next_id(): int {
        # Get the total number of timeservers in configuration
        $timeservers = $this->get_config(path: 'system/timeservers', default: '');
        $timeservers = explode(' ', $timeservers);
        $timeservers = array_filter($timeservers); // Remove empty array items
        return count($timeservers);
    }

    /**
     * Defines the steps necessary to save this timeserver to all applicable configuration areas.
     * @param bool $no_write Only set configuration value, do not write the changes to configuration. This is used to
     * skip multiple writes from occurring when updating since we simply delete and recreate the object for updates.
     */
    public function _create(bool $no_write = false): void {
        # Set config values according to this objects current field values
        $this->id = $this->get_next_id();
        $this->set_system_timeservers();
        $this->set_ntpd_ispool();
        $this->set_ntpd_ispeer();
        $this->set_ntpd_prefer();
        $this->set_ntpd_noselect();

        # Write the changes to configuration
        if (!$no_write) {
            $this->write_config("Added $this->verbose_name via API");
        }
    }

    /**
     * Defines the steps necessary to delete this timeserver from all applicable configuration areas.
     * @param bool $no_write Only remove configuration values, do not write the changes to configuration. This is used to
     * skip multiple writes from occurring when updating since we simply delete and recreate the object for updates.
     */
    public function _delete(bool $no_write = false): void {
        # Set config values according to this objects current field values
        $this->set_system_timeservers(delete: true);
        $this->set_ntpd_ispool(delete: true);
        $this->set_ntpd_ispeer(delete: true);
        $this->set_ntpd_prefer(delete: true);
        $this->set_ntpd_noselect(delete: true);

        # Write the changes to configuration
        if (!$no_write) {
            $this->write_config("Deleted $this->verbose_name via API");
        }
    }

    /**
     * Defines the steps necessary to update this timeserver in all applicable configuration areas.
     */
    public function _update(): void {
        $this->_delete(no_write: true);
        $this->_create(no_write: true);
        $this->write_config("Updated $this->verbose_name via API");
    }

    /**
     * Applies changes to NTP server.
     */
    public function apply(): void {
        system_ntp_configure();
    }

    /**
     * Sets or removes this timeserver object in system/timeservers according to this object's current values and
     * the requested action.
     * @param bool $delete Forcibly remove this timeserver from system/timeservers
     */
    private function set_system_timeservers(bool $delete = false): void {
        # Variables
        $timeservers = explode(' ', $this->get_config(path: 'system/timeservers', default: ''));

        # Assign this timeserver to the system timeserver if it is not already present and we are not deleting
        if (!in_array($this->timeserver->value, $timeservers) and !$delete) {
            $timeservers[] = $this->timeserver->value;
            $this->set_config(path: 'system/timeservers', value: implode(' ', $timeservers), default: []);
        }

        # Remove this timeserver from the system timeservers if deletion was requested
        if ($delete) {
            $del_key = array_search($this->timeserver->value, $timeservers);
            unset($timeservers[$del_key]);
            $this->set_config(path: 'system/timeservers', value: implode(' ', $timeservers), default: []);
        }
    }

    /**
     * Sets or removes this timeserver object from ntpd/ispool according to this object's current values and the
     * requested action.
     * @param bool $delete Forcibly remove this timeserver from ntpd/ispool
     */
    private function set_ntpd_ispool(bool $delete = false): void {
        # Variables
        $pools = explode(' ', $this->get_config(path: 'ntpd/ispool', default: ''));

        # If this timeserver is a pool and is not already present in the ispool config, assign it
        if ($this->type->value === 'pool' and !in_array($this->timeserver->value, $pools)) {
            $pools[] = $this->timeserver->value;
            $this->set_config(path: 'ntpd/ispool', value: implode(' ', $pools), default: []);
        }

        # If this timeserver is not a pool and is present in the ispool config or delete was requested, remove it
        if ($delete or !$this->type->value === 'pool' and in_array($this->timeserver->value, $pools)) {
            $del_key = array_search($this->timeserver->value, $pools);
            unset($pools[$del_key]);
            $this->set_config(path: 'ntpd/ispool', value: implode(' ', $pools), default: []);
        }
    }

    /**
     * Sets or removes this timeserver object from ntpd/ispeer according to this object's current values and the
     * requested action.
     * @param bool $delete Forcibly remove this timeserver from ntpd/ispeer
     */
    private function set_ntpd_ispeer(bool $delete = false): void {
        # Variables
        $peers = explode(' ', $this->get_config(path: 'ntpd/ispeer', default: ''));

        # If this timeserver is a peer and is not already present in the ispeer config, assign it
        if ($this->type->value === 'peer' and !in_array($this->timeserver->value, $peers)) {
            $peers[] = $this->timeserver->value;
            $this->set_config(path: 'ntpd/ispeer', value: implode(' ', $peers), default: []);
        }

        # If this timeserver is not a peer and is present in the ispeer config or delete was requested, remove it
        if ($delete or !$this->type->value === 'peer' and in_array($this->timeserver->value, $peers)) {
            $del_key = array_search($this->timeserver->value, $peers);
            unset($peers[$del_key]);
            $this->set_config(path: 'ntpd/ispeer', value: implode(' ', $peers), default: []);
        }
    }

    /**
     * Sets or removes this timeserver object from ntpd/prefer according to this object's current values and the
     * requested action.
     * @param bool $delete Forcibly remove this timeserver from ntpd/prefer
     */
    private function set_ntpd_prefer(bool $delete = false): void {
        # Variables
        $prefers = explode(' ', $this->get_config(path: 'ntpd/prefer', default: ''));

        # If this timeserver is a prefer and is not already present in the prefer config, assign it
        if ($this->prefer->value and !in_array($this->timeserver->value, $prefers)) {
            $prefers[] = $this->timeserver->value;
            $this->set_config(path: 'ntpd/prefer', value: implode(' ', $prefers), default: []);
        }

        # If this timeserver is not a prefer and is present in the prefer config or delete was requested, remove it
        if ($delete or !$this->prefer->value and in_array($this->timeserver->value, $prefers)) {
            $del_key = array_search($this->timeserver->value, $prefers);
            unset($prefers[$del_key]);
            $this->set_config(path: 'ntpd/prefer', value: implode(' ', $prefers), default: []);
        }
    }

    /**
     * Sets or removes this timeserver object from ntpd/noselect according to this object's current values and the
     * requested action.
     * @param bool $delete Forcibly remove this timeserver from ntpd/noselect
     */
    private function set_ntpd_noselect(bool $delete = false): void {
        # Variables
        $noselects = explode(' ', $this->get_config(path: 'ntpd/noselect', default: ''));

        # If this timeserver is a noselect and is not already present in the noselect config, assign it
        if ($this->noselect->value and !in_array($this->timeserver->value, $noselects)) {
            $noselects[] = $this->timeserver->value;
            $this->set_config(path: 'ntpd/noselect', value: implode(' ', $noselects), default: []);
        }

        # If this timeserver is not a noselect and is present in the noselect config or delete was requested, remove it
        if ($delete or !$this->noselect->value and in_array($this->timeserver->value, $noselects)) {
            $del_key = array_search($this->timeserver->value, $noselects);
            unset($noselects[$del_key]);
            $this->set_config(path: 'ntpd/noselect', value: implode(' ', $noselects), default: []);
        }
    }
}
