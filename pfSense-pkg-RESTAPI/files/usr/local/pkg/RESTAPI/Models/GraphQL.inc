<?php

namespace RESTAPI\Models;

use GraphQL\Executor\ExecutionResult;
use RESTAPI\Core\Model;
use RESTAPI\Fields\ObjectField;
use RESTAPI\Fields\StringField;
use RESTAPI\Schemas\GraphQLSchema;

/**
 * Defines a model that accept GraphQL queries and mutations and executes them against the GraphQL schema.
 */
class GraphQL extends Model {
    public StringField $query;
    public ObjectField $variables;

    /**
     * @var ExecutionResult|null The result of the GraphQL query or mutation.
     */
    public ?ExecutionResult $result;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_internal';
        $this->verbose_name = 'GraphQL';
        $this->many = false;
        $this->always_apply = true;

        # Set model fields
        $this->query = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: PHP_INT_MAX,
            verbose_name: 'Query',
            help_text: 'The GraphQL query/mutation to execute.',
        );
        $this->variables = new ObjectField(
            default: [],
            allow_empty: true,
            verbose_name: 'Variables',
            help_text: 'The variables to pass to the GraphQL query or mutation. In general, this will be an object ' .
                'containing the variables to pass to the query or mutation.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Pseudo-method that acts as an internal callable. Since this Model has no internal values, this method always
     * returns an empty array.
     * @return array An empty array.
     */
    public function get_internal(): array {
        return [];
    }

    /**
     * Executes the GraphQL query or mutation against the GraphQL schema.
     */
    public function _create(): void {
        # Obtain the GraphQL schema
        $schema = new GraphQLSchema();

        # Execute the GraphQL query or mutation
        $this->result = \GraphQL\GraphQL::executeQuery(
            schema: $schema->get_schema(),
            source: $this->query->value,
            contextValue: ['auth' => $this->client],
            variableValues: $this->variables->value,
        );
    }

    /**
     * Prevents the 'result' property from being included in serialization. This property is not serializable and will
     * cause an error when setting the Model::$initial_data property.
     */
    public function __sleep(): array {
        # Get the properties of this object
        $properties = get_object_vars($this);

        # Remove the 'result' property
        unset($properties['result']);

        # Return the remaining properties to serialize
        return array_keys($properties);
    }
}
