<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\SystemRebootDispatcher;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model that performs a system reboot operation.
 */
class SystemReboot extends Model {
    public BooleanField $dry_run;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = false;

        # Set model fields
        $this->dry_run = new BooleanField(
            default: false,
            write_only: true,
            verbose_name: 'Dry Run',
            help_text: "Run through the call but don't actually initiate a reboot.",
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Returns an empty array. This method is the internal callable for this Model
     */
    protected function void(): array {
        return [];
    }

    /**
     * Defines the steps necessary to initiate a reboot.
     */
    public function _create(): void {
        # Only initiate the reboot if this is not a dry run
        if (!$this->dry_run->value) {
            (new SystemRebootDispatcher($this->async))->spawn_process();
        }
    }
}
