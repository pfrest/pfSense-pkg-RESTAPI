<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Auth;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\FirewallAlias;
use RESTAPI\Models\MCP;
use RESTAPI\Models\RESTAPISettings;

class APIModelsMCPTestCase extends TestCase {
    /**
     * @var Auth $auth Auth object passed to the MCP model so privilege checks succeed.
     */
    public Auth $auth;

    /**
     * Set up Auth and FirewallAlias models for testing. Also enables the MCP server in RESTAPISettings since
     * it is disabled by default.
     */
    public function setup(): void {
        # Enable the MCP server. The TestCase framework restores the original config after every test method.
        $settings = new RESTAPISettings(async: false);
        $settings->mcp_enabled->value = true;
        $settings->update();

        # Use an admin Auth object so per-tool privilege checks succeed.
        $this->auth = new Auth();
        $this->auth->username = 'admin';

        # Create a few FirewallAlias models to query against.
        $alias = new FirewallAlias();
        $alias->replace_all([
            [
                'name' => 'mcp_test1',
                'type' => 'host',
                'descr' => 'MCP test alias 1',
                'address' => ['1.2.3.4'],
                'detail' => ['mcp detail 1'],
            ],
            [
                'name' => 'mcp_test2',
                'type' => 'network',
                'descr' => 'MCP test alias 2',
                'address' => ['1.2.3.4/32'],
                'detail' => ['mcp detail 2'],
            ],
        ]);
    }

    /**
     * Clean up FirewallAlias models after testing.
     */
    public function teardown(): void {
        $alias = new FirewallAlias();
        $alias->delete_all();
    }

    /**
     * Ensures that an MCP `initialize` request returns a JSON-RPC envelope with the server capabilities.
     */
    public function test_initialize(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'initialize';
        $mcp->params->value = ['protocolVersion' => '2025-06-18'];
        $mcp->id->value = '1';
        $mcp->create();

        $this->assert_equals($mcp->result['jsonrpc'], '2.0');
        $this->assert_equals($mcp->result['id'], 1);
        $this->assert_is_true(isset($mcp->result['result']));
    }

    /**
     * Ensures that an MCP `tools/list` request returns a list of tools that includes representative tools for
     * a known Endpoint (FirewallAlias).
     */
    public function test_tools_list_includes_known_tools(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/list';
        $mcp->id->value = '2';
        $mcp->create();

        $this->assert_equals($mcp->result['jsonrpc'], '2.0');
        $tools = json_decode(json_encode($mcp->result['result']['tools']), true);
        $tool_names = array_column($tools, 'name');

        $this->assert_is_true(in_array('readFirewallAlias', $tool_names));
        $this->assert_is_true(in_array('queryFirewallAliases', $tool_names));
        $this->assert_is_true(in_array('createFirewallAlias', $tool_names));
        $this->assert_is_true(in_array('updateFirewallAlias', $tool_names));
        $this->assert_is_true(in_array('deleteFirewallAlias', $tool_names));
        $this->assert_is_true(in_array('replaceAllFirewallAliases', $tool_names));
        $this->assert_is_true(in_array('deleteManyFirewallAliases', $tool_names));
        $this->assert_is_true(in_array('deleteAllFirewallAliases', $tool_names));
    }

    /**
     * Ensures that the `tools/list` response excludes the MCP endpoint itself to prevent recursive exposure.
     */
    public function test_tools_list_excludes_mcp_endpoint(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/list';
        $mcp->id->value = '3';
        $mcp->create();

        $tools = json_decode(json_encode($mcp->result['result']['tools']), true);
        $tool_names = array_column($tools, 'name');

        # The MCP endpoint should never expose itself as a tool.
        $this->assert_is_false(in_array('createMcp', $tool_names));
        $this->assert_is_false(in_array('readMcp', $tool_names));
    }

