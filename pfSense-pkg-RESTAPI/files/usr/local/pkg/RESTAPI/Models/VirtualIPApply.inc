<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\VirtualIPApplyDispatcher;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model that provides details on pending virtual IP changes and allows virtual IP changes
 * to be applied.
 */
class VirtualIPApply extends Model {
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
            help_text: 'Displays `true` when all virtual IP changes are applied and there are no pending changes left.' .
                'Displays `false` when there are pending virtual IP changes that have not been applied.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the current applied status.
     * @return array The internal value array to be loaded by this model as the internal callable
     */
    public function get_applied_status(): array {
        return ['applied' => !is_file('/tmp/.firewall_virtual_ip.apply')];
    }

    /**
     * Override the _create method so it does nothing. We only want to apply changes when this is called.
     */
    public function _create(): void {}

    /**
     * Applies pending changes.
     */
    public function apply(): void {
        (new VirtualIPApplyDispatcher(async: $this->async))->spawn_process();
    }
}
