<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for HAProxy Frontend Addresses.
 */
class HAProxyFrontendAddress extends Model {
    public StringField $extaddr;
    public StringField $extaddr_custom;
    public PortField $extaddr_port;
    public BooleanField $extaddr_ssl;
    public StringField $exaddr_advanced;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyFrontend';
        $this->config_path = 'a_extaddr/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Frontend Address';

        # Set model fields
        $this->extaddr = new StringField(
            required: true,
            choices: ['custom', 'any_ipv4', 'any_ipv6', 'localhost_ipv4', 'localhost_ipv6'],
            help_text: 'The external address to use.',
        );
        $this->extaddr_custom = new StringField(
            required: true,
            conditions: ['extaddr' => 'custom'],
            help_text: 'The custom IPv4 or IPv6 address to use as the external address.',
        );
        $this->extaddr_port = new PortField(
            default: null,
            allow_alias: false,
            allow_range: false,
            allow_null: true,
            help_text: 'The port to bind to for this address.',
        );
        $this->extaddr_ssl = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables using SSL/TLS for this address.',
        );
        $this->exaddr_advanced = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'The advanced configuration to apply to this address.',
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
