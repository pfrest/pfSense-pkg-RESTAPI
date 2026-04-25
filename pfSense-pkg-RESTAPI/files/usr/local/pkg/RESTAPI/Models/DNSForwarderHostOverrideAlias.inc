<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DNSForwarderApplyDispatcher;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that interacts with DNS Forwarder host override aliases.
 */
class DNSForwarderHostOverrideAlias extends Model {
    public StringField $host;
    public StringField $domain;
    public StringField $description;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'DNSForwarderHostOverride';
        $this->config_path = 'aliases/item';
        $this->subsystem = 'hosts';
        $this->many = true;

        # Set model fields
        $this->host = new StringField(
            required: true,
            verbose_name: 'Host',
            help_text: 'The hostname of this override alias.',
        );
        $this->domain = new StringField(
            required: true,
            verbose_name: 'Domain',
            help_text: 'The domain of this override alias.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The description of this override alias.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Apply changes to the DNS Forwarder
     */
    public function apply(): void {
        # Apply changes to the DNS Forwarder
        (new DNSForwarderApplyDispatcher(async: $this->async))->spawn_process();
    }
}
