<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Field;
use RESTAPI\Core\Model;
use RESTAPI\Core\ModelCache;
use RESTAPI\Core\ModelSet;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Field  that adds validation and representation for Fields that relate to a different Model object field.
 * For example, a ForeignModelField can be used to relate a static route to its parent Gateway model object.
 */
class ForeignModelField extends Field {
    const MODELS_NAMESPACE = 'RESTAPI\\Models\\';
    public array $models = [];
    /**
     * Defines the ForeignModelField object and sets its options.
     * @param string|array $model_name The name(s) of the foreign Model class(es) this field relates to. This should be
     * the name(s) of existing Model class(es) in the \RESTAPI\Models namespace, but should not include the \RESTAPI\Models
     * namespace prefix. If multiple Model classes are specified as an array, they must all contain the $model_field
     * and $model_field_internal values specified. Only use multiple Model classes if the Model objects that are
     * similar in structure and can be used interchangeably.
     * @param string $model_field The field on the assigned $model_name class that this field's value relates to. For
     * example, if this Fields value must be an existing Gateway model object's `name`, you would set $model_name to
     * `Gateway` and $model_field to `name`. Note: If this field is not a unique field, related models will be matched
     * on a first-match basis.
     * @param string $model_field_internal In the event that you want to store a different field value internally,
     * specify that field name here. Leave empty to use the $model_field for both the internal and representation values.
     * For example, if this field's value must be set in config as an existing Gateway model object's `name`, but you
     * want clients to reference the Gateway model object by its `id` instead, you would set $model_field to `id` and
     * $model_field_internal to `name`.
     * @param array $model_query Enter query parameters to limit which of the $model's existing objects can be
     * referenced by this field. For example, say this field's value must be set to an existing FirewallAlias object's
     * `name`, but you only want to allow `port` type aliases; you would set $model_name to `FirewallAlias`, $model_field to
     * `name`, and $model_query to `["type" => "port"]`. Defaults to all existing $model objects.
     * @param array $parent_model_query When the assigned $model_name class has a parent Model, use this variable to
     * limit the scope of which parent Model objects can have their child Models included. By default, the child
     * Models of ALL parent Models are included.
     * @param array $allowed_keywords Specific values that should always be allowed, even if they don't relate to the
     * assigned Model or its fields.
     * @param bool $required If `true`, this field is required to have a value at all times.
     * @param bool $unique If `true`, this field must be unique from all other parent model objects. Enabling this
     * option requires the Model $context to be set AND the Model $context must have a `config_path` set.
     * @param bool $skip_unique_validation If `true`, skip uniqueness enforcement while retaining the `unique` metadata.
     * @param mixed|null $default Assign a default string value to assign this Field if no value is present.
     * @param string $default_callable Defines a callable method that should be called to populate the default value
     * for this field. It is strongly encouraged to use a default callable when the default is variable and may change
     * dynamically.
     * @param array $choices An array of value choices this Field can be assigned. This can either be an indexed array
     * of the exact choice values, or an associative array where the array key is the exact choice value and the array
     * value is a verbose name for the choice. Verbose choice name are used by ModelForms when generating web pages
     * for a given Model.
     * @param string $choices_callable Assign a callable method from this Field object OR the parent Model context to
     * execute to populate choices for this field. This callable must be a method assigned on this Field object OR the
     * parent Model object that returns an array of valid choices in the same format as $choices. This is helpful when
     * choices are dynamic and must be populated at runtime instead of pre-determined sets of values.
     * @param bool $allow_empty If `true`, empty strings will be allowed by this field.
     * @param bool $allow_null If `true`, null values will be allowed by this field.
     * @param bool $editable Set to `false` to prevent this field's value from being changed after its initial creation.
     * @param bool $read_only If `true`, this field can only read its value and cannot write its value to config.
     * @param bool $write_only Set the `true` to make this field write-only. This will prevent the field's current value
     * from being displayed in the representation data. This is ideal for potentially sensitive Fields like passwords,
     * keys, and hashes.
     * @param bool $representation_only Set to `true` to make this field only present in its representation form. This
     * effectively prevents the Field from being converted to an internal value which is saved to the pfSense config.
     * This should only be used for Fields that do not relate directly to a configuration value.
     * @param bool $many If `true`, the value must be an array of many strings.
     * @param int $many_minimum When $many is set to `true`, this sets the minimum number of array entries required.
     * @param int $many_maximum When $many is set to `true`, this sets the maximum number of array entries allowed.
     * @param string|null $delimiter Assigns the string delimiter to use when writing array values to config.
     *  Use `null` if this field is stored as an actual array in config. This is only available if $many is set to
     *  `true`. Defaults to `,` to store as comma-separated string.
     * @param string $verbose_name The detailed name for this Field. This name will be used in non-programmatic areas
     * like web pages and help text. This Field will default to property name assigned to the parent Model with
     * underscores converted to spaces.
     * @param string $verbose_name_plural The plural form of $verbose_name. This defaults to $verbose_name with `s`
     * suffixed or `es` suffixes to strings already ending with `s`.
     * @param string $internal_name Assign a different field name to use when referring to the internal field as it's
     * stored in the pfSense configuration.
     * @param string $internal_namespace Sets the namespace this field belongs to internally. This can be used to nest
     * the Fields internal value under a specific namespace as an associative array. This only applies to the internal
     * value, not the representation value.
     * @param array $referenced_by An array that specifies other Models and Field's that reference this Field's parent
     * Model using this Field's value. This will prevent the parent Model object from being deleted while it is actively
     * referenced by another Model object. The array key must be the name of the Model class that references this Field,
     * and the value must be a Field within that Model. The framework will automatically search for any existing Model
     * objects that have the referenced Field assigned a value that matches this Field's value.
     * @param array $conditions An array of conditions the field must meet to be included. This allows you to specify
     * conditions of other Fields within the parent Model context. For example, if the parent Model context has two
     * Fields, one field named `type` and the other being this field; and you only want this field to be included if
     * `type` is equal to `type1`, you could assign ["type" => "type1"] to this parameter.
     * @param array $validators An array of Validator objects to run against this field.
     * @param string $help_text Set a description for this field. This description will be used in API documentation.
     */
    public function __construct(
        public string|array $model_name,
        public string $model_field,
        public string $model_field_internal = '',
        public array $model_query = [],
        public array $parent_model_query = [],
        public array $allowed_keywords = [],
        bool $required = false,
        bool $unique = false,
        bool $skip_unique_validation = false,
        mixed $default = null,
        string $default_callable = '',
        array $choices = [],
        string $choices_callable = '',
        bool $allow_empty = false,
        bool $allow_null = false,
        bool $editable = true,
        bool $read_only = false,
        bool $write_only = false,
        bool $representation_only = false,
        bool $many = false,
        int $many_minimum = 0,
        int $many_maximum = Field::MANY_MAXIMUM,
        string|null $delimiter = ',',
        string $verbose_name = '',
        string $verbose_name_plural = '',
        string $internal_name = '',
        string $internal_namespace = '',
        array $referenced_by = [],
        array $conditions = [],
        array $validators = [],
        string $help_text = '',
    ) {
        # Assign properties unique to this Field object
        $this->model_field_internal = $model_field_internal ?: $model_field;
        $this->model_name = is_array($model_name) ? $model_name : [$model_name];

        # Create reference Model objects for each assigned $model_name
        foreach ($this->model_name as $model_name) {
            # Ensure the properties assigned are allowed and that the assigned $model_name can be constructed
            $this->check_construct(self::MODELS_NAMESPACE . $model_name);
        }

        # Construct the parent Field object
        parent::__construct(
            type: $model_field === 'id' ? 'integer' : $this->models[0]->$model_field->type,
            required: $required,
            unique: $unique,
            skip_unique_validation: $skip_unique_validation,
            default: $default,
            default_callable: $default_callable,
            choices: $choices,
            choices_callable: $choices_callable,
            allow_empty: $allow_empty,
            allow_null: $allow_null,
            editable: $editable,
            read_only: $read_only,
            write_only: $write_only,
            representation_only: $representation_only,
            many: $many,
            many_minimum: $many_minimum,
            many_maximum: $many_maximum,
            delimiter: $delimiter,
            verbose_name: $verbose_name,
            verbose_name_plural: $verbose_name_plural,
            internal_name: $internal_name,
            internal_namespace: $internal_namespace,
            referenced_by: $referenced_by,
            conditions: $conditions,
            validators: $validators,
            help_text: $help_text,
        );
    }

