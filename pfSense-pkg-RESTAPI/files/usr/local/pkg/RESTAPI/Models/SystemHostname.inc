<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\SystemHostnameApplyDispatcher;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\HostnameValidator;

/**
 * Defines a Model that represents the system's current hostname.
 */
class SystemHostname extends Model {
    public StringField $hostname;
    public StringField $domain;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'system';
        $this->update_strategy = 'merge';
        $this->always_apply = true;

        # Set model Fields
        $this->hostname = new StringField(
            required: true,
            allow_empty: true,
            validators: [new HostnameValidator(allow_hostname: true, allow_domain: false)],
            verbose_name: 'Hostname',
            help_text: 'The hostname portion of the FQDN to assign to this system.',
        );
        $this->domain = new StringField(
            required: true,
            allow_empty: true,
            validators: [new HostnameValidator(allow_hostname: false, allow_domain: true)],
            verbose_name: 'Domain',
            help_text: 'The domain portion of the FQDN to assign to this system.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Apply this hostname to the system and restart associated services.
     */
    public function apply() {
        (new SystemHostnameApplyDispatcher(async: $this->async))->spawn_process();
    }
}
