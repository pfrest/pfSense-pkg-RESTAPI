<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;

/**
 * Defines a Model for interacting with the advanced firewall settings.
 */
class FirewallAdvancedSettings extends Model {
    public IntegerField $aliasesresolveinterval;
    public BooleanField $checkaliasesurlcert;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'system';
        $this->update_strategy = 'merge';

        # Set model fields
        $this->aliasesresolveinterval = new IntegerField(
            default: PHP_INT_MAX,
            minimum: 0,
            verbose_name: 'Alias Resolve Interval',
            help_text: 'The interval (in seconds) at which to resolve hostnames in aliases.',
        );
        $this->checkaliasesurlcert = new BooleanField(
            default: true,
            verbose_name: 'Check Alias URL Certs',
            help_text: 'Check the certificate of URLs used in aliases.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    public function apply(): void {
        # Restart the filterdns service if the aliasesresolveinterval field has changed
        if ($this->initial_object->aliasesresolveinterval->value !== $this->aliasesresolveinterval->value) {
            $this->restart_filterdns();
        }

        # Apply changes to the firewall
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Restarts the filterdns service.
     */
    private function restart_filterdns(): void {
        global $g;

        # If the filterdns process ID is present, kill it.
        if (isvalidpid("{$g['varrun_path']}/filterdns.pid")) {
            killbypid("{$g['varrun_path']}/filterdns.pid");
        }
    }
}
