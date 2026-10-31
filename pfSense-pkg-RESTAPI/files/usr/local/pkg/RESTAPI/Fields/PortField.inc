<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Field;
use RESTAPI\Models\FirewallAlias;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Field object for validating and storing TCP/UDP port values in the pfSense configuration. This includes
 * options for port aliases and port ranges.
 */
class PortField extends Field {
    const REPR_RANGE_CHAR = ':';
    public bool $allow_alias;
    public bool $allow_range;
    public string $internal_range_char;

    /**
     * Defines the PortField object and sets its options.
     * @param bool $required If `true`, this field is required to have a value at all times.
     * @param bool $unique If `true`, this field must be unique from all other parent model objects. Enabling this
     * option requires the Model $context to be set AND the Model $context must have a `config_path` set.
     * @param bool $skip_unique_validation If `true`, skip uniqueness enforcement while retaining the `unique` metadata.
     * @param mixed|null $default Assign a default string value to assign this Field if no value is present.
     * @param array $choices An array of value choices this Field can be assigned. This can either be an indexed array
     * of the exact choice values, or an associative array where the array key is the exact choice value and the array
     * value is a verbose name for the choice. Verbose choice name are used by ModelForms when generating web pages
     * for a given Model.
     * @param string $choices_callable Assign a callable method from this Field object OR the parent Model context to
     * execute to populate choices for this field. This callable must be a method assigned on this Field object OR the
     * parent Model object that returns an array of valid choices in the same format as $choices. This is helpful when
     * choices are dynamic and must be populated at runtime instead of pre-determined sets of values.
     * @param bool $allow_alias If `true`, existing port firewall alias names will be allowed by this field.
     * @param bool $allow_range If `true`, port range values will be allowed by this field
     * @param string $internal_range_char The character to use internally that separates the two values in a range.
     * In some cases pfSense will store range values internally with different range characters. This only changes
     * the range character for the internal value, not the represented value which is always `:`.
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
        bool $required = false,
        bool $unique = false,
        bool $skip_unique_validation = false,
        mixed $default = null,
        array $choices = [],
        string $choices_callable = '',
        bool $allow_alias = false,
        bool $allow_range = false,
        string $internal_range_char = '-',
        bool $allow_empty = false,
        bool $allow_null = false,
        bool $editable = true,
        bool $read_only = false,
        bool $write_only = false,
        bool $representation_only = false,
        bool $many = false,
        int $many_minimum = 0,
        int $many_maximum = Field::MANY_MAXIMUM,
        ?string $delimiter = ',',
        string $verbose_name = '',
        string $verbose_name_plural = '',
        string $internal_name = '',
        string $internal_namespace = '',
        array $referenced_by = [],
        array $conditions = [],
        array $validators = [],
        string $help_text = '',
    ) {
        # Set properties specific to this field
        $this->allow_alias = $allow_alias;
        $this->allow_range = $allow_range;
        $this->internal_range_char = $internal_range_char;

        # Construct the parent object for this field
        parent::__construct(
            type: 'string',
            required: $required,
            unique: $unique,
            skip_unique_validation: $skip_unique_validation,
            default: $default,
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

        # To prevent the need to add redundant openapi to the help text, append a standard set of help text.
        $this->help_text .= " {$this->get_valid_opts_str()}";
    }

    /**
     * Adds extra validation to determine if this value should be accepted given the current Field settings.
     */
    public function validate_extra(mixed $value): void {
        # Allow this value if it is an existing port alias name and alias values are allowed
        if ($this->allow_alias and FirewallAlias::query(['type' => 'port', 'name' => $value])->exists()) {
            $this->labels[] = 'is_alias';
            return;
        }

        # Allow this value if it is a valid port range and ranges are allowed
        if ($this->allow_range and is_portrange($value)) {
            $this->labels[] = 'is_portrange';
            return;
        }

        # Always allow singular port values
        if (is_port($value)) {
            $this->labels[] = 'is_port';
            return;
        }

        # Any other value should be considered invalid, throw an error
        throw new ValidationError(
            message: "Field `$this->name` contains invalid port value `$value`. " . $this->get_valid_opts_str(),
            response_id: 'PORT_FIELD_INVALID',
        );
    }

    /**
     * Format a string of valid value options this Field's value can be. This is used to dynamically generate
     * help messages depending on this Field's current settings.
     * @return string A string representation of values that are considered valid for this Field.
     */
    private function get_valid_opts_str(): string {
        $opts_str = 'Valid options are: a TCP/UDP port number, ';
        $opts_str .= $this->allow_range ? 'a TCP/UDP port range separated by `' . self::REPR_RANGE_CHAR . '`, ' : '';
        $opts_str .= $this->allow_alias ? 'an existing port type firewall alias, ' : '';

        # Return the value but replace the trailing ", " characters
        return substr($opts_str, 0, -2);
    }

    /**
     * Converts the represented value into it's internal form.
     * @param mixed $representation_value The value to convert into it's internal form.
     * @return array|string|null The internal value(s) suitable for writing to the pfSense configuration.
     */
    protected function _to_internal(mixed $representation_value): array|string|null {
        # Return null values if they are already null
        if ($representation_value === null) {
            return null;
        }

        # Otherwise, replace the representation range character to the internal range character
        $representation_value = str_replace(self::REPR_RANGE_CHAR, $this->internal_range_char, $representation_value);

        return parent::_to_internal($representation_value);
    }

    /**
     * Converts the stored internal config value to its represented value.
     * @param string $internal_value The internal config value to convert to a representation value.
     * @throws ServerError When the child Field class does not override this method.
     */
    protected function _from_internal(string $internal_value) {
        # Replace the internal range character to the representation range character
        return str_replace($this->internal_range_char, self::REPR_RANGE_CHAR, $internal_value);
    }
}