    /**
     * Checks that the object to be constructed has no conflicts.
     * @param string $model_name The foreign Model class's FQN.
     */
    private function check_construct(string $model_name): void {
        # Ensure the assigned $model_name is an existing Model class
        if (!class_exists($model_name)) {
            throw new ServerError(
                message: "ForeignModelField's `model_name` property must be an existing Model class FQN, " .
                    "received `$model_name`.",
                response_id: 'FOREIGN_MODEL_FIELD_WITH_UNKNOWN_MODEL_NAME',
            );
        }

        # Create a new object using the assigned foreign Model class
        $model = new $model_name(skip_init: true);
        $this->models[] = $model;

        # Ensure the foreign Model class is a `many` Model
        if (!$model->many) {
            throw new ServerError(
                message: 'ForeignModelField `model_name` must be a Model class with `many` enabled.',
                response_id: 'FOREIGN_MODEL_FIELD_ASSIGNED_NON_MANY_MODEL',
            );
        }

        # Ensure the `model_field` exists on this Model class
        if (!in_array($this->model_field, $model->get_fields()) and $this->model_field !== 'id') {
            throw new ServerError(
                message: "ForeignModelField's `model_field` does not exist in class `$model_name`.",
                response_id: 'FOREIGN_MODEL_FIELD_WITH_UNKNOWN_MODEL_FIELD',
            );
        }

        # Ensure the `model_field_internal` exists on this Model class
        if (!in_array($this->model_field_internal, $model->get_fields()) and $this->model_field_internal !== 'id') {
            throw new ServerError(
                message: "ForeignModelField's `model_field_internal` does not exist in class `$model_name`.",
                response_id: 'FOREIGN_MODEL_FIELD_WITH_UNKNOWN_MODEL_FIELD_INTERNAL',
            );
        }
    }

