<?php

namespace RESTAPI\MCP;

require_once 'RESTAPI/autoloader.inc';

use Mcp\Schema\Enum\ProtocolVersion;
use Mcp\Schema\Implementation;
use Mcp\Schema\JsonRpc\Error as JsonRpcError;
use Mcp\Schema\JsonRpc\MessageInterface;
use Mcp\Schema\Result\CallToolResult;
use Mcp\Schema\Result\InitializeResult;
use Mcp\Schema\Result\ListToolsResult;
use Mcp\Schema\Result\EmptyResult;
use Mcp\Schema\ServerCapabilities;
use Mcp\Schema\Content\TextContent;
use Throwable;
use RESTAPI\Core\Auth;
use RESTAPI\Core\Response;
use RESTAPI\Schemas\MCPSchema;

/**
 * Defines the MCP server that handles JSON-RPC 2.0 requests for the /api/v2/mcp endpoint.
 *
 * The Server is intentionally stateless and request/response oriented, mirroring the way the rest of the
 * pfSense REST API behaves. The Server uses the official mcp/sdk schema/JSON-RPC classes to ensure that the
 * messages it produces conform to the Model Context Protocol specification, while it dispatches requests
 * directly through this class so it does not depend on the SDK's session/transport machinery.
 */
class Server {
    /**
     * @var MCPSchema The MCPSchema used to enumerate the available MCP tools.
     */
    public MCPSchema $schema;

    /**
     * @var Implementation The MCP server identification information returned during the initialize handshake.
     */
    public Implementation $server_info;

    /**
     * @var ServerCapabilities The capabilities advertised by this MCP server during the initialize handshake.
     */
    public ServerCapabilities $capabilities;

    /**
     * Constructs a new MCP Server.
     * @param MCPSchema|null $schema An optional MCPSchema object to use. If null, a new one will be created.
     */
    public function __construct(?MCPSchema $schema = null) {
        $this->schema = $schema ?? new MCPSchema();
        $this->server_info = new Implementation(
            name: 'pfSense-pkg-RESTAPI MCP Server',
            version: $this->get_api_version(),
            description: 'A dynamic Model Context Protocol server for the pfSense REST API. ' .
                'All REST API operations are exposed as MCP tools.',
        );
        $this->capabilities = new ServerCapabilities(
            tools: true,
            toolsListChanged: false,
            resources: false,
            resourcesSubscribe: false,
            resourcesListChanged: false,
            prompts: false,
            promptsListChanged: false,
            logging: false,
            completions: false,
        );
    }

    /**
     * Dispatches a JSON-RPC 2.0 request to the appropriate MCP method handler.
     * @param array $request The decoded JSON-RPC request.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The JSON-RPC envelope (result or error) to send back to the client.
     */
    public function handle(array $request, Auth $auth): array {
        # Notifications (requests without an `id`) receive no response per the JSON-RPC spec.
        $id = $request['id'] ?? null;
        $is_notification = !array_key_exists('id', $request);

        # Validate the JSON-RPC envelope.
        if (($request['jsonrpc'] ?? null) !== MessageInterface::JSONRPC_VERSION) {
            return $is_notification ? [] : $this->error_envelope(
                JsonRpcError::forInvalidRequest('Invalid or missing "jsonrpc" version.', $id ?? ''),
            );
        }
        if (!isset($request['method']) or !is_string($request['method'])) {
            return $is_notification ? [] : $this->error_envelope(
                JsonRpcError::forInvalidRequest('Invalid or missing "method".', $id ?? ''),
            );
        }

        $method = $request['method'];
        $params = $request['params'] ?? [];

        # Notifications carry no response. Acknowledge by returning an empty array.
        if ($is_notification) {
            return [];
        }

        try {
            $result = match ($method) {
                'initialize' => $this->handle_initialize($params),
                'ping' => new EmptyResult(),
                'tools/list' => $this->handle_tools_list($params),
                'tools/call' => $this->handle_tools_call($params, $auth),
                default => throw new MethodNotFoundException("Unknown MCP method '$method'."),
            };
        } catch (MethodNotFoundException $e) {
            return $this->error_envelope(JsonRpcError::forMethodNotFound($e->getMessage(), $id));
        } catch (InvalidParamsException $e) {
            return $this->error_envelope(JsonRpcError::forInvalidParams($e->getMessage(), $id));
        } catch (Response $e) {
            # Surface RESTAPI Response exceptions (e.g. ForbiddenError) as JSON-RPC errors.
            return $this->error_envelope(
                new JsonRpcError(
                    id: $id,
                    code: $e->code === 403 ? JsonRpcError::INVALID_REQUEST : JsonRpcError::SERVER_ERROR,
                    message: $e->message,
                    data: ['response_id' => $e->response_id, 'http_code' => $e->code],
                ),
            );
        } catch (Throwable $e) {
            return $this->error_envelope(JsonRpcError::forInternalError($e->getMessage(), $id));
        }

        return [
            'jsonrpc' => MessageInterface::JSONRPC_VERSION,
            'id' => $id,
            'result' => $result,
        ];
    }

