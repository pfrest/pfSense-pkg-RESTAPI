<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DNSResolverApplyDispatcher;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\HostnameValidator;

/**
 * Defines a Model that interacts with DNS Resolver host override aliases.
 */
class DNSResolverHostOverrideAlias extends Model {
    public StringField $host;
    public StringField $domain;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->parent_model_class = 'DNSResolverHostOverride';
        $this->config_path = 'aliases/item';
        $this->subsystem = 'unbound';
        $this->many = true;
        $this->sort_order = SORT_ASC;
        $this->sort_by = ['host'];
        $this->unique_together_fields = ['host', 'domain'];

        # Set model Fields
        $this->host = new StringField(
            required: true,
            allow_empty: true,
            maximum_length: 255,
            validators: [new HostnameValidator()],
            verbose_name: 'Host',
            help_text: 'The hostname portion of the host override alias.',
        );
        $this->domain = new StringField(
            required: true,
            maximum_length: 255,
            validators: [new HostnameValidator()],
            verbose_name: 'Domain',
            help_text: 'The hostname portion of the host override alias.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'description',
            verbose_name: 'Description',
            help_text: 'A detailed description for this host override alias.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies pending DNS Resolver changes.
     */
    public function apply() {
        (new DNSResolverApplyDispatcher(async: $this->async))->spawn_process();
    }
}
