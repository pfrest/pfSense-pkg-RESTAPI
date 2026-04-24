<?php

namespace RESTAPI\Responses;

require_once 'RESTAPI/autoloader.inc';

use Mcp\Schema\JsonRpc\Error as JsonRpcError;
use Mcp\Schema\JsonRpc\MessageInterface;
use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Core\ResourceLinkSet;
use RESTAPI\Core\Response;
use RESTAPI\Models\MCP;

/**
 * Response class that represents an MCP JSON-RPC 2.0 response. Like the GraphQL response, this response does not
 * itself indicate success or failure: every MCP response returns HTTP 200 and the JSON-RPC envelope determines
 * whether an error occurred. The MCPResponse uses the official mcp/sdk JSON-RPC error class to ensure that error
 * envelopes conform to the MCP specification.
 */
class MCPResponse extends Response {
    /**
     * @var int $code The HTTP status code. MCP responses always return 200 because errors are encoded into the
     * JSON-RPC envelope.
     */
    public $code = 200;

    /**
     * @var string $help_text Description of this Response used in the OpenAPI documentation.
     */
    public string $help_text =
        'The MCP server processed the JSON-RPC request. All MCP responses return HTTP 200 regardless of whether ' .
        'the request succeeded; consult the JSON-RPC envelope to determine whether the request resulted in an ' .
        'error.';

    /**
     * Constructs a new MCPResponse object.
     * @param mixed $message The verbose message to include in the Response. (unused)
     * @param string $response_id A unique response ID. (unused)
     * @param Model|ModelSet $data The MCP Model object containing the JSON-RPC envelope to return.
     * @param ResourceLinkSet $_links A ResourceLinkSet for this response. (unused)
     */
    public function __construct(
        public $message = '',
        public string $response_id = '',
        public Model|ModelSet $data = new MCP(),
        public ResourceLinkSet $_links = new ResourceLinkSet(),
    ) {
        parent::__construct($message, $response_id, $data, $_links);
    }

    /**
     * Converts this MCPResponse object into an array representation of its contents. The array is the raw
     * JSON-RPC envelope produced by the MCP Server.
     * @return array The JSON-RPC envelope to return to the client.
     */
    public function to_representation(): array {
        if ($this->data instanceof MCP and is_array($this->data->result)) {
            return $this->data->result;
        }
        return [];
    }

    /**
     * Converts a standard Response object into an MCPResponse. This is used by the MCPEndpoint to wrap REST API
     * errors (e.g. authentication failures) into a JSON-RPC error envelope so MCP clients receive a consistent
     * response format regardless of where the failure originated.
     * @param Response $response The Response object to convert.
     * @return MCPResponse The wrapped MCPResponse.
     */
    public static function to_mcp_response(Response $response): MCPResponse {
        $mcp = new MCP();

        if ($response->code === 200) {
            # Successful responses are echoed through the JSON-RPC envelope.
            $mcp->result = [
                'jsonrpc' => MessageInterface::JSONRPC_VERSION,
                'id' => null,
                'result' => $response->data instanceof Model ? $response->data->to_representation() : [],
            ];
        } else {
            # Map common HTTP error codes to JSON-RPC error codes per the MCP specification.
            $code = match (true) {
                $response->code === 401, $response->code === 403 => JsonRpcError::INVALID_REQUEST,
                $response->code === 404 => JsonRpcError::METHOD_NOT_FOUND,
                $response->code >= 400 and $response->code < 500 => JsonRpcError::INVALID_PARAMS,
                default => JsonRpcError::SERVER_ERROR,
            };
            $error = new JsonRpcError(
                id: '',
                code: $code,
                message: $response->message,
                data: ['response_id' => $response->response_id, 'http_code' => $response->code],
            );
            $mcp->result = $error->jsonSerialize();
        }

        return new MCPResponse(data: $mcp);
    }

    /**
     * Returns the OpenAPI schema describing the JSON-RPC envelope produced by the MCP server.
     */
    public function to_openapi_schema(): array {
        return [
            'type' => 'object',
            'properties' => [
                'jsonrpc' => [
                    'description' => 'The JSON-RPC protocol version. Always "2.0".',
                    'type' => 'string',
                ],
                'id' => [
                    'description' => 'The request ID echoed from the original JSON-RPC request.',
                    'type' => ['string', 'integer', 'null'],
                ],
                'result' => [
                    'description' => 'The MCP method result. Present on successful responses.',
                    'type' => 'object',
                ],
                'error' => [
                    'description' => 'The MCP error object. Present on failed responses.',
                    'type' => 'object',
                    'properties' => [
                        'code' => ['description' => 'The JSON-RPC error code.', 'type' => 'integer'],
                        'message' => ['description' => 'A short description of the error.', 'type' => 'string'],
                        'data' => [
                            'description' => 'Additional information about the error including the originating ' .
                                'pfSense REST API response_id and HTTP code.',
                            'type' => 'object',
                        ],
                    ],
                ],
            ],
        ];
    }
}

