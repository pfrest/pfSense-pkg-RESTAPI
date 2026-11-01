<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\HAProxyFrontend;
use RESTAPI\Models\HAProxyFrontendAddress;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Models\RESTAPISettings;
use const RESTAPI\Core\Globals\HAL_REL_NAMESPACE;

/**
 * Tests interface address storage and HATEOAS links on HAProxy frontends.
 */
class APIModelsHAProxyFrontendAddressTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-haproxy'];

    /**
     * Checks interface address creation, reads, updates, and related links in all representation modes.
     */
    public function test_interface_addresses(): void {
        $dirty_file = '/var/run/haproxy.conf.dirty';
        $was_dirty = file_exists($dirty_file);
        $settings = new RESTAPISettings();

        try {
            foreach (['id', 'if', 'descr'] as $mode) {
                $settings->represent_interfaces_as->value = $mode;
                $settings->update();
                $interface = NetworkInterface::query(query_params: ['id' => 'wan'])->first();
                $represented = $mode === 'id' ? 'wan' : $interface->$mode->value;
                $frontend = new HAProxyFrontend(
                    name: 'test_interface_addresses_' . $mode,
                    type: 'http',
                    a_extaddr: [['extaddr' => $represented . '_ipv4', 'extaddr_port' => '8080']],
                );
                $frontend->create();

                try {
                    $address = HAProxyFrontendAddress::query(parent_id: $frontend->id, id: 0)->first();
                    $this->assert_equals($address->extaddr->value, $represented . '_ipv4');
                    $stored = Model::get_config($frontend->get_config_path() . '/' . $frontend->id);
                    $this->assert_equals($stored['a_extaddr']['item'][0]['extaddr'], 'wan_ipv4');
                    $links = $address->get_related_links()->to_representation()['_links'];
                    $link = $links[HAL_REL_NAMESPACE . ':field:extaddr'];
                    $this->assert_equals($link['id'], 'wan');
                    $this->assert_str_contains($link['href'], '/api/v2/interface?id=wan');

                    $address->extaddr->value = $represented . '_ipv6';
                    $address->update();
                    $address = HAProxyFrontendAddress::query(parent_id: $frontend->id, id: 0)->first();
                    $this->assert_equals($address->extaddr->value, $represented . '_ipv6');
                    $this->assert_equals($address->to_internal()['extaddr'], 'wan_ipv6');

                    foreach (['custom', 'any_ipv4', 'any_ipv6', 'localhost_ipv4', 'localhost_ipv6'] as $keyword) {
                        $address->extaddr->value = $keyword;
                        $address->extaddr_custom->value = '127.0.0.1';
                        $address->update();
                        $address = HAProxyFrontendAddress::query(parent_id: $frontend->id, id: 0)->first();
                        $this->assert_equals($address->extaddr->value, $keyword);
                        $this->assert_equals($address->to_internal()['extaddr'], $keyword);
                        $links = $address->get_related_links()->to_representation()['_links'];
                        $this->assert_is_false(array_key_exists(HAL_REL_NAMESPACE . ':field:extaddr', $links));
                    }
                } finally {
                    $frontend->delete();
                }
            }
        } finally {
            if (!$was_dirty and file_exists($dirty_file)) {
                unlink($dirty_file);
            }
        }
    }
}
