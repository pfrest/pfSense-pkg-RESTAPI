<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\SystemRebootDispatcher;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model that performs a system reboot operation.
 */
class SystemReboot extends Model {
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = false;

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
        (new SystemRebootDispatcher($this->async))->spawn_process();
    }
}
