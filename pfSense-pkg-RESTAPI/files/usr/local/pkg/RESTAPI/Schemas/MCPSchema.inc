<?php

namespace RESTAPI\Schemas;

require_once 'RESTAPI/autoloader.inc';

use Mcp\Schema\Tool;
use RESTAPI\Core\Endpoint;
use RESTAPI\Core\Field;
use RESTAPI\Core\Model;
use RESTAPI\Core\Schema;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\MCP\Resolver;
use function RESTAPI\Core\Tools\get_classes_from_namespace;
use function RESTAPI\Core\Tools\to_upper_camel_case;

/**
 * Defines a Schema that represents the pfSense REST API as a set of MCP tools using the official mcp/sdk Tool
 * schema class. This class is responsible for dynamically discovering every Endpoint and Model in this API and
 * producing one MCP tool per supported operation. Tool names mirror the GraphQL operation naming so that
 * consumers familiar with the GraphQL schema can use the same identifiers via MCP.
 */
class MCPSchema extends Schema {
    /**
     * @var string $file_name The file name for this schema when written to disk.
     */
    public string $file_name = 'mcp.json';

    /**
     * @var array<string, Tool> $tools An associative array of MCP Tool objects keyed by tool name.
     */
    public array $tools = [];

    /**
     * @var array<string, array{resolver: Resolver, action: string, endpoint: Endpoint}> $tool_handlers
     * A handler map keyed by tool name. Each entry contains the Resolver, the resolver method to invoke and the
     * Endpoint the tool was generated from.
     */
    public array $tool_handlers = [];

    /**
     * Constructs a new MCPSchema and eagerly builds the tool catalog.
     */
    public function __construct() {
        $this->build_tools();
        parent::__construct();
    }

    /**
     * Returns every MCP Tool object generated from this API's Endpoints.
     * @return array<string, Tool> The MCP Tool objects keyed by tool name.
     */
    public function get_tools(): array {
        return $this->tools;
    }

    /**
     * Returns the handler map used to dispatch MCP `tools/call` requests to the correct Resolver method.
     * @return array<string, array{resolver: Resolver, action: string, endpoint: Endpoint}> The handler map.
     */
    public function get_tool_handlers(): array {
        return $this->tool_handlers;
    }

    /**
     * @inheritDoc
     */
    public function get_schema_str(): string {
        return json_encode(
            ['tools' => array_values($this->tools)],
            JSON_PRETTY_PRINT | JSON_UNESCAPED_SLASHES,
        );
    }

    /**
     * Builds the catalog of MCP Tool objects by walking every Endpoint in the RESTAPI\Endpoints namespace.
     * The MCPEndpoint itself is intentionally excluded so the MCP server cannot expose itself as a tool.
     */
    public function build_tools(): void {
        $this->tools = [];
        $this->tool_handlers = [];

        foreach (get_classes_from_namespace('\\RESTAPI\\Endpoints\\') as $endpoint_class) {
            # Avoid recursive exposure of the MCP endpoint itself.
            if ($endpoint_class === '\\RESTAPI\\Endpoints\\MCPEndpoint') {
                continue;
            }
            # The GraphQL endpoint is also excluded since it is itself a tool dispatcher.
            if ($endpoint_class === '\\RESTAPI\\Endpoints\\GraphQLEndpoint') {
                continue;
            }

            $endpoint = new $endpoint_class();
            $this->add_tools_for_endpoint($endpoint);
        }
    }

