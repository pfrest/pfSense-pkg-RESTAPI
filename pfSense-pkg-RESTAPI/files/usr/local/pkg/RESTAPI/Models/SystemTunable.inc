<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model that represents a sysctl tunable on this system.
 */
class SystemTunable extends Model {
    public StringField $tunable;
    public StringField $value;
    public StringField $descr;
    private array $sysctl_defaults;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'sysctl/item';
        $this->many = true;
        $this->subsystem = 'sysctl';

        # Set model Fields
        $this->tunable = new StringField(required: true, unique: true, verbose_name: 'Tunable', help_text: 'The name of the tunable to set.');
        $this->value = new StringField(
            required: true,
            validators: [new RegexValidator(pattern: '/[^a-zA-Z0-9.\-_%\/]/', invert: true)],
            verbose_name: 'Value',
            help_text: 'The value to assign this tunable.',
        );
        $this->descr = new StringField(default: '', allow_empty: true, verbose_name: 'Description', help_text: 'A description for this tunable.');

        # Keep track of the original sysctl defaults
        global $sysctls;
        $this->sysctl_defaults = $sysctls;

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Apply all configured sysctl tunables.
     */
    public function apply() {
        # Setup all sysctl values
        system_setup_sysctl();

        # Re-assign the default sysctls so they are accurate for the next `apply()` call.
        # This addresses an issue where a PHP script that both creates/updates and deletes a SystemTunable object
        # would not set the correct value after deletion.
        global $sysctls;
        $sysctls = $this->sysctl_defaults;
    }
}
