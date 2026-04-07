<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UIDField;
use RESTAPI\Validators\FilterNameValidator;

/**
 * Defines a Model that interacts with firewall schedules.
 */
class FirewallSchedule extends Model {
    public UIDField $schedlabel;
    public StringField $name;
    public StringField $descr;
    public BooleanField $active;
    public NestedModelField $timerange;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'schedules/schedule';
        $this->many = true;
        $this->always_apply = true;
        $this->sort_by = ['name'];
        $this->sort_order = SORT_ASC;

        # Set model fields
        $this->schedlabel = new UIDField(verbose_name: 'Schedule Label', help_text: 'A unique ID for this schedule used internally by the system.');
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [new FilterNameValidator()],
            verbose_name: 'Name',
            help_text: 'The unique name to assign this schedule.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description of this schedules purpose.',
        );
        $this->active = new BooleanField(
            default: false,
            read_only: true,
            representation_only: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Active',
            help_text: 'Displays whether the schedule is currently active or not.',
        );
        $this->timerange = new NestedModelField(
            model_class: 'FirewallScheduleTimeRange',
            required: true,
            verbose_name: 'Time Ranges',
            help_text: 'The date/times this firewall schedule will be active.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the `active` status for this FirewallSchedule
     */
    protected function from_internal_active(): bool {
        # Only attempt to obtain this value if this FirewallSchedule already exists
        if (isset($this->id)) {
            return filter_get_time_based_rule_status($this->to_internal());
        }

        # Otherwise, assume this schedule is not active
        return false;
    }

    /**
     * Applies the schedule by reloading the filter.
     */
    public function apply(): void {
        # Reload the filter
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();

        # Refresh the 'active' field so it reflects any changes made.
        $this->active->value = $this->from_internal_active();
    }
}