    /**
     * Adds every MCP Tool that corresponds to a supported operation on the given Endpoint. The supported
     * operations mirror the REST API + GraphQL operations: read/query for GET, create for POST, update for
     * PATCH, delete for DELETE, and replaceAll/deleteMany/deleteAll for the `many` variants.
     * @param Endpoint $endpoint The Endpoint to generate MCP tools from.
     */
    public function add_tools_for_endpoint(Endpoint $endpoint): void {
        $resolver = new Resolver($endpoint->model);

        # GET-based operations
        if (in_array('GET', $endpoint->request_method_options)) {
            $endpoint->many ? $this->add_query_tool($endpoint, $resolver) : $this->add_read_tool($endpoint, $resolver);
        }

        # Non-many mutations
        if (!$endpoint->many and in_array('POST', $endpoint->request_method_options)) {
            $this->add_create_tool($endpoint, $resolver);
        }
        if (!$endpoint->many and in_array('PATCH', $endpoint->request_method_options)) {
            $this->add_update_tool($endpoint, $resolver);
        }
        if (!$endpoint->many and in_array('DELETE', $endpoint->request_method_options)) {
            $this->add_delete_tool($endpoint, $resolver);
        }

        # Many-only mutations
        if ($endpoint->many and in_array('PUT', $endpoint->request_method_options)) {
            $this->add_replace_all_tool($endpoint, $resolver);
        }
        if ($endpoint->many and in_array('DELETE', $endpoint->request_method_options)) {
            $this->add_delete_many_tool($endpoint, $resolver);
            $this->add_delete_all_tool($endpoint, $resolver);
        }
    }

    /**
     * Builds the MCP operation name that corresponds with a given Endpoint and operation prefix. The naming
     * convention is identical to the GraphQL operation naming so MCP and GraphQL share the same vocabulary.
     * @param string $operation The operation prefix (e.g. read, query, create).
     * @param Endpoint $endpoint The Endpoint to derive the operation name from.
     * @return string The MCP tool name (e.g. readFirewallAlias, queryFirewallAliases).
     */
    public function endpoint_to_operation_name(string $operation, Endpoint $endpoint): string {
        $operation = lcfirst($operation);
        $name = str_replace('/api/v2/', '', $endpoint->url);
        $name = str_replace('/', ' ', $name);
        return $operation . to_upper_camel_case($name);
    }

    /**
     * Registers a single MCP Tool and its handler in the schema.
     * @param Endpoint $endpoint The Endpoint the tool is generated from.
     * @param Resolver $resolver The Resolver to use when dispatching the tool call.
     * @param string $operation The operation prefix used to name the tool.
     * @param string $action The Resolver method to invoke when the tool is called.
     * @param array $properties The JSON Schema `properties` map for the tool's input.
     * @param array $required The required property names for the tool's input.
     * @param string $description The human-readable description for the tool.
     */
    private function register_tool(
        Endpoint $endpoint,
        Resolver $resolver,
        string $operation,
        string $action,
        array $properties,
        array $required,
        string $description,
    ): void {
        $name = $this->endpoint_to_operation_name(operation: $operation, endpoint: $endpoint);
        $this->tools[$name] = new Tool(
            name: $name,
            inputSchema: [
                'type' => 'object',
                'properties' => empty($properties) ? new \stdClass() : $properties,
                'required' => $required,
            ],
            description: $description,
            annotations: null,
        );
        $this->tool_handlers[$name] = ['resolver' => $resolver, 'action' => $action, 'endpoint' => $endpoint];
    }

    /**
     * Adds a `query{ModelName}` tool that maps to the Resolver::query method.
     */
    private function add_query_tool(Endpoint $endpoint, Resolver $resolver): void {
        $properties = [
            'query_params' => [
                'type' => 'object',
                'description' => 'An object containing the query parameters used to filter the results.',
                'additionalProperties' => true,
                'default' => new \stdClass(),
            ],
            'limit' => ['type' => 'integer', 'description' => 'The maximum number of objects to return.', 'default' => 0],
            'offset' => ['type' => 'integer', 'description' => 'The offset to start returning objects from.', 'default' => 0],
            'reverse' => ['type' => 'boolean', 'description' => 'Reverse the order of returned objects.', 'default' => false],
            'sort_by' => [
                'type' => 'array',
                'description' => 'The fields to sort the returned objects by.',
                'items' => ['type' => 'string'],
                'default' => [],
            ],
            'sort_order' => ['type' => 'integer', 'description' => 'The order to use when sorting.', 'default' => SORT_ASC],
        ];
        $description = "Queries multiple {$endpoint->model->verbose_name} objects. Equivalent to GET {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'query', 'query', $properties, [], $description);
    }

