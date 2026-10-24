<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\InterfaceAddressField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for HAProxy Frontend Addresses.
 */
class HAProxyFrontendAddress extends Model {
    public InterfaceAddressField $extaddr;
    public StringField $extaddr_custom;
    public PortField $extaddr_port;
    public BooleanField $extaddr_ssl;
    public StringField $exaddr_advanced;

    /**
     * Defines the HAProxy frontend address fields and configuration location.
     * @param mixed $id The address ID.
     * @param mixed $parent_id The frontend ID.
     * @param mixed $data The address data.
     * @param mixed ...$options Additional Model options.
     */
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyFrontend';
        $this->config_path = 'a_extaddr/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Frontend Address';

        # Set model fields
        $this->extaddr = new InterfaceAddressField(
            required: true,
            allow_custom: ['custom', 'any_ipv4', 'any_ipv6', 'localhost_ipv4', 'localhost_ipv6'],
            verbose_name: 'External Address',
            help_text: 'The external address to use. Interface references must match the format set as the REST APIs ' .
                'interface representation format in the REST API settings, and must end with `_ipv4` or `_ipv6` to ' .
                'select the address family.',
        );
        $this->extaddr_custom = new StringField(
            required: true,
            verbose_name: 'Custom External Address',
            conditions: ['extaddr' => 'custom'],
            help_text: 'The custom IPv4 or IPv6 address to use as the external address.',
        );
        $this->extaddr_port = new PortField(
            default: null,
            allow_alias: false,
            allow_range: false,
            allow_null: true,
            verbose_name: 'External Address Port',
            help_text: 'The port to bind to for this address.',
        );
        $this->extaddr_ssl = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'External Address SSL',
            help_text: 'Enables or disables using SSL/TLS for this address.',
        );
        $this->exaddr_advanced = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Advanced External Address',
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
