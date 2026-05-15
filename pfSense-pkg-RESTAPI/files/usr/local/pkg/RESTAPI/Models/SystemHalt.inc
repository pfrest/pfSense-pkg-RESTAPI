<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\SystemHaltDispatcher;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model that performs a system halt operation.
 */
class SystemHalt extends Model {
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
     * Defines the steps necessary to initiate a shutdown.
     */
    public function _create(): void {
        (new SystemHaltDispatcher($this->async))->spawn_process();
    }
}
