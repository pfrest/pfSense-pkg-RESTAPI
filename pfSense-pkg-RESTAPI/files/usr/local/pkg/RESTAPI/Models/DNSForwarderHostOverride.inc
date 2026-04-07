<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DNSForwarderApplyDispatcher;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that interacts with DNS Forwarder host overrides.
 */
class DNSForwarderHostOverride extends Model {
    public StringField $host;
    public StringField $domain;
    public StringField $ip;
    public StringField $descr;
    public NestedModelField $aliases;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'dnsmasq/hosts';
        $this->subsystem = 'hosts';
        $this->unique_together_fields = ['host', 'domain'];
        $this->many = true;

        # Set model fields
        $this->host = new StringField(required: true, verbose_name: 'Host', help_text: 'The hostname of this override.');
        $this->domain = new StringField(required: true, verbose_name: 'Domain', help_text: 'The domain of this override.');
        $this->ip = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'IP',
            help_text: 'The IP address of this override.',
        );
        $this->descr = new StringField(default: '', allow_empty: true, verbose_name: 'Description', help_text: 'The description for this override.');
        $this->aliases = new NestedModelField(
            model_class: 'DNSForwarderHostOverrideAlias',
            default: [],
            allow_empty: true,
            verbose_name: 'Aliases',
            help_text: 'The aliases for this override.',
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
