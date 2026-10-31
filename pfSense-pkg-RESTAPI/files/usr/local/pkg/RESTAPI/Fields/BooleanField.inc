<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;

/**
 * Defines a Field object for storing boolean values in the pfSense configuration.
 */
class BooleanField extends RESTAPI\Core\Field {
    /**
     * @var mixed $indicates_true The internal value that indicates a `true` boolean. This will typically be an empty
     * string, 'on', 'yes', 'enabled', etc.
     */
    public mixed $indicates_true;

    /**
     * @var mixed $indicates_false The internal value that indicates a `false` boolean. This will typically be null,
     * 'off', 'no', 'disabled', etc.
     */
    public mixed $indicates_false;

    /**
     * Defines the StringField object and sets its options.
     * @param bool $required If `true`, this field is required to have a value at all times.
     * @param bool $unique If `true`, this field must be unique from all other parent model objects. Enabling this
     * option requires the Model $context to be set AND the Model $context must have a `config_path` set.
     * @param bool $skip_unique_validation If `true`, skip uniqueness enforcement while retaining the `unique` metadata.
     * @param mixed|null $default Assign a default string value to assign this Field if no value is present.
     * @param string $default_callable Defines a callable method that should be called to populate the default value
     * for this field. It is strongly encouraged to use a default callable when the default is variable and may change
     * dynamically.
     * @param bool $allow_empty If `true`, empty strings will be allowed by this field.
     * @param bool $allow_null If `true`, null values will be allowed by this field.
     * @param bool $editable Set to `false` to prevent this field's value from being changed after its initial creation.
     * @param bool $read_only If `true`, this field can only read its value and cannot write its value to config.
     * $many is enabled.
     * @param bool $write_only Set to `true` to make this field write-only. This will prevent the field's current value
     * from being displayed in the representation data. This is ideal for potentially sensitive Fields like passwords,
     * keys, and hashes.
     * @param bool $representation_only Set to `true` to make this field only present in its representation form. This
     * effectively prevents the Field from being converted to an internal value which is saved to the pfSense config.
     * This should only be used for Fields that do not relate directly to a configuration value.
     * @param mixed $indicates_true The internal string that indicates whether a value is `true`.
     * @param mixed $indicates_false The internal value that indicates whether a value is `false`. Use `null`
     * if the lack of a value for this field indicates `false` internally.
     * @param string $verbose_name The detailed name for this Field. This name will be used in non-programmatic areas
     * like web pages and help text. This Field will default to property name assigned to the parent Model with
     * underscores converted to spaces.
     * @param string $verbose_name_plural The plural form of $verbose_name. This defaults to $verbose_name with `s`
     * suffixed or `es` suffixes to strings already ending with `s`.
     * @param string $internal_name Assign a different field name to use when referring to the internal field as it's
     * stored in the pfSense configuration.
     * @param string $internal_namespace Sets the namespace this field belongs to internally. This can be used to nest
     * the field internal value under a specific namespace as an associative array. This only applies to the internal
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
        bool $required = false,
        bool $unique = false,
        bool $skip_unique_validation = false,
        mixed $default = null,
        string $default_callable = '',
        bool $allow_empty = false,
        bool $allow_null = false,
        bool $editable = true,
        bool $read_only = false,
        bool $write_only = false,
        bool $representation_only = false,
        mixed $indicates_true = '',
        mixed $indicates_false = null,
        string $verbose_name = '',
        string $verbose_name_plural = '',
        string $internal_name = '',
        string $internal_namespace = '',
        array $referenced_by = [],
        array $conditions = [],
        array $validators = [],
        string $help_text = '',
    ) {
        $this->indicates_true = $indicates_true;
        $this->indicates_false = $indicates_false;
        parent::__construct(
            type: 'boolean',
            required: $required,
            unique: $unique,
            skip_unique_validation: $skip_unique_validation,
            default: $default,
            default_callable: $default_callable,
            allow_empty: $allow_empty,
            allow_null: $allow_null,
            editable: $editable,
            read_only: $read_only,
            write_only: $write_only,
            representation_only: $representation_only,
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
     * Converts the field value to its representation form from it's internal pfSense configuration value.
     * @param mixed $internal_value The internal value from the pfSense configuration.
     * @return bool The field value in its representation form.
     */
    protected function _from_internal(mixed $internal_value): bool {
        # Check if the string matches our `indicates_true` value, if so, set this value as true.
        if ($internal_value === $this->indicates_true) {
            return true;
        }

        # Otherwise, we assume the value is `false`
        return false;
    }

    /**
     * Converts the field value to its internal form that is suitable for writing to the pfSense config.
     * @param mixed $representation_value The representation form of this field to convert to internal.
     * @return string|null The internal string used to indicate if the value is `true` or `false`.
     */
    protected function _to_internal(mixed $representation_value): string|null {
        # If the representation value is `true`, set it's internal value as the current `indicates_true` value.
        if ($representation_value === true) {
            return $this->indicates_true;
        }

        # Otherwise, just return the `indicates_false` value.
        return $this->indicates_false;
    }

    /**
     * Converts this Field object into a pfSense webConfigurator form input. This method can be overridden by a child
     * class to add custom input field creation.
     * @param string $type The HTML input tag type. Not all Fields support input types.
     * @param array $attributes An array of additional HTML input tag attributes. Not all Fields support input attributes.
     * @return object The pfSense webConfigurator form input object.
     * @link https://github.com/pfsense/pfsense/tree/master/src/usr/local/www/classes/Form
     */
    public function to_form_input(string $type = 'text', array $attributes = []): object {
        # Populate the GENERAL section of the UI form
        $input = new \Form_Checkbox(
            $this->name,
            ucwords($this->verbose_name),
            'Enable',
            $this->value,
            $this->indicates_true,
        );
        $input->setHelp($this->help_text);

        return $input;
    }
}