    /**
     * Ensures that the foreign models this field relates to are indexed by their foreign model fields and
     * internal fields for faster querying.
     */
    protected function _index_foreign_models(): void {
        # Index all Models from the referenced Model classes
        foreach ($this->models as $model) {
            $model_name = $model->get_class_fqn();

            # Index the Model objects by the `model_field`
            ModelCache::get_instance()::index_modelset_by_field(
                model_class: $model_name,
                index_field: $this->model_field,
            );

            # Index the Model objects by the `model_field_internal` if different
            if ($this->model_field_internal !== $this->model_field) {
                ModelCache::get_instance()::index_modelset_by_field(
                    model_class: $model_name,
                    index_field: $this->model_field_internal,
                );
            }
        }
    }

    /**
     * Obtains the referenced Model object from the model cache index by its internal value.
     */
    public function get_referenced_model_by_internal_value(mixed $internal_value): Model|null {
        # First, obtain the model cache and ensure foreign models are indexed
        $model_cache = ModelCache::get_instance();
        $this->_index_foreign_models();

        # Then, attempt to obtain the Model object from the index
        foreach ($this->models as $model) {
            # Obtain the Model class FQN
            $model_name = $model->get_class_fqn();

            # Check if a Model object is indexed by this internal value
            if (
                $model_cache::has_model(
                    $model_name,
                    index_field: $this->model_field_internal,
                    index_value: $internal_value,
                )
            ) {
                return $model_cache::fetch_model(
                    model_class: $model_name,
                    index_field: $this->model_field_internal,
                    index_value: $internal_value,
                );
            }
        }

        # Otherwise, return null
        return null;
    }

    /**
     * Converts the field value to its representation form from it's internal pfSense configuration value.
     * @param mixed $internal_value The internal value from the pfSense configuration.
     * @return mixed The field value in its representation form.
     */
    protected function _from_internal(mixed $internal_value): mixed {
        # Fetch the related Model object using the internal value
        $related_model = $this->get_referenced_model_by_internal_value($internal_value);

        # If the related Model object exists, return the existing `model_field` value.
        if ($related_model !== null) {
            return $related_model->{$this->model_field}->value;
        }

        # As a failsafe, return the existing internal value if we could not find the related object
        return $internal_value;
    }

    /**
     * Converts the represented value into the internal pfSense value.
     * @param mixed $representation_value The value to convert into it's internal form.
     * @return array|string|null The internal value(s) suitable for writing to the pfSense configuration.
     */
    protected function _to_internal(mixed $representation_value): array|string|null {
        # Obtain Model objects that matches this field's criteria
        $query_modelset = $this->__get_matches($this->model_field, $representation_value);

        # If the model object exists, return the existing `model_field_internal` value.
        if ($query_modelset->exists()) {
            return $query_modelset->first()->{$this->model_field_internal}->value;
        }

        # As a failsafe, return the existing representation value if we could not find the related object
        return parent::_to_internal($representation_value);
    }

