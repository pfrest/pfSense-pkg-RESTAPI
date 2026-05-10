<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Field;

/**
 * Defines a Field object for validating and storing a floating point number.
 */
class FloatField extends Field {
    /**
     * Defines the FloatField object and sets its options.
     * @param bool $required If `true`, this field is required to have a value at all times.
     * @param bool $unique If `true`, this field must be unique from all other parent model objects. Enabling this
     * option requires the Model $context to be set AND the Model $context must have a `config_path` set.
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
     * choices are dynamic and must be populate at runtime instead of pre-determined sets of values.
     * @param bool $allow_null If `true`, null values will be allowed by this field.
     * @param bool $editable Set to `false` to prevent this field's value from being changed after its initial creation.
     * @param bool $read_only If `true`, this field can only read its value and cannot write its value to config.
     * @param bool $write_only Set to `true` to make this field write-only. This will prevent the field's current value
     * from being displayed in the representation data. This is ideal for potentially sensitive Fields like passwords,
     * keys, and hashes.
     * @param bool $representation_only Set to `true` to make this field only present in its representation form. This
     * effectively prevents the Field from being converted to an internal value which is saved to the pfSense config.
     * This should only be used for Fields that do not relate directly to a configuration value.
     * @param bool $many If `true`, the value must be an array of many strings.
     * @param int $many_minimum When $many is set to `true`, this sets the minimum number of array entries required.
     * @param int $many_maximum When $many is set to `true`, this sets the maximum number of array entries allowed.
     * @param int $minimum The minimum value this value can be.
     * @param int $maximum The maximum value this value can be.
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
        bool $required = false,
        bool $unique = false,
        mixed $default = null,
        string $default_callable = '',
        array $choices = [],
        string $choices_callable = '',
        bool $allow_null = false,
        bool $editable = true,
        bool $read_only = false,
        bool $write_only = false,
        bool $representation_only = false,
        bool $many = false,
        int $many_minimum = 0,
        int $many_maximum = Field::MANY_MAXIMUM,
        public int $minimum = 0,
        public int $maximum = PHP_INT_MAX,
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
        parent::__construct(
            type: 'double', // gettype() identifies floats as doubles
            required: $required,
            unique: $unique,
            default: $default,
            default_callable: $default_callable,
            choices: $choices,
            choices_callable: $choices_callable,
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
            validators: $validators + [
                new RESTAPI\Validators\NumericRangeValidator(minimum: $minimum, maximum: $maximum),
            ],
            help_text: $help_text,
        );
    }

    /**
     * Converts the field value to its representation form from it's internal pfSense configuration value.
     * @param string $internal_value The internal value from the pfSense configuration.
     * @return float|null The field value in its representation form.
     */
    protected function _from_internal(mixed $internal_value): ?float {
        # Allow commas as the decimal separator, but convert them to points so it passes the is_numeric() check
        $internal_value = str_replace(',', '.', $internal_value);

        # Return the value as an integer if it's numeric
        if (is_numeric($internal_value)) {
            return floatval($internal_value);
        }

        # If the value is an empty string, assume it's null
        if ($internal_value === '') {
            return null;
        }

        # Otherwise, the internal value cannot be represented by this Field. Throw an error.
        throw new RESTAPI\Responses\ServerError(
            message: "Cannot parse FloatField '$this->name' from internal because its internal value is not " .
                'a numeric value. Consider changing this field to a StringField.',
            response_id: 'FLOAT_FIELD_WITH_NON_FLOAT_INTERNAL_VALUE',
        );
    }

    /**
     * Converts this Field object to a PHP array representation of an OpenAPI schema property configuration. This is
     * used when auto-generating API documentation. This method can be extended to add additional options to the OpenAPI
     * schema property.
     * @link https://swagger.io/docs/specification/data-models/
     * @return array A PHP array containing this field as a OpenAPI schema property configuration.
     */
    public function to_openapi_property(): array {
        # Run the parent to_openapi_property() to obtain the base property object, then make changes as needed.
        $openapi_property = parent::to_openapi_property();

        # Add the minimum and maximum to the OpenAPI property.
        if ($this->many) {
            $openapi_property['items']['type'] = 'number';
            $openapi_property['items']['format'] = 'float';
            $openapi_property['items']['minimum'] = $this->minimum;
            $openapi_property['items']['maximum'] = $this->maximum;
        } else {
            $openapi_property['type'] = 'number';
            $openapi_property['format'] = 'float';
            $openapi_property['minimum'] = $this->minimum;
            $openapi_property['maximum'] = $this->maximum;
        }

        return $openapi_property;
    }

    /**
     * Converts this Field object into a pfSense webConfigurator form input. This method can be overridden by a child
     * class to add custom input field creation.
     * @param string $type The HTML input tag type. Not all Fields support input types.
     * @param array $attributes An array of additional HTML input tag attributes. Not all Fields support input attributes.
     * @return object The pfSense webConfigurator form input object.
     * @link https://github.com/pfsense/pfsense/tree/master/src/usr/local/www/classes/Form
     */
    public function to_form_input(string $type = 'number', array $attributes = []): object {
        $attributes += ['min' => $this->minimum, 'max' => $this->maximum];
        return parent::to_form_input(type: $type, attributes: $attributes);
    }
}
