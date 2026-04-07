<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DNSResolverApplyDispatcher;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\HostnameValidator;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that interacts with DNS Resolver host overrides.
 */
class DNSResolverHostOverride extends Model {
    public StringField $host;
    public StringField $domain;
    public StringField $ip;
    public StringField $descr;
    public NestedModelField $aliases;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'unbound/hosts';
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
            help_text: 'The hostname portion of the host override.',
        );
        $this->domain = new StringField(
            required: true,
            maximum_length: 255,
            validators: [new HostnameValidator()],
            verbose_name: 'Domain',
            help_text: 'The hostname portion of the host override.',
        );
        $this->ip = new StringField(
            required: true,
            many: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'IP',
            help_text: 'The IP addresses this host override will resolve.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A detailed description for this host override.',
        );
        $this->aliases = new NestedModelField(
            model_class: 'DNSResolverHostOverrideAlias',
            default: [],
            allow_empty: true,
            verbose_name: 'Aliases',
            help_text: 'Additional alias hostnames that should resolve the same IP(s).',
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