    /**
     * Checks that the value assigned to this Field corresponds to an existing foreign model object during validation.
     * @param mixed $value The value being validated. In the event that this is a `many` field, this method will
     * receive each value of the array individually, not the array value itself.
     */
    public function validate_extra(mixed $value): void {
        # Accept this value matches an allowed keyword exactly
        if (in_array($value, $this->allowed_keywords)) {
            return;
        }

        # Obtain Models that match this Field's criteria
        $query_modelset = $this->__get_matches($this->model_field, $value);

        # Format a string containing the in scope class names
        $model_names = implode(' or ', array_map(fn($model) => $model->verbose_name, $this->models));

        # If the model object exists, return the existing `model_field_internal` value.
        if (!$query_modelset->exists()) {
            throw new NotFoundError(
                message: "Field `$this->name` could not locate `$model_names` object with " .
                    "`$this->model_field` set to `$value`",
                response_id: 'FOREIGN_MODEL_FIELD_VALUE_NOT_FOUND',
            );
        }
    }

    /**
     * Obtains the ModelSet of Model objects that are in-scope for this field using the $model_query and
     * $parent_model_query properties.
     * @return array An array of ModelSet objects containing all in-scope Models for this ForeignModelField.
     */
    public function get_in_scope_models(): array {
        # Variables
        $in_scope_modelsets = [];

        # Get in scope Models from all assigned $model_name classes
        foreach ($this->models as $model) {
            # For Models with parent Mdoels assigned, ensure only child Models from in-scope parent Models are included
            if ($model->parent_model_class) {
                $parent_model_class = $model->get_parent_model();
                $parent_model = new $parent_model_class();
                $parent_models = $parent_model->query($this->parent_model_query);
                $in_scope_parent_model_ids = array_map(fn($parent) => $parent->id, $parent_models->model_objects);
                $models = $model->query(parent_id__in: $in_scope_parent_model_ids);
            }
            # Otherwise, just use all of this $model's current objects
            else {
                $models = $model->read_all();
            }

            # Filter out Models that do not meet the assigned $model_query criteria
            $models = $models->query($this->model_query);
            $in_scope_modelsets[$model->get_class_fqn()] = $models;
        }

        # Query for the Model object this value relates to.
        return $in_scope_modelsets;
    }

    /**
     * Obtains a ModelSet of the Model(s) that match this field's criteria.
     * @param string $field_name The name of the field used to check for matching values. This is typically set to the
     * same value as $this->field_name.
     * @param mixed $field_value The value of the $field_name that indicates there is a match. This is typically set
     * to the same value as $this->value.
     */
    private function __get_matches(string $field_name, mixed $field_value): ModelSet {
        self::_index_foreign_models();

        # Loop through all in cope modelsets to find matches
        foreach ($this->get_in_scope_models() as $model_class => $in_scope_modelset) {
            # Move to the next model class if we found no matches in the index
            if (!ModelCache::get_instance()::has_model($model_class, $field_name, $field_value)) {
                continue;
            }

            # Obtain the matched model
            $matched_model = ModelCache::get_instance()::fetch_model($model_class, $field_name, $field_value);

            # Move to the next model class if the matched model we found was not in scope
            if ($this->model_query and !$in_scope_modelset->query(id: $matched_model->id)->exists()) {
                continue;
            }

            # Otherwise, the matched model is valid. Return it within a ModelSet
            return new ModelSet(model_objects: [$matched_model]);
        }

        # Return an empty ModelSet if no matches were found
        return new ModelSet();
    }

    /**
     * Obtains the Model object associated with this field's current value. This is only applicable when this is not
     * a $many enabled field.
     * @returns Model|null Returns the Model object associated with this Field's current value.
     */
    public function get_related_model(): Model|null {
        # Get the Model objects that match this field's criteria
        $query_modelset = $this->__get_matches($this->model_field, $this->value);

        # Return the related model object if it exists
        if ($query_modelset->exists()) {
            return $query_modelset->first();
        }

        # Otherwise return null
        return null;
    }

    /**
     * Obtains the Model objects associated with this field's current values. This is only applicable when this is
     * a $many enabled field.
     * @returns ModelSet Returns a ModelSet containing all Models associated with this Field's current values.
     */
    public function get_related_models(): ModelSet {
        # Create a ModelSet we can use to store matching objects
        $modelset = new ModelSet();

        # Loop through each current value and query for model objects that match them
        foreach ($this->value as $value) {
            # Obtain Model objects that match this value
            $query_modelset = $this->__get_matches($this->model_field, $value);

            # Only add the Model object if it exists
            if ($query_modelset->exists()) {
                $modelset->model_objects[] = $query_modelset->first();
            }
        }

        return $modelset;
    }
}
