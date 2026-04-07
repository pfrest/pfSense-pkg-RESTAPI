<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model that represents the console configuration on this system.
 */
class SystemConsole extends Model {
    public BooleanField $passwd_protect_console;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'system';
        $this->update_strategy = 'merge';
        $this->many = false;
        $this->always_apply = true;

        # Set model fields
        $this->passwd_protect_console = new BooleanField(
            default: false,
            internal_name: 'disableconsolemenu',
            verbose_name: 'Passwd Protect Console',
            help_text: 'Enables or disables password protecting the console.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the console configuration.
     */
    public function apply() {
        console_configure();
    }
}
