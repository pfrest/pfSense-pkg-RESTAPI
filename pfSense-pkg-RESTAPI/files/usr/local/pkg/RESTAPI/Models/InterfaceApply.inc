<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\InterfaceApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\InterfaceField;

/**
 * Defines a Model object that gathers details on pending interface changes as well as tools to apply pending interface changes.
 */
class InterfaceApply extends Model {
    public BooleanField $applied;
    public InterfaceField $pending_interfaces;

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
            help_text: 'Displays `true` when all interfaces are applied and there are no pending changes left.' .
                'Displays `false` when there are pending interface changes that have not been applied.',
        );
        $this->pending_interfaces = new InterfaceField(
            allow_empty: true,
            read_only: true,
            many: true,
            verbose_name: 'Pending Interfaces',
            help_text: 'Displays a list of interfaces that have pending changes waiting to be applied.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * The internal callable used to populate the internal values for this Model's Fields. Gathers info on pending
     * interface changes.
     * @return array An array of internal values for Field objects in this Model
     */
    public function get_applied_status(): array {
        global $g;

        # Check if there are pending interface changes
        $applied = !is_file($g['tmp_path'] . '/.interfaces.apply');
        $pending_interfaces = [];

        # Populate pending interfaces if there are pending changes
        if (!$applied) {
            # Unserialize the file to determine which interfaces need to be applied
            $interfaces = unserialize(file_get_contents("{$g['tmp_path']}/.interfaces.apply"));
            $pending_interfaces = array_keys($interfaces);
        }

        return [
            'applied' => $applied,
            'pending_interfaces' => implode(',', $pending_interfaces),
        ];
    }

    public function _create() {
        # Override this method but don't do anything. We only want to apply the changes.
    }

    /**
     * Spawns a new dispatcher process which applies any pending interface changes.
     */
    public function apply() {
        (new InterfaceApplyDispatcher(async: $this->async))->spawn_process();
    }
}