    /**
     * Handles the MCP `initialize` request and returns the server's capabilities.
     * @param array $params The parameters from the JSON-RPC request.
     * @return InitializeResult The InitializeResult that describes this server.
     */
    public function handle_initialize(array $params): InitializeResult {
        # Use the protocol version requested by the client when supported, otherwise fall back to the latest one.
        $requested = isset($params['protocolVersion']) ? ProtocolVersion::tryFrom($params['protocolVersion']) : null;
        $protocol_version = $requested ?? ProtocolVersion::V2025_06_18;

        return new InitializeResult(
            capabilities: $this->capabilities,
            serverInfo: $this->server_info,
            instructions: 'This MCP server exposes the pfSense REST API as dynamically generated tools. Each tool ' .
                'corresponds to a REST API operation and uses the same authentication, authorization and ' .
                'privileges as the REST API. Use tools/list to discover the available tools.',
            protocolVersion: $protocol_version,
        );
    }

    /**
     * Handles the MCP `tools/list` request and returns every tool generated by the MCPSchema.
     * @param array $params The parameters from the JSON-RPC request (currently unused).
     * @return ListToolsResult The ListToolsResult containing all generated tools.
     */
    public function handle_tools_list(array $params): ListToolsResult {
        return new ListToolsResult(tools: array_values($this->schema->get_tools()));
    }

    /**
     * Handles the MCP `tools/call` request by routing to the appropriate Resolver method.
     * @param array $params The parameters from the JSON-RPC request, must contain a `name` and may contain `arguments`.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return CallToolResult The CallToolResult containing the tool result or an error.
     * @throws InvalidParamsException If `name` is missing or unknown.
     */
    public function handle_tools_call(array $params, Auth $auth): CallToolResult {
        if (!isset($params['name']) or !is_string($params['name'])) {
            throw new InvalidParamsException('Missing or invalid "name" in tools/call params.');
        }

        $name = $params['name'];
        $arguments = $params['arguments'] ?? [];
        if (!is_array($arguments)) {
            throw new InvalidParamsException('"arguments" must be an object/array.');
        }

        # Look up the tool handler by name. The handler array is keyed by tool name and contains a Resolver method.
        $handlers = $this->schema->get_tool_handlers();
        if (!array_key_exists($name, $handlers)) {
            throw new InvalidParamsException("Unknown MCP tool '$name'.");
        }

        $handler = $handlers[$name];
        $resolver = $handler['resolver'];
        $action = $handler['action'];

        try {
            $data = $resolver->$action($arguments, $auth);
            return CallToolResult::success([new TextContent(text: $data)]);
        } catch (Response $e) {
            # Tool-level errors are returned inside the CallToolResult per the MCP spec so the client can show
            # the error to the model. Protocol-level errors are still surfaced via JSON-RPC errors above.
            return CallToolResult::error([
                new TextContent(
                    text: [
                        'message' => $e->message,
                        'response_id' => $e->response_id,
                        'http_code' => $e->code,
                    ],
                ),
            ]);
        }
    }

    /**
     * Builds a JSON-RPC error envelope from a given Mcp\Schema\JsonRpc\Error.
     * @param JsonRpcError $error The JsonRpcError object describing the error.
     * @return array The serialized JSON-RPC error envelope.
     */
    public function error_envelope(JsonRpcError $error): array {
        return $error->jsonSerialize();
    }

    /**
     * Returns the current pfSense REST API version, used as the server version in the initialize handshake.
     * @return string The current REST API version string, or 'dev' when unavailable.
     */
    private function get_api_version(): string {
        try {
            $version = new \RESTAPI\Models\RESTAPIVersion();
            return (string) ($version->current_version->value ?? 'dev');
        } catch (Throwable) {
            return 'dev';
        }
    }
}