    /**
     * Ensures the `tools/call` resolver can read an existing object via the dynamically generated read tool.
     */
    public function test_tools_call_read(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = ['name' => 'readFirewallAlias', 'arguments' => ['id' => 0]];
        $mcp->id->value = '4';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);
        $payload = json_decode($result['content'][0]['text'], associative: true);
        $this->assert_equals($payload['name'], 'mcp_test1');
        $this->assert_equals($payload['type'], 'host');
    }

    /**
     * Ensures the `tools/call` resolver can query objects via the dynamically generated query tool.
     */
    public function test_tools_call_query(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = ['name' => 'queryFirewallAliases', 'arguments' => []];
        $mcp->id->value = '5';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);
        $payload = json_decode($result['content'][0]['text'], associative: true);
        $this->assert_equals(count($payload), 2);
        $this->assert_equals($payload[0]['name'], 'mcp_test1');
        $this->assert_equals($payload[1]['name'], 'mcp_test2');
    }

    /**
     * Ensures the `tools/call` resolver can create a new object via the dynamically generated create tool.
     */
    public function test_tools_call_create(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = [
            'name' => 'createFirewallAlias',
            'arguments' => [
                'name' => 'mcp_test3',
                'type' => 'host',
                'descr' => 'MCP test alias 3',
                'address' => ['9.9.9.9'],
                'detail' => ['mcp detail 3'],
            ],
        ];
        $mcp->id->value = '6';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);

        # Ensure the alias was actually created.
        $alias_q = FirewallAlias::query(name: 'mcp_test3');
        $this->assert_is_true($alias_q->exists());
        $alias_q->first()->delete();
    }

    /**
     * Ensures the `tools/call` resolver can update an object via the dynamically generated update tool.
     */
    public function test_tools_call_update(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = [
            'name' => 'updateFirewallAlias',
            'arguments' => ['id' => 0, 'descr' => 'updated by mcp'],
        ];
        $mcp->id->value = '7';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);

        $alias = new FirewallAlias(id: 0);
        $this->assert_equals($alias->descr->value, 'updated by mcp');
    }

    /**
     * Ensures the `tools/call` resolver can delete an object via the dynamically generated delete tool.
     */
    public function test_tools_call_delete(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = ['name' => 'deleteFirewallAlias', 'arguments' => ['id' => 0]];
        $mcp->id->value = '8';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);
        $this->assert_is_false(FirewallAlias::query(name: 'mcp_test1')->exists());
    }

    /**
     * Ensures the `tools/call` resolver can replace all objects via the dynamically generated replace_all tool.
     */
    public function test_tools_call_replace_all(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = [
            'name' => 'replaceAllFirewallAliases',
            'arguments' => [
                'objects' => [
                    [
                        'name' => 'mcp_replaced',
                        'type' => 'host',
                        'descr' => 'replaced',
                        'address' => ['8.8.8.8'],
                        'detail' => ['replaced'],
                    ],
                ],
            ],
        ];
        $mcp->id->value = '9';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);
        $aliases = FirewallAlias::read_all();
        $this->assert_equals($aliases->count(), 1);
        $this->assert_equals($aliases->first()->name->value, 'mcp_replaced');
    }

    /**
     * Ensures the `tools/call` resolver can delete many objects via the dynamically generated delete_many tool.
     */
    public function test_tools_call_delete_many(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = [
            'name' => 'deleteManyFirewallAliases',
            'arguments' => ['query_params' => ['name' => 'mcp_test1']],
        ];
        $mcp->id->value = '10';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);
        $this->assert_is_false(FirewallAlias::query(name: 'mcp_test1')->exists());
        $this->assert_is_true(FirewallAlias::query(name: 'mcp_test2')->exists());
    }

    /**
     * Ensures the `tools/call` resolver can delete all objects via the dynamically generated delete_all tool.
     */
    public function test_tools_call_delete_all(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = ['name' => 'deleteAllFirewallAliases', 'arguments' => []];
        $mcp->id->value = '11';
        $mcp->create();

        $result = $mcp->result['result'];
        $this->assert_is_false($result['isError']);
        $this->assert_equals(FirewallAlias::read_all()->count(), 0);
    }

    /**
     * Ensures unknown tool names return a JSON-RPC InvalidParams error.
     */
    public function test_tools_call_unknown_tool(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/call';
        $mcp->params->value = ['name' => 'thisToolDoesNotExist', 'arguments' => []];
        $mcp->id->value = '12';
        $mcp->create();

        $this->assert_is_true(isset($mcp->result['error']));
        $this->assert_equals($mcp->result['error']['code'], -32602);
    }

    /**
     * Ensures unknown JSON-RPC methods return a JSON-RPC MethodNotFound error.
     */
    public function test_unknown_method(): void {
        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'unknown/method';
        $mcp->id->value = '13';
        $mcp->create();

        $this->assert_is_true(isset($mcp->result['error']));
        $this->assert_equals($mcp->result['error']['code'], -32601);
    }

    /**
     * Ensures the MCP model throws a ServiceUnavailableError when the MCP server is disabled.
     */
    public function test_mcp_disabled_throws_service_unavailable(): void {
        # Disable the MCP server for this test.
        $settings = new RESTAPISettings(async: false);
        $settings->mcp_enabled->value = false;
        $settings->update();

        $mcp = new MCP(client: $this->auth);
        $mcp->method->value = 'tools/list';
        $mcp->id->value = '14';

        $this->assert_throws_response(
            response_id: 'MCP_SERVER_NOT_ENABLED',
            code: 503,
            callable: function () use ($mcp) {
                $mcp->create();
            },
        );
    }

    /**
     * Ensures the MCP Model's internal callable returns an empty array.
     */
    public function test_internal_callable(): void {
        $mcp = new MCP(client: $this->auth);
        $this->assert_equals($mcp->get_internal(), []);
    }

    /**
     * Ensures the MCP `result` property is excluded from serialization to avoid breaking the Model's
     * `initial_object` property.
     */
    public function test_result_property_excluded_from_serialization(): void {
        $mcp = new MCP(client: $this->auth);
        $this->assert_is_true(!in_array('result', $mcp->__sleep()));
        $this->assert_does_not_throw(
            callable: function () use ($mcp) {
                serialize($mcp);
            },
        );
    }
}

