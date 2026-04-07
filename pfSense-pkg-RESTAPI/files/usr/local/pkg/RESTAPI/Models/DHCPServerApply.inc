<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DHCPServerApplyDispatcher;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model for applying changes to the DHCP server configuration.
 */
class DHCPServerApply extends Model {
    public BooleanField $applied;

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
            help_text: 'Displays `true` if all DHCP server changes are applied and there are no pending changes left.' .
                'Displays `false` when there are pending DHCP server changes that have not been applied.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the current applied status.
     * @return array The internal value array to be loaded by this model as the internal callable
     */
    public function get_applied_status(): array {
        return ['applied' => !is_subsystem_dirty('dhcpd')];
    }

    /**
     * Overrides the default _create() method but does nothing. This model is only intended to apply pending changes.
     */
    protected function _create(): void {}

    /**
     * Applies changes to the DHCP server configuration.
     */
    public function apply(): void {
        (new DHCPServerApplyDispatcher(async: $this->async))->spawn_process();
    }
}
