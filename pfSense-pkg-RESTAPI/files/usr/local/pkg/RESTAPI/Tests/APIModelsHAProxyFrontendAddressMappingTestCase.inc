<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\HAProxyFrontend;
use RESTAPI\Models\HAProxyFrontendAddress;

/**
 * Checks the public listener representation against the native HAProxy configuration keys.
 */
class APIModelsHAProxyFrontendAddressMappingTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-haproxy'];

    /**
     * Existing API clients write advanced options to the native key without changing their request field.
     */
    public function test_advanced_options_use_native_key(): void {
        $frontend = new HAProxyFrontend(
            data: [
                'name' => 'api_address_mapping_test',
                'type' => 'http',
                'a_extaddr' => [
                    ['extaddr' => 'localhost_ipv4', 'extaddr_port' => '18080', 'exaddr_advanced' => 'backlog 128'],
                ],
            ],
            dry_run: true,
        );
        $frontend->create();
        $internal = $frontend->to_internal();
        $address = $internal['a_extaddr']['item'][0];
        $this->assert_equals($address['extaddr_advanced'], 'backlog 128');
        $this->assert_is_false(array_key_exists('exaddr_advanced', $address));

        # Read through the regular model path using only a process-local fixture.
        Model::set_config("{$frontend->get_config_path()}/{$frontend->id}", $internal);
        $read = new HAProxyFrontendAddress(id: 0, parent_id: $frontend->id);
        $this->assert_equals($read->exaddr_advanced->value, 'backlog 128');
        $read->exaddr_advanced->value = 'backlog 256';
        $read->dry_run = true;
        $read->update();
        $this->assert_equals($read->to_internal()['extaddr_advanced'], 'backlog 256');
        $this->assert_equals(Model::get_config("{$frontend->get_config_path()}/{$frontend->id}"), $internal);
    }

    /**
     * GUI-created native options remain visible through the existing public API property.
     */
    public function test_native_advanced_options_are_read(): void {
        $frontend = new HAProxyFrontend(data: ['name' => 'api_native_address_test', 'type' => 'http'], dry_run: true);
        $frontend->create();
        $internal = $frontend->to_internal();
        $internal['a_extaddr']['item'] = [
            ['extaddr' => 'localhost_ipv4', 'extaddr_port' => '18081', 'extaddr_advanced' => 'backlog 64'],
        ];
        Model::set_config("{$frontend->get_config_path()}/{$frontend->id}", $internal);
        $read = new HAProxyFrontendAddress(id: 0, parent_id: $frontend->id);
        $this->assert_equals($read->exaddr_advanced->value, 'backlog 64');
    }

    /**
     * Listeners without advanced options keep the existing empty-string default.
     */
    public function test_advanced_options_default_to_empty(): void {
        $frontend = new HAProxyFrontend(
            data: [
                'name' => 'api_default_address_test',
                'type' => 'http',
                'a_extaddr' => [['extaddr' => 'localhost_ipv4', 'extaddr_port' => '18082']],
            ],
            dry_run: true,
        );
        $frontend->create();
        $this->assert_equals($frontend->to_internal()['a_extaddr']['item'][0]['extaddr_advanced'], '');
        $address = new HAProxyFrontendAddress();
        $this->assert_equals($address->exaddr_advanced->to_openapi_property()['type'], 'string');
    }
}
