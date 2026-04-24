<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Endpoints\MCPEndpoint;
use RESTAPI\Models\RESTAPISettings;
use RESTAPI\Responses\MCPResponse;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\Success;
use RESTAPI\Responses\ValidationError;

class APIEndpointsMCPEndpointTestCase extends TestCase {
    /**
     * Ensures the response_handler for the MCPEndpoint correctly converts standard REST API responses into MCP
     * JSON-RPC envelopes for both successful and unsuccessful responses.
     */
    public function test_response_handler(): void {
        $not_found_resp = new NotFoundError(message: 'Test not found', response_id: 'TEST_NOT_FOUND');
        $bad_request_resp = new ValidationError(message: 'Test bad request', response_id: 'TEST_BAD_REQUEST');
        $success_resp = new Success(message: 'Test success', response_id: 'TEST_SUCCESS');

        $endpoint = new MCPEndpoint();

        $this->assert_is_true($endpoint->response_handler($not_found_resp) instanceof MCPResponse);
        $this->assert_is_true($endpoint->response_handler($bad_request_resp) instanceof MCPResponse);
        $this->assert_is_true($endpoint->response_handler($success_resp) instanceof MCPResponse);
    }

    /**
     * Ensures the MCPEndpoint declares the expected URL, model, request methods and response types so it
     * stays in sync with the documented contract.
     */
    public function test_endpoint_attributes(): void {
        $endpoint = new MCPEndpoint();
        $this->assert_equals($endpoint->url, '/api/v2/mcp');
        $this->assert_equals($endpoint->model_name, 'MCP');
        $this->assert_equals($endpoint->request_method_options, ['POST']);
        $this->assert_equals($endpoint->response_types, ['MCPResponse']);
    }

    /**
     * Make a POST request to /api/v2/mcp with the MCP server enabled to ensure it works end-to-end. The MCP
     * server is enabled before the request and reset to disabled afterwards by the TestCase config restore.
     */
    public function test_endpoint_e2e(): void {
        # Enable the MCP server so the endpoint will respond.
        $settings = new RESTAPISettings(async: false);
        $settings->mcp_enabled->value = true;
        $settings->update();

        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: 'https://localhost/api/v2/mcp',
            method: 'POST',
            data: ['jsonrpc' => '2.0', 'id' => 1, 'method' => 'tools/list'],
            headers: ['Content-Type' => 'application/json'],
            username: 'admin',
            password: 'pfsense',
            validate_certs: false,
        );
        $resp = json_decode($json_resp, associative: true);

        $this->assert_equals($resp['jsonrpc'], '2.0');
        $this->assert_is_true(isset($resp['result']['tools']));
        $tool_names = array_column($resp['result']['tools'], 'name');
        $this->assert_is_true(in_array('readSystemHostname', $tool_names));
    }

    /**
     * Make a POST request to /api/v2/mcp while the MCP server is disabled to ensure the endpoint returns the
     * expected MCP error envelope rather than executing tools.
     */
    public function test_endpoint_e2e_disabled(): void {
        # Explicitly disable the MCP server.
        $settings = new RESTAPISettings(async: false);
        $settings->mcp_enabled->value = false;
        $settings->update();

        $json_resp = \RESTAPI\Core\Tools\http_request(
            url: 'https://localhost/api/v2/mcp',
            method: 'POST',
            data: ['jsonrpc' => '2.0', 'id' => 1, 'method' => 'tools/list'],
            headers: ['Content-Type' => 'application/json'],
            username: 'admin',
            password: 'pfsense',
            validate_certs: false,
        );
        $resp = json_decode($json_resp, associative: true);

        # The endpoint should wrap the ServiceUnavailableError into a JSON-RPC error envelope.
        $this->assert_is_true(isset($resp['error']));
        $this->assert_equals($resp['error']['data']['response_id'], 'MCP_SERVER_NOT_ENABLED');
    }
}

