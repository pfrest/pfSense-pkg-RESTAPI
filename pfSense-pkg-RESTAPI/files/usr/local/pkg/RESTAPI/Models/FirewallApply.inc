<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;

/**
 * Defiens a Model object that gathers details on pending firewall changes as well as tools to apply pending firewall changes.
 */
class FirewallApply extends Model {
    const FIREWALL_SUBSYSTEMS = ['aliases', 'natconf', 'filter', 'shaper'];

    public BooleanField $applied;
    public StringField $pending_subsystems;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_applied_status';
        $this->always_apply = true;

        # Set model Fields
        $this->applied = new BooleanField(
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Applied',
            help_text: 'Displays `true` when all firewall changes are applied and there are no pending changes left.' .
                'Displays `false` when there are pending firewall changes that have not been applied.',
        );
        $this->pending_subsystems = new StringField(
            allow_empty: true,
            read_only: true,
            many: true,
            delimiter: null,
            verbose_name: 'Pending Subsystems',
            help_text: 'Displays the specific firewall subsystems that have pending changes.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * The internal callable used to populate the internal values for this Model's Fields. Gathers info on pending
     * firewall changes.
     * @return array An array of internal values for Field objects in this Model
     */
    public function get_applied_status(): array {
        # Variables
        $internal_values = ['applied' => true, 'pending_subsystems' => []];

        # Loop through each firewall subsystem and check if there are pending changes.
        foreach (self::FIREWALL_SUBSYSTEMS as $subsystem) {
            # If this subsystem has pending changes, changed $applied to false and note this subsystem has pending changes
            if (is_subsystem_dirty($subsystem)) {
                $internal_values['applied'] = false;
                $internal_values['pending_subsystems'][] = $subsystem;
            }
        }

        return $internal_values;
    }

    public function _create() {
        # Override this method but don't do anything. We only want to apply the changes.
    }

    /**
     * Spawns a new dispatcher process which applies any pending firewall changes.
     */
    public function apply() {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
        $this->from_internal();
    }
}
