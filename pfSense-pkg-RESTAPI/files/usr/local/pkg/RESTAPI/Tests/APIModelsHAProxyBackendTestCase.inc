<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\HAProxyBackend;
use RESTAPI\Models\HAProxyBackendACL;
use RESTAPI\Models\HAProxyBackendAction;

class APIModelsHAProxyBackendTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-haproxy'];

    /**
     * Ensure that the HAProxyBackend model can be created, updated, and deleted.
     */
    public function test_crud(): void {
        # Create a new HAProxyBackend model
        $backend = new HAProxyBackend(
            name: 'example_backend',
            balance: '',
            acls: [
                [
                    'name' => 'example_acl',
                    'expression' => 'host_starts_with',
                    'value' => 'example',
                    'casesensitive' => false,
                    'not' => false,
                ],
            ],
            actions: [
                [
                    'action' => 'http-response_lua',
                    'acl' => 'example_acl',
                    'lua_function' => 'example function',
                ],
            ],
            check_type: 'HTTP',
            log_health_checks: false,
            httpcheck_method: 'OPTIONS',
            monitor_uri: '',
            monitor_httpversion: '',
            agent_checks: false,
            agent_port: '',
            persist_cookie_enabled: false,
            persist_cookie_name: '',
            persist_cookie_mode: 'passive',
            haproxy_cookie_domains: [],
            haproxy_cookie_dynamic_cookie_key: '',
            persist_sticky_type: 'none',
            persist_stick_expire: '',
            persist_stick_tablesize: '',
            persist_stick_cookiename: '',
            email_level: '',
            email_to: '',
            stats_enabled: true,
            stats_uri: '/',
            stats_scope: ['.'],
            stats_realm: '',
            stats_username: '',
            stats_admin: '',
            stats_node: 'NODE1',
            stats_desc: '',
            stats_refresh: 10,
            cookie_attribute_secure: false,
            advanced: '',
            advanced_backend: '',
            transparent_clientip: true,
            transparent_interface: 'lan',
        );
        $backend->create();

        # Ensure the model was created
        $read_backend = HAProxyBackend::query(id: $backend->id);
        $this->assert_is_true($read_backend->exists());
        $read_backend = $read_backend->first();
        $this->assert_equals($read_backend->name->value, 'example_backend');
        $this->assert_equals($read_backend->balance->value, '');
        $this->assert_equals($read_backend->check_type->value, 'HTTP');
        $this->assert_equals($read_backend->log_health_checks->value, false);
        $this->assert_equals($read_backend->httpcheck_method->value, 'OPTIONS');
        $this->assert_equals($read_backend->monitor_uri->value, '');
        $this->assert_equals($read_backend->monitor_httpversion->value, '');
        $this->assert_is_empty($read_backend->monitor_username->value, '');
        $this->assert_is_empty($read_backend->monitor_domain->value, '');
        $this->assert_equals($read_backend->agent_checks->value, false);
        $this->assert_is_empty($read_backend->agent_port->value, '');
        $this->assert_is_empty($read_backend->persist_cookie_enabled->value, false);
        $this->assert_is_empty($read_backend->persist_cookie_name->value, '');
        $this->assert_is_empty($read_backend->persist_cookie_mode->value, 'passive');
        $this->assert_is_empty($read_backend->haproxy_cookie_dynamic_cookie_key->value, '');
        $this->assert_equals($read_backend->persist_sticky_type->value, 'none');
        $this->assert_is_empty($read_backend->persist_stick_expire->value, '');
        $this->assert_is_empty($read_backend->persist_stick_tablesize->value, '');
        $this->assert_is_empty($read_backend->persist_stick_cookiename->value, '');
        $this->assert_is_empty($read_backend->email_level->value, '');
        $this->assert_is_empty($read_backend->email_to->value, '');
        $this->assert_equals($read_backend->stats_enabled->value, true);
        $this->assert_equals($read_backend->stats_uri->value, '/');
        $this->assert_equals($read_backend->stats_scope->value, ['.']);
        $this->assert_is_empty($read_backend->stats_realm->value, '');
        $this->assert_is_empty($read_backend->stats_username->value, '');
        $this->assert_is_empty($read_backend->stats_admin->value, '');
        $this->assert_equals($read_backend->stats_node->value, 'NODE1');
        $this->assert_is_empty($read_backend->stats_desc->value, '');
        $this->assert_equals($read_backend->stats_refresh->value, 10);
        $this->assert_equals($read_backend->cookie_attribute_secure->value, false);
        $this->assert_is_empty($read_backend->advanced->value, '');
        $this->assert_is_empty($read_backend->advanced_backend->value, '');
        $this->assert_equals($read_backend->transparent_clientip->value, true);
        $this->assert_equals($read_backend->transparent_interface->value, 'lan');

        # Ensure the HAProxyBackendACL model was created
        $this->assert_is_true(HAProxyBackendACL::query(parent_id: $backend->id, id: 0)->exists());
        $this->assert_equals($read_backend->acls->value, [
            [
                'parent_id' => $backend->id,
                'id' => 0,
                'name' => 'example_acl',
                'expression' => 'host_starts_with',
                'value' => 'example',
                'casesensitive' => false,
                'not' => false,
            ],
        ]);

        # Ensure the HAProxyBackendAction model was created
        $this->assert_is_true(HAProxyBackendAction::query(parent_id: $backend->id, id: 0)->exists());
        $this->assert_equals($read_backend->actions->value[0]['action'], 'http-response_lua');
        $this->assert_equals($read_backend->actions->value[0]['acl'], 'example_acl');
        $this->assert_equals($read_backend->actions->value[0]['lua_function'], 'example function');

        # Update the HAProxyBackend model
        $backend->balance->value = 'roundrobin';
        $backend->update();
        $read_backend = HAProxyBackend::query(id: $backend->id)->first();
        $this->assert_equals($read_backend->balance->value, 'roundrobin');

        # Delete the HAProxyBackend model
        $backend->delete();
        $this->assert_is_false(HAProxyBackend::query(id: $backend->id)->exists());
    }

    /**
     * Ensure that the HAProxyBackend::$advanced_backend property is base64 encoded internally.
     */
    public function test_advanced_backend_is_base64(): void {
        # Ensure the advanced_backend property is base64 encoded when converted to internal config value
        $backend = new HAProxyBackend(advanced_backend: 'test');
        $this->assert_equals($backend->advanced_backend->to_internal(), base64_encode('test'));

        # Ensure the advanced_backend property is base64 decoded when converted from internal config value
        $backend = new HAProxyBackend();
        $backend->advanced_backend->from_internal(base64_encode('test'));
        $this->assert_equals($backend->advanced_backend->value, 'test');
    }

    /**
     * Checks that the `haproxy_cookie_dynamic_cookie_key` field is hidden from representations by default.
     */
    public function test_dynamic_cookie_key_is_sensitive(): void {
        $backend = new HAProxyBackend(persist_cookie_enabled: true, haproxy_cookie_dynamic_cookie_key: 'test');
        $this->assert_is_false(array_key_exists('haproxy_cookie_dynamic_cookie_key', $backend->to_representation()));
    }
}
