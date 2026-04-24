<?php

namespace RESTAPI\Endpoints;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Endpoint;
use RESTAPI\Core\Model;
use RESTAPI\Core\Response;
use RESTAPI\Models\MCP;
use RESTAPI\Responses\MCPResponse;

/**
 * Defines an Endpoint for the Model Context Protocol (MCP) server at /api/v2/mcp.
 *
 * The MCPEndpoint accepts MCP JSON-RPC 2.0 requests and dispatches them through the MCP Model. Tools are
 * dynamically generated from every Endpoint and Model in this API and reuse the same per-Endpoint privileges
 * so MCP clients require the same permissions as REST API clients.
 */
class MCPEndpoint extends Endpoint {
    public Model|MCP $model;

    public function __construct() {
        # Set Endpoint attributes
        $this->url = '/api/v2/mcp';
        $this->model_name = 'MCP';
        $this->request_method_options = ['POST'];
        $this->response_types = ['MCPResponse'];

        # Set help text for this Endpoint
        $this->post_help_text =
            'Dispatch a Model Context Protocol (MCP) JSON-RPC 2.0 request. The MCP server is dynamically generated ' .
            'from this API\'s Endpoints and Models, exposing each REST API operation as an MCP tool. The MCP server ' .
            'is disabled by default and must be enabled via the REST API settings before use.';

        # Construct the parent Endpoint object
        parent::__construct();
    }

    /**
     * A custom handler used to wrap REST API responses into MCP JSON-RPC envelopes. Successful MCP responses
     * are returned as-is; non-200 responses are wrapped into MCP error envelopes so MCP clients always receive
     * a JSON-RPC compliant response.
     * @param Response $response The incoming response object.
     * @return Response The response object to return to the client.
     */
    public function response_handler(Response $response): Response {
        if ($response->code !== 200) {
            return MCPResponse::to_mcp_response($response);
        }

        return new MCPResponse(data: $this->model);
    }
}