    /**
     * Adds a `read{ModelName}` tool that maps to the Resolver::read method.
     */
    private function add_read_tool(Endpoint $endpoint, Resolver $resolver): void {
        $properties = [];
        $required = [];
        if ($endpoint->model->many) {
            $properties['id'] = $this->id_property($endpoint->model->id_type, 'The ID of the object to read.');
            $required[] = 'id';
        }
        if ($endpoint->model->parent_model_class) {
            $properties['parent_id'] = $this->id_property(
                $endpoint->model->parent_id_type,
                'The parent ID of the object to read.',
            );
            $required[] = 'parent_id';
        }
        $description = "Reads a single {$endpoint->model->verbose_name} object. Equivalent to GET {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'read', 'read', $properties, $required, $description);
    }

    /**
     * Adds a `create{ModelName}` tool that maps to the Resolver::create method.
     */
    private function add_create_tool(Endpoint $endpoint, Resolver $resolver): void {
        $args = $this->model_to_input_properties(
            model: $endpoint->model,
            require_id: false,
            ignore_required: false,
            exclude_fields: ['append', 'delete'],
        );
        $description = "Creates a new {$endpoint->model->verbose_name} object. Equivalent to POST {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'create', 'create', $args['properties'], $args['required'], $description);
    }

    /**
     * Adds an `update{ModelName}` tool that maps to the Resolver::update method.
     */
    private function add_update_tool(Endpoint $endpoint, Resolver $resolver): void {
        $args = $this->model_to_input_properties(
            model: $endpoint->model,
            require_id: $endpoint->model->many,
            ignore_required: true,
        );
        $description = "Updates an existing {$endpoint->model->verbose_name} object. Equivalent to PATCH {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'update', 'update', $args['properties'], $args['required'], $description);
    }

    /**
     * Adds a `delete{ModelName}` tool that maps to the Resolver::delete method.
     */
    private function add_delete_tool(Endpoint $endpoint, Resolver $resolver): void {
        $args = $this->model_to_input_properties(
            model: $endpoint->model,
            require_id: $endpoint->model->many,
            only_id: true,
        );
        $description = "Deletes an existing {$endpoint->model->verbose_name} object. Equivalent to DELETE {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'delete', 'delete', $args['properties'], $args['required'], $description);
    }

    /**
     * Adds a `replaceAll{ModelName}s` tool that maps to the Resolver::replace_all method.
     */
    private function add_replace_all_tool(Endpoint $endpoint, Resolver $resolver): void {
        $properties = [
            'objects' => [
                'type' => 'array',
                'description' => 'The objects to replace all existing objects with.',
                'items' => $this->model_to_object_schema($endpoint->model, ignore_required: true),
            ],
        ];
        $description = "Replaces every {$endpoint->model->verbose_name} object. Equivalent to PUT {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'replaceAll', 'replace_all', $properties, ['objects'], $description);
    }

    /**
     * Adds a `deleteMany{ModelName}s` tool that maps to the Resolver::delete_many method.
     */
    private function add_delete_many_tool(Endpoint $endpoint, Resolver $resolver): void {
        $properties = [
            'query_params' => [
                'type' => 'object',
                'description' => 'An object containing the query parameters used to filter which objects to delete.',
                'additionalProperties' => true,
            ],
            'limit' => ['type' => 'integer', 'description' => 'The maximum number of objects to delete.', 'default' => 0],
            'offset' => ['type' => 'integer', 'description' => 'The offset to start deleting objects from.', 'default' => 0],
        ];
        $description = "Deletes many {$endpoint->model->verbose_name} objects matching a query. Equivalent to DELETE {$endpoint->url}.";
        $this->register_tool($endpoint, $resolver, 'deleteMany', 'delete_many', $properties, ['query_params'], $description);
    }

    /**
     * Adds a `deleteAll{ModelName}s` tool that maps to the Resolver::delete_all method.
     */
    private function add_delete_all_tool(Endpoint $endpoint, Resolver $resolver): void {
        $description = "Deletes every {$endpoint->model->verbose_name} object. Equivalent to DELETE {$endpoint->url}?all=true.";
        $this->register_tool($endpoint, $resolver, 'deleteAll', 'delete_all', [], [], $description);
    }

