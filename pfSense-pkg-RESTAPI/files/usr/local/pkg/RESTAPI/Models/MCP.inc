<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\ObjectField;
use RESTAPI\Fields\StringField;
use RESTAPI\MCP\Server;
use RESTAPI\Responses\ServiceUnavailableError;

/**
 * Defines a Model that accepts MCP (Model Context Protocol) JSON-RPC 2.0 requests and dispatches them through
 * the MCP Server. The MCP server is dynamically generated from this API's Endpoints and Models, and remains
 * disabled until the `mcp_enabled` field on the RESTAPISettings model is set to true.
 */
class MCP extends Model {
    public StringField $jsonrpc;
    public StringField $method;
    public ObjectField $params;
    public StringField $id;

    /**
     * @var array|null $result The JSON-RPC response envelope produced by the MCP Server. This property is excluded
     * from serialization to avoid issues with the Model::$initial_object property.
     */
    public ?array $result = null;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_internal';
        $this->verbose_name = 'MCP';
        $this->many = false;
        $this->always_apply = true;

        # Set model fields. These represent the JSON-RPC 2.0 envelope sent by an MCP client.
        $this->jsonrpc = new StringField(
            default: '2.0',
            allow_empty: true,
            verbose_name: 'JSON-RPC Version',
            help_text: 'The JSON-RPC protocol version. This must be "2.0" for compliance with the MCP specification.',
        );
        $this->method = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Method',
            help_text: 'The MCP method to invoke. Common values are "initialize", "tools/list" and "tools/call".',
        );
        $this->params = new ObjectField(
            default: [],
            allow_empty: true,
            verbose_name: 'Parameters',
            help_text: 'The parameters object passed to the requested MCP method.',
        );
        $this->id = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Request ID',
            help_text: 'The JSON-RPC request ID. Notifications without an ID will not receive a response.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Pseudo-method that acts as an internal callable. The MCP model has no internal config so this always
     * returns an empty array.
     * @return array An empty array.
     */
    public function get_internal(): array {
        return [];
    }

    /**
     * Dispatches the assigned JSON-RPC request to the MCP Server. When the MCP server is disabled in
     * RESTAPISettings, this method will throw a ServiceUnavailableError.
     * @throws ServiceUnavailableError When the MCP server is not enabled.
     */
    public function _create(): void {
        # Bail early when the MCP server is not enabled.
        $settings = new RESTAPISettings();
        if (!$settings->mcp_enabled->value) {
            throw new ServiceUnavailableError(
                message: 'The MCP server is not enabled. Enable it in the REST API settings before using /api/v2/mcp.',
                response_id: 'MCP_SERVER_NOT_ENABLED',
            );
        }

        # Reconstruct the original JSON-RPC envelope and dispatch it through the MCP Server.
        $request = [
            'jsonrpc' => $this->jsonrpc->value ?: '2.0',
            'method' => $this->method->value,
            'params' => $this->params->value,
        ];
        # Only include the `id` when one was provided so notifications behave correctly.
        if ($this->id->value !== '' and $this->id->value !== null) {
            $id = $this->id->value;
            $request['id'] = is_numeric($id) ? (int) $id : $id;
        }

        $server = new Server();
        $this->result = $server->handle($request, $this->client);
    }

    /**
     * Prevents the `result` property from being included in serialization. This property may contain non-
     * serializable data (e.g. mcp/sdk schema objects) and would otherwise cause errors when the Model
     * sets the `initial_object` property.
     * @return array The list of property names to serialize.
     */
    public function __sleep(): array {
        $properties = get_object_vars($this);
        unset($properties['result']);
        return array_keys($properties);
    }
}

