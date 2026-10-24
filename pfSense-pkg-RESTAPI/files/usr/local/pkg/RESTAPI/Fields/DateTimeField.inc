<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';

use DateTime;
use RESTAPI;
use RESTAPI\Core\Field;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Field object for storing and validating DateTime values.
 */
class DateTimeField extends Field {
    /**
     * Defines the DateTimeField object and sets its options.
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
     * @param string $datetime_format the PHP DateTime format this value should be formatted as.
     * https://www.php.net/manual/en/datetime.format.php
     * @param bool $auto_add_now Automatically set the value of this field to the current DateTime during initial
     * creation. This field can be manually updated when this value is set.
     * @param bool $auto_update_now Automatically update the value of this field to the current DateTime whenever the
     * field is first created and whenever it is updated. This field will become read-only whenever this field is set.
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
        string $default_callable = '',
        array $choices = [],
        string $choices_callable = '',
        public string $datetime_format = 'm/d/Y',
        public bool $auto_add_now = false,
        public bool $auto_update_now = false,
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
        # Do not allow $auto_update_now and $many at the same time. The resulting array would just be redundant values.
        if ($auto_update_now and $many) {
            throw new ServerError(
                message: 'DateTimeField cannot set `auto_update_now` and `many` at the same time',
                response_id: 'DATETIME_FIELD_CANNOT_AUTO_UPDATE_NOW_WITH_MANY',
            );
        }

        # Construct the parent object for this field
        parent::__construct(
            type: 'string',
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

        # Set the Fields value to now if $auto_add_now is enabled
        if ($this->auto_add_now) {
            $current_time = (new DateTime())->format($this->datetime_format);
            $this->value = $this->many ? [$current_time] : $current_time;
        }
    }

    /**
     * @param string $datetime The datetime string to validate. (i.e. 12/31/1999)
     * @param string $format The DateTime format of the $datetime string. (i.e. m/d/Y)
     * @return bool `true` when the datetime string is valid, `false` when it is not.
     */
    public static function is_valid_datetime(string $datetime, string $format): bool {
        # Try to create a DateTime object with this datetime string and format, then check for errors
        $datetime_obj = DateTime::createFromFormat($format, $datetime);
        $errors = DateTime::getLastErrors();

        # This is not a valid datetime string if there are warnings
        if ($errors['warning_count']) {
            return false;
        }

        return $datetime_obj !== false;
    }

    /**
     * Converts the represented value into the internal pfSense value. In the event that `auto_update_now` is set, this
     * method will automatically update the current value to the current datetime before converting the value to internal.
     * @param mixed $representation_value The value to convert into it's internal form.
     * @return array|string|null The internal value(s) suitable for writing to the pfSense configuration.
     */
    protected function _to_internal(mixed $representation_value): array|string|null {
        # When `auto_update_now` is set, set the value to the current datetime.
        if ($this->auto_update_now) {
            $this->value = (new DateTime())->format($this->datetime_format);
            return parent::_to_internal($this->value);
        }

        # Otherwise, just return the value as usual
        return parent::_to_internal($representation_value);
    }

    /**
     * Converts the field value to its representation form from it's internal pfSense configuration value.
     * @param mixed $internal_value The internal value from the pfSense configuration.
     * @return string The field value in its representation form.
     */
    protected function _from_internal(mixed $internal_value): string {
        return strval($internal_value);
    }

    /**
     * Add extra validation to DateTimeFields to ensure the value matches the DateTime format.
     * @param mixed $value The incoming
     */
    public function validate_extra(mixed $value): void {
        # Check to ensure that the datetime value matches this Field's DateTime format.
        if (!$this->is_valid_datetime($value, $this->datetime_format)) {
            throw new ValidationError(
                message: "Field `$this->name` must be a DateTime in format `$this->datetime_format`. Received `$value`",
                response_id: 'DATETIME_FIELD_MUST_MATCH_FORMAT',
            );
        }
    }
}
