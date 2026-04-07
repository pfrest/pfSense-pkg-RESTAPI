<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\IntegerField;

/**
 * Defines a Model for interacting with the firewall states size.
 */
class FirewallStatesSize extends Model {
    public IntegerField $maximumstates;
    public IntegerField $defaultmaximumstates;
    public IntegerField $currentstates;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'system';
        $this->update_strategy = 'merge';
        $this->always_apply = true;

        # Set model Fields
        $this->maximumstates = new IntegerField(
            default: pfsense_default_state_size(),
            minimum: 10,
            verbose_name: 'Maximum States',
            help_text: 'The maximum number of firewall state entries allowed by this firewall.',
        );
        $this->defaultmaximumstates = new IntegerField(
            read_only: true,
            verbose_name: 'Default Max States',
            help_text: 'The default number of firewall state entries allowed by this firewall.',
        );
        $this->currentstates = new IntegerField(
            read_only: true,
            verbose_name: 'Current States',
            help_text: 'The number of firewall state entries currently registered in the states table.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the default maximum states value from internal pfSense functions.
     */
    protected function from_internal_defaultmaximumstates(): int {
        return (int) pfsense_default_state_size();
    }

    /**
     * Obtains the current number of states value from internal pfSense functions.
     */
    protected function from_internal_currentstates(): int {
        return (int) explode('/', get_pfstate())[0];
    }

    /**
     * Applies the firewall states size changes.
     */
    public function apply() {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
