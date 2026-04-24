<?php

namespace RESTAPI\Tests;

use Mcp\Schema\Tool;
use RESTAPI\Core\TestCase;
use RESTAPI\Endpoints\FirewallAliasEndpoint;
use RESTAPI\Endpoints\FirewallAliasesEndpoint;
use RESTAPI\Schemas\MCPSchema;

class APISchemasMCPSchemaTestCase extends TestCase {
    /**
     * Ensures the MCPSchema dynamically generates Tool objects for representative Endpoints and that the tool
     * objects use the official mcp/sdk Tool class.
     */
    public function test_tools_generated_from_endpoints(): void {
        $schema = new MCPSchema();
        $tools = $schema->get_tools();

        $this->assert_is_true(!empty($tools));
        $this->assert_is_true(array_key_exists('readFirewallAlias', $tools));
        $this->assert_is_true($tools['readFirewallAlias'] instanceof Tool);
    }

    /**
     * Ensures the MCPSchema reuses GraphQL operation naming so MCP and GraphQL use the same vocabulary.
     */
    public function test_operation_naming_matches_graphql(): void {
        $schema = new MCPSchema();
        $endpoint = new FirewallAliasEndpoint();
        $endpoint_many = new FirewallAliasesEndpoint();

        $this->assert_equals(
            $schema->endpoint_to_operation_name(operation: 'read', endpoint: $endpoint),
            'readFirewallAlias',
        );
        $this->assert_equals(
            $schema->endpoint_to_operation_name(operation: 'create', endpoint: $endpoint),
            'createFirewallAlias',
        );
        $this->assert_equals(
            $schema->endpoint_to_operation_name(operation: 'query', endpoint: $endpoint_many),
            'queryFirewallAliases',
        );
        $this->assert_equals(
            $schema->endpoint_to_operation_name(operation: 'replaceAll', endpoint: $endpoint_many),
            'replaceAllFirewallAliases',
        );
    }

    /**
     * Ensures every supported CRUD operation produces a tool for the FirewallAlias endpoints.
     */
    public function test_endpoint_tool_coverage(): void {
        $schema = new MCPSchema();
        $tools = $schema->get_tools();

        # Single-object endpoints
        $this->assert_is_true(array_key_exists('readFirewallAlias', $tools));
        $this->assert_is_true(array_key_exists('createFirewallAlias', $tools));
        $this->assert_is_true(array_key_exists('updateFirewallAlias', $tools));
        $this->assert_is_true(array_key_exists('deleteFirewallAlias', $tools));

        # Many-object endpoints
        $this->assert_is_true(array_key_exists('queryFirewallAliases', $tools));
        $this->assert_is_true(array_key_exists('replaceAllFirewallAliases', $tools));
        $this->assert_is_true(array_key_exists('deleteManyFirewallAliases', $tools));
        $this->assert_is_true(array_key_exists('deleteAllFirewallAliases', $tools));
    }

    /**
     * Ensures the MCPEndpoint and GraphQLEndpoint are excluded from the generated tool catalog so the MCP
     * server cannot recursively expose itself or the GraphQL dispatcher.
     */
    public function test_excludes_mcp_and_graphql_endpoints(): void {
        $schema = new MCPSchema();
        $tools = $schema->get_tools();
        $names = array_keys($tools);

        foreach ($names as $name) {
            $this->assert_is_false(str_contains(strtolower($name), 'graphql'));
            $this->assert_is_false(str_contains(strtolower($name), 'mcp'));
        }
    }

    /**
     * Ensures the input schema for a representative read tool requires an `id` field.
     */
    public function test_read_tool_requires_id(): void {
        $schema = new MCPSchema();
        $tool = $schema->get_tools()['readFirewallAlias'];

        $this->assert_equals($tool->inputSchema['type'], 'object');
        $this->assert_is_true(in_array('id', $tool->inputSchema['required']));
        $this->assert_equals($tool->inputSchema['properties']['id']['type'], 'integer');
    }

    /**
     * Ensures the input schema for a representative create tool exposes Field properties as JSON Schema.
     */
    public function test_create_tool_includes_field_properties(): void {
        $schema = new MCPSchema();
        $tool = $schema->get_tools()['createFirewallAlias'];
        $properties = $tool->inputSchema['properties'];

        $this->assert_is_true(array_key_exists('name', $properties));
        $this->assert_is_true(array_key_exists('type', $properties));
        $this->assert_equals($properties['name']['type'], 'string');

        # The `type` field has choices, so it should be exposed as an enum.
        $this->assert_is_true(array_key_exists('enum', $properties['type']));
    }

    /**
     * Ensures that a handler is registered for every generated tool so `tools/call` requests can be dispatched.
     */
    public function test_tool_handlers_registered(): void {
        $schema = new MCPSchema();
        $tools = $schema->get_tools();
        $handlers = $schema->get_tool_handlers();

        foreach ($tools as $name => $_tool) {
            $this->assert_is_true(array_key_exists($name, $handlers));
            $this->assert_is_true(in_array(
                $handlers[$name]['action'],
                ['query', 'read', 'create', 'update', 'delete', 'replace_all', 'delete_many', 'delete_all'],
            ));
        }
    }

    /**
     * Ensures the schema string is valid JSON containing a `tools` array, allowing it to be persisted to disk.
     */
    public function test_get_schema_str_returns_valid_json(): void {
        $schema = new MCPSchema();
        $decoded = json_decode($schema->get_schema_str(), associative: true);
        $this->assert_type($decoded, 'array');
        $this->assert_is_true(array_key_exists('tools', $decoded));
        $this->assert_type($decoded['tools'], 'array');
    }
}

