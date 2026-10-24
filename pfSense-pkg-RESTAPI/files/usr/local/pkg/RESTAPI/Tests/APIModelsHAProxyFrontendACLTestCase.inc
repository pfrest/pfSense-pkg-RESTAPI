<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\TestCase;
use RESTAPI\Models\HAProxyFrontend;
use RESTAPI\Models\HAProxyFrontendACL;

/**
 * Tests HAProxy frontend ACL expression compatibility with parent processing types.
 */
class APIModelsHAProxyFrontendACLTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-haproxy'];

    private const SUPPORTED_EXPRESSIONS = [
        'http' => [
            'host_starts_with',
            'host_ends_with',
            'host_matches',
            'host_regex',
            'host_contains',
            'path_starts_with',
            'path_ends_with',
            'path_matches',
            'path_regex',
            'path_contains',
            'path_dir',
            'url_parameter',
            'ssl_c_verify_code',
            'ssl_c_verify',
            'ssl_c_ca_commonname',
            'source_ip',
            'backendservercount',
            'custom',
        ],
        'https' => [
            'source_ip',
            'backendservercount',
            'ssl_sni_matches',
            'ssl_sni_contains',
            'ssl_sni_starts_with',
            'ssl_sni_ends_with',
            'ssl_sni_regex',
            'custom',
        ],
        'tcp' => ['source_ip', 'backendservercount', 'traffic_is_http', 'traffic_is_ssl', 'custom'],
    ];

    /**
     * Checks every expression against each persisted parent frontend type.
     */
    public function test_expression_type_matrix(): void {
        $choices = (new HAProxyFrontendACL())->expression->choices;

        foreach (self::SUPPORTED_EXPRESSIONS as $type => $supported) {
            $frontend = new HAProxyFrontend(name: "acl_validation_$type", type: $type);
            $frontend->create();

            foreach ($choices as $expression) {
                $acl = new HAProxyFrontendACL(
                    parent_id: $frontend->id,
                    name: 'test_acl',
                    expression: $expression,
                    value: '',
                );

                if (in_array($expression, $supported, true)) {
                    $this->assert_is_true($acl->validate());
                    $this->assert_equals($acl->expression->value, $expression);
                } else {
                    $this->assert_throws_response(
                        response_id: 'HAPROXY_FRONTEND_ACL_EXPRESSION_TYPE_MISMATCH',
                        code: 400,
                        callable: fn() => $acl->validate(),
                    );
                }
            }
        }
    }

    /**
     * Checks nested ACL validation uses the unsaved parent frontend's type.
     */
    public function test_nested_expression_type_matrix(): void {
        $choices = (new HAProxyFrontendACL())->expression->choices;

        foreach (self::SUPPORTED_EXPRESSIONS as $type => $supported) {
            foreach ($choices as $expression) {
                $frontend = new HAProxyFrontend(
                    name: "nested_acl_validation_$type",
                    type: $type,
                    ha_acls: [['name' => 'test_acl', 'expression' => $expression, 'value' => '']],
                );

                if (in_array($expression, $supported, true)) {
                    $this->assert_is_true($frontend->validate());
                    $this->assert_equals($frontend->ha_acls->value[0]['expression'], $expression);
                } else {
                    $this->assert_throws_response(
                        response_id: 'HAPROXY_FRONTEND_ACL_EXPRESSION_TYPE_MISMATCH',
                        code: 400,
                        callable: fn() => $frontend->validate(),
                    );
                }
            }
        }
    }

    /**
     * Ensures incompatible expressions cannot be created or written over an existing ACL.
     */
    public function test_reject_incompatible_writes(): void {
        $frontend = new HAProxyFrontend(name: 'acl_write_validation', type: 'http');
        $frontend->create();
        $acl = new HAProxyFrontendACL(
            parent_id: $frontend->id,
            name: 'test_acl',
            expression: 'traffic_is_ssl',
            value: '',
        );

        $this->assert_throws_response(
            response_id: 'HAPROXY_FRONTEND_ACL_EXPRESSION_TYPE_MISMATCH',
            code: 400,
            callable: fn() => $acl->create(),
        );
        $this->assert_is_false(HAProxyFrontendACL::query(parent_id: $frontend->id)->exists());

        $acl = new HAProxyFrontendACL(
            parent_id: $frontend->id,
            name: 'test_acl',
            expression: 'host_matches',
            value: 'example.com',
        );
        $acl->create();
        $acl->expression->value = 'ssl_sni_matches';

        $this->assert_throws_response(
            response_id: 'HAPROXY_FRONTEND_ACL_EXPRESSION_TYPE_MISMATCH',
            code: 400,
            callable: fn() => $acl->update(),
        );
        $this->assert_equals(
            HAProxyFrontendACL::query(parent_id: $frontend->id, id: $acl->id)->first()->expression->value,
            'host_matches',
        );
    }

    /**
     * Ensures unknown expressions still fail the Field's choice validation.
     */
    public function test_reject_unknown_expression(): void {
        $frontend = new HAProxyFrontend(name: 'acl_unknown_validation', type: 'http');
        $frontend->create();
        $acl = new HAProxyFrontendACL(
            parent_id: $frontend->id,
            name: 'test_acl',
            expression: 'unknown_expression',
            value: '',
        );

        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: fn() => $acl->validate(),
        );
    }
}