    /**
     * Builds an MCP/JSON-Schema input properties map for a given Model. Handles ID fields, parent IDs, regular
     * fields and nested model fields.
     * @param Model $model The Model whose Fields should be converted into JSON Schema properties.
     * @param bool $require_id Whether to require an `id` property.
     * @param bool $only_id Whether to include only the ID/parent_id properties.
     * @param bool $ignore_required Whether to ignore the Field::$required attribute.
     * @param array $exclude_fields A list of field names to exclude from the schema.
     * @return array{properties: array<string, mixed>, required: array<int, string>}
     */
    public function model_to_input_properties(
        Model $model,
        bool $require_id = true,
        bool $only_id = false,
        bool $ignore_required = false,
        array $exclude_fields = [],
    ): array {
        $properties = [];
        $required = [];

        if ($require_id) {
            $properties['id'] = $this->id_property($model->id_type, 'The ID of the object.');
            $required[] = 'id';
        }
        if ($model->parent_model_class) {
            $properties['parent_id'] = $this->id_property($model->parent_id_type, 'The parent ID of the object.');
            $required[] = 'parent_id';
        }

        if ($only_id) {
            return ['properties' => $properties, 'required' => $required];
        }

        foreach ($model->get_fields() as $field) {
            if (in_array($field, $exclude_fields)) {
                continue;
            }
            $properties[$field] = $this->field_to_property($model->$field);
            if ($model->$field->required and !$model->$field->conditions and !$ignore_required) {
                $required[] = $field;
            }
        }

        return ['properties' => $properties, 'required' => $required];
    }

    /**
     * Builds a JSON Schema object representation of a Model, used as the `items` schema for replace_all tools.
     * @param Model $model The Model to represent as a JSON Schema object.
     * @param bool $ignore_required Whether to ignore the Field::$required attribute.
     * @return array<string, mixed> The JSON Schema for the Model.
     */
    public function model_to_object_schema(Model $model, bool $ignore_required = false): array {
        $args = $this->model_to_input_properties($model, require_id: false, ignore_required: $ignore_required);
        return [
            'type' => 'object',
            'description' => $model->verbose_name,
            'properties' => empty($args['properties']) ? new \stdClass() : $args['properties'],
            'required' => $args['required'],
        ];
    }

    /**
     * Converts a single Field object into a JSON Schema property.
     * @param Field $field The Field to convert.
     * @return array<string, mixed> The JSON Schema property definition.
     */
    public function field_to_property(Field $field): array {
        # Nested model fields recursively expose the nested Model's schema.
        if ($field instanceof NestedModelField) {
            $nested = new $field->model_class(skip_init: true);
            $object = $this->model_to_object_schema($nested);
            if ($field->many) {
                return [
                    'type' => 'array',
                    'description' => $field->help_text,
                    'items' => $object,
                ];
            }
            $object['description'] = $field->help_text;
            return $object;
        }

        $base = match ($field->type) {
            'boolean' => ['type' => 'boolean'],
            'integer' => ['type' => 'integer'],
            'double' => ['type' => 'number'],
            'string' => ['type' => 'string'],
            'array' => ['type' => 'object', 'additionalProperties' => true],
            default => ['type' => 'string'],
        };

        if ($field->choices) {
            $base = ['type' => $base['type'], 'enum' => array_values($field->choices)];
        }

        if ($field->many) {
            $base = ['type' => 'array', 'items' => $base];
        }

        if ($field->help_text) {
            $base['description'] = $field->help_text;
        }

        return $base;
    }

    /**
     * Builds a JSON Schema property for an ID/parent_id value with the appropriate type.
     * @param string $id_type The Model's id_type ('integer' or 'string').
     * @param string $description A human-readable description for the property.
     * @return array<string, string> The JSON Schema property definition.
     */
    private function id_property(string $id_type, string $description): array {
        return ['type' => $id_type === 'integer' ? 'integer' : 'string', 'description' => $description];
    }
}

