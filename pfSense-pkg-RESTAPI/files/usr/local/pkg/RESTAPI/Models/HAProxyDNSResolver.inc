<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for HAProxy DNS Resolvers.
 */
class HAProxyDNSResolver extends Model {
    public StringField $name;
    public StringField $server;
    public PortField $port;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxySettings';
        $this->config_path = 'dns_resolvers/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy DNS Resolver';

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            allow_empty: true,
            help_text: 'The descriptive name for this DNS server.',
        );
        $this->server = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            help_text: 'The IP or hostname of the DNS server.',
        );
        $this->port = new PortField(
            default: '53',
            allow_alias: false,
            allow_range: false,
            help_text: 'The port used by this DNS server.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Ensures haproxy is marked as dirty before applying.
     */
    public function pre_apply(): void {
        touch('/var/run/haproxy.conf.dirty');
    }

    /**
     * Applies changes to the HAProxy configuration.
     */
    public function apply(): void {
        (new HAProxyApplyDispatcher(async: $this->async))->spawn_process();
    }
}
