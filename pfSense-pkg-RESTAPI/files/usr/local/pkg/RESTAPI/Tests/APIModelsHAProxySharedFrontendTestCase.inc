<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\HAProxyFrontend;

/**
 * Tests shared frontend relationships without persisting fixtures or applying HAProxy changes.
 */
class APIModelsHAProxySharedFrontendTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-haproxy'];

    private const FRONTEND_PATH = 'installedpackages/haproxy/ha_backends/item';

    /**
     * Clears only the process-local frontend fixtures, leaving package configuration available.
     */
    private function reset_frontends(): void {
        Model::set_config(self::FRONTEND_PATH, []);
    }

    /**
     * Validates a frontend and stores its native representation only in process memory.
     * @param array $data The frontend representation to validate.
     * @return HAProxyFrontend The fixture reloaded through the normal Model read path.
     */
    private function fixture(array $data): HAProxyFrontend {
        $frontend = new HAProxyFrontend(data: $data, dry_run: true);
        $frontend->create();
        Model::set_config(self::FRONTEND_PATH . "/{$frontend->id}", $frontend->to_internal());
        return new HAProxyFrontend(id: $frontend->id, dry_run: true);
    }

    /**
     * Keeps existing independent frontends compatible when the new native fields are absent.
     */
    public function test_independent_frontend_defaults(): void {
        $this->reset_frontends();
        $primary = $this->fixture(['name' => 'primary', 'type' => 'http']);
        $internal = $primary->to_internal();
        unset($internal['secondary'], $internal['primary_frontend'], $internal['ssloffload']);
        Model::set_config(self::FRONTEND_PATH . "/{$primary->id}", $internal);

        $read = new HAProxyFrontend(id: $primary->id, dry_run: true);
        $this->assert_is_false($read->secondary->value);
        $this->assert_equals($read->primary_frontend->value, null);
        $this->assert_is_false($read->ssloffload->value);
        $this->assert_equals($read->type->value, 'http');
        $read->update();
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH . "/{$primary->id}"), $internal);
    }

    /**
     * Creates a shared frontend without its own mode and resolves its primary by name.
     */
    public function test_shared_frontend_inherits_primary(): void {
        $this->reset_frontends();
        $primary = $this->fixture([
            'name' => 'primary',
            'type' => 'http',
            'a_extaddr' => [['extaddr' => 'localhost_ipv4', 'extaddr_port' => '18080']],
        ]);
        $shared = $this->fixture([
            'name' => 'shared',
            'secondary' => true,
            'primary_frontend' => 'primary',
        ]);

        $this->assert_is_true($shared->secondary->value);
        $this->assert_equals($shared->primary_frontend->value, 'primary');
        $this->assert_equals($shared->type->value, null);
        $this->assert_is_empty($shared->a_extaddr->value);
        $this->assert_equals($shared->to_internal()['secondary'], 'yes');
        $this->assert_equals(get_primaryfrontend($shared->to_internal()), $primary->to_internal());
        $this->assert_equals(get_frontend_ipport($shared->to_internal()), get_frontend_ipport($primary->to_internal()));
    }

    /**
     * Accepts hidden GUI mode and address values while native routing still uses the primary.
     */
    public function test_shared_frontend_accepts_hidden_gui_values(): void {
        $this->reset_frontends();
        $primary = $this->fixture([
            'name' => 'primary',
            'type' => 'http',
            'a_extaddr' => [['extaddr' => 'localhost_ipv4', 'extaddr_port' => '18080']],
        ]);
        $shared = $this->fixture([
            'name' => 'shared',
            'secondary' => true,
            'primary_frontend' => 'primary',
            'type' => 'tcp',
            'a_extaddr' => [['extaddr' => 'localhost_ipv4', 'extaddr_port' => '19090']],
        ]);

        $this->assert_equals($shared->type->value, null);
        $this->assert_equals($shared->a_extaddr->value[0]['extaddr_port'], '19090');
        $this->assert_equals(get_frontend_ipport($shared->to_internal()), get_frontend_ipport($primary->to_internal()));
        $before = Model::get_config(self::FRONTEND_PATH);
        $shared->descr->value = 'Updated shared frontend';
        $shared->update();
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
    }

    /**
     * Rejects absent, unknown, self and shared parents before changing any configuration.
     */
    public function test_reject_invalid_primary_references(): void {
        $this->reset_frontends();
        $this->fixture(['name' => 'primary', 'type' => 'http']);
        $this->fixture(['name' => 'other_shared', 'secondary' => true, 'primary_frontend' => 'primary']);
        $before = Model::get_config(self::FRONTEND_PATH);

        foreach ([null, '', 'missing', 'shared', 'other_shared'] as $primary_name) {
            $this->assert_throws_response(
                response_id: 'HAPROXY_FRONTEND_INVALID_PRIMARY_FRONTEND',
                code: 400,
                callable: function () use ($primary_name) {
                    (new HAProxyFrontend(
                        data: ['name' => 'shared', 'secondary' => true, 'primary_frontend' => $primary_name],
                        dry_run: true,
                    ))->create();
                },
            );
            $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
        }
    }

    /**
     * Rejects renaming or converting a primary while shared frontends still reference it.
     */
    public function test_reject_changes_that_orphan_shared_frontends(): void {
        $this->reset_frontends();
        $primary = $this->fixture(['name' => 'primary', 'type' => 'http']);
        $this->fixture(['name' => 'other_primary', 'type' => 'http']);
        $this->fixture(['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'primary']);
        $before = Model::get_config(self::FRONTEND_PATH);

        foreach ([['name' => 'renamed'], ['secondary' => true, 'primary_frontend' => 'other_primary']] as $changes) {
            $this->assert_throws_response(
                response_id: 'HAPROXY_FRONTEND_PRIMARY_IN_USE',
                code: 400,
                callable: function () use ($primary, $changes) {
                    $updated = new HAProxyFrontend(dry_run: true);
                    $updated->from_representation(['id' => $primary->id] + $changes);
                    $updated->update();
                },
            );
            $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
        }
    }

    /**
     * Rejects renaming a frontend and converting it into a child of its previous name in one update.
     */
    public function test_reject_renamed_primary_referencing_itself(): void {
        $this->reset_frontends();
        $primary = $this->fixture(['name' => 'primary', 'type' => 'http']);
        $before = Model::get_config(self::FRONTEND_PATH);

        $this->assert_throws_response(
            response_id: 'HAPROXY_FRONTEND_INVALID_PRIMARY_FRONTEND',
            code: 400,
            callable: function () use ($primary) {
                $updated = new HAProxyFrontend(dry_run: true);
                $updated->from_representation([
                    'id' => $primary->id,
                    'name' => 'renamed',
                    'secondary' => true,
                    'primary_frontend' => 'primary',
                ]);
                $updated->update();
            },
        );
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
    }

    /**
     * Protects a referenced primary from deletion while allowing the shared frontend to be removed.
     */
    public function test_reject_referenced_primary_deletion(): void {
        $this->reset_frontends();
        $primary = $this->fixture(['name' => 'primary', 'type' => 'http']);
        $shared = $this->fixture(['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'primary']);
        $before = Model::get_config(self::FRONTEND_PATH);

        $this->assert_throws_response(
            response_id: 'MODEL_CANNOT_BE_DELETED_WHILE_REFERENCED',
            code: 409,
            callable: function () use ($primary) {
                $primary->delete();
            },
        );
        $shared->delete();
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
    }

    /**
     * Checks deletion dependencies using the stored frontend name even if the request overrides its name.
     */
    public function test_delete_cannot_override_referenced_primary_name(): void {
        $this->reset_frontends();
        $primary = $this->fixture(['name' => 'primary', 'type' => 'http']);
        $this->fixture(['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'primary']);
        $before = Model::get_config(self::FRONTEND_PATH);

        $this->assert_throws_response(
            response_id: 'MODEL_CANNOT_BE_DELETED_WHILE_REFERENCED',
            code: 409,
            callable: function () use ($primary) {
                $deleted = new HAProxyFrontend(dry_run: true);
                $deleted->from_representation(['id' => $primary->id, 'name' => 'unreferenced']);
                $deleted->delete();
            },
        );
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
    }

    /**
     * Does not treat the GUI's inactive primary self-reference as a dependent shared frontend.
     */
    public function test_gui_primary_self_reference_does_not_block_changes(): void {
        $this->reset_frontends();
        $primary = $this->fixture([
            'name' => 'primary',
            'type' => 'http',
            'primary_frontend' => 'primary',
        ]);
        $before = Model::get_config(self::FRONTEND_PATH);

        $this->assert_is_empty($primary->get_model_references()->model_objects);
        $primary->delete();
        $primary->name->value = 'renamed';
        $primary->update();
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
    }

    /**
     * Updates one shared frontend's representation without altering its primary or sibling.
     */
    public function test_shared_update_preserves_primary_and_sibling(): void {
        $this->reset_frontends();
        $primary = $this->fixture(['name' => 'primary', 'type' => 'http']);
        $first = $this->fixture(['name' => 'first', 'secondary' => true, 'primary_frontend' => 'primary']);
        $second = $this->fixture(['name' => 'second', 'secondary' => true, 'primary_frontend' => 'primary']);
        $before = Model::get_config(self::FRONTEND_PATH);

        $first->descr->value = 'Updated first application';
        $first->ssloffload->value = true;
        $first->update();
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
        Model::set_config(self::FRONTEND_PATH . "/{$first->id}", $first->to_internal());

        $after = Model::get_config(self::FRONTEND_PATH);
        $this->assert_equals($after[$primary->id], $before[$primary->id]);
        $this->assert_equals($after[$second->id], $before[$second->id]);
        $this->assert_equals($after[$first->id]['descr'], 'Updated first application');
        $this->assert_equals($after[$first->id]['ssloffload'], 'yes');
    }

    /**
     * Stores the shared certificate toggle using HAProxy's native yes and empty-string values.
     */
    public function test_shared_ssl_offload_round_trip(): void {
        $this->reset_frontends();
        $this->fixture(['name' => 'primary', 'type' => 'http']);

        foreach ([false, true] as $enabled) {
            $shared = $this->fixture([
                'name' => $enabled ? 'shared_tls' : 'shared_plain',
                'secondary' => true,
                'primary_frontend' => 'primary',
                'ssloffload' => $enabled,
            ]);
            $this->assert_equals($shared->ssloffload->value, $enabled);
            $this->assert_equals($shared->to_internal()['ssloffload'], $enabled ? 'yes' : '');
        }
    }

    /**
     * Requires an explicit processing mode for primary frontends, including converted shared frontends.
     */
    public function test_primary_type_is_required(): void {
        $this->reset_frontends();
        $this->assert_throws_response(
            response_id: 'FIELD_IS_REQUIRED',
            code: 400,
            callable: function () {
                (new HAProxyFrontend(data: ['name' => 'primary'], dry_run: true))->create();
            },
        );

        $this->fixture(['name' => 'primary', 'type' => 'http']);
        $shared = $this->fixture(['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'primary']);
        $shared->secondary->value = false;
        $this->assert_throws_response(
            response_id: 'FIELD_IS_REQUIRED',
            code: 400,
            callable: function () use ($shared) {
                $shared->update();
            },
        );
        $shared->type->value = 'tcp';
        $shared->update();
    }

    /**
     * Advertises a nullable optional type so schema-generated clients can create shared frontends.
     */
    public function test_shared_type_schema_is_nullable_and_optional(): void {
        $frontend = new HAProxyFrontend();
        $schema = $frontend->to_openapi_schema();

        $this->assert_is_true($schema['properties']['type']['nullable']);
        $this->assert_is_false(in_array('type', $schema['required'] ?? [], true));
        $this->assert_is_false($frontend->type->required);
        $this->assert_equals($schema['properties']['type']['enum'], ['http', 'https', 'tcp']);
    }

    /**
     * Validates a complete replacement ordered primary-first without mutating stored frontends.
     */
    public function test_dry_run_replacement_primary_first(): void {
        $this->reset_frontends();
        $this->fixture(['name' => 'existing', 'type' => 'tcp']);
        $before = Model::get_config(self::FRONTEND_PATH);
        $result = (new HAProxyFrontend(dry_run: true))->replace_all([
            ['name' => 'primary', 'type' => 'http'],
            ['name' => 'first', 'secondary' => true, 'primary_frontend' => 'primary'],
            ['name' => 'second', 'secondary' => true, 'primary_frontend' => 'primary', 'ssloffload' => true],
        ]);

        $this->assert_equals(count($result->model_objects), 3);
        $this->assert_equals($result->model_objects[1]->primary_frontend->value, 'primary');
        $this->assert_is_true($result->model_objects[2]->ssloffload->value);
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
    }

    /**
     * Rejects replacements whose primary appears too late or is missing without changing configuration.
     */
    public function test_dry_run_replacement_rejects_invalid_parent_order(): void {
        $this->reset_frontends();
        $before = Model::get_config(self::FRONTEND_PATH);

        foreach (
            [
                [
                    ['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'primary'],
                    ['name' => 'primary', 'type' => 'http'],
                ],
                [
                    ['name' => 'primary', 'type' => 'http'],
                    ['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'missing'],
                ],
            ]
            as $replacement
        ) {
            $this->assert_throws_response(
                response_id: 'HAPROXY_FRONTEND_INVALID_PRIMARY_FRONTEND',
                code: 400,
                callable: function () use ($replacement) {
                    (new HAProxyFrontend(dry_run: true))->replace_all($replacement);
                },
            );
            $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
            # A failed generic replace_all may leave validation objects in the process-local cache.
            Model::clear_model_cache();
        }
    }

    /**
     * Rejects a replacement that references an existing primary omitted from the replacement set.
     */
    public function test_dry_run_replacement_rejects_omitted_existing_primary(): void {
        $this->reset_frontends();
        $this->fixture(['name' => 'existing_primary', 'type' => 'http']);
        $before = Model::get_config(self::FRONTEND_PATH);

        $this->assert_throws_response(
            response_id: 'HAPROXY_FRONTEND_INVALID_PRIMARY_FRONTEND',
            code: 400,
            callable: function () {
                (new HAProxyFrontend(dry_run: true))->replace_all([
                    ['name' => 'shared', 'secondary' => true, 'primary_frontend' => 'existing_primary'],
                ]);
            },
        );
        $this->assert_equals(Model::get_config(self::FRONTEND_PATH), $before);
        Model::clear_model_cache();
    }
}
