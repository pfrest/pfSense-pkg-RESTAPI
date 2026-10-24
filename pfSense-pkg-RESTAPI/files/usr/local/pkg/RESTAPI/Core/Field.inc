<?php

namespace RESTAPI\Core;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\LengthValidator;
use function RESTAPI\Core\Tools\is_assoc_array;
use const RESTAPI\Core\Globals\RESERVED_FIELD_NAMES;

/**
 * Defines Model Fields and their attributes. Field's define how value should be obtained on pfSense internally, and
 * converted into its representation form. Fields are also responsible for validation of this Field's value. This
 * includes strict type validation, choice options, whether the value can be null or empty, etc. Field's can also be
 * assigned custom Validator objects to add custom validation to the Field. Field's must be assigned as a properties
 * Model objects.
 */
class Field {
    use BaseTraits;

    /**
     * @const SENSITIVE_MASK The value that is used to mask the value of a $sensitive Field object. This only applies
     * to values shown in the webConfigurator.
     */
    const SENSITIVE_MASK = '********';

    /**
     * @const MANY_MAXIMUM is the maximum number of items a 'many' field can hold it's array.
     */
    const MANY_MAXIMUM = 65535;

    /**
     * Represents the current value for this Field.
     *
     * @var mixed $value
     */
    public mixed $value = null;

    /**
     * Contains an array of validation labels assigned to this Field. These labels indicate that specific validations have
     * already been performed and can be referenced, preventing redundant validation in the Model's `validate_*()` methods.
     *
     * @var array $labels
     */
    public array $labels = [];

    /**
     * Contains the verbose choices originally assigned in the $choices property during object construction.
     * This array is automatically populated using the $choices value passed in during object creation.
     *
     * @var array $verbose_choices
     *      An associative array where the key is the exact choice value, and the value is the verbose description for the choice.
     */
    public array $verbose_choices = [];

    /**
     * Sets the name of this Field object. Automatically set by the parent Model object to the Model's property name for this Field.
     * Do not attempt to modify this name outside the parent Model class.
     *
     * @var string $name
     */
    public string $name = '';

    /**
     * Represents the parent Model this Field object is assigned to. Use this to obtain additional context about the Model,
     * and other Fields/attributes assigned to the parent Model.
     *
     * @var Model|null $context
     */
    public Model|null $context = null;

    /**
     * Defines attributes for a Model Field.
     *
     * @param string $type
     *      The data type representation. Must match the type of the $value property.
     *      - If $allow_null is true, $value can also be `null`.
     *      - If $many is true, $value must be an array of the specified type.
     *      Format must be recognized by the PHP `gettype()` function.
     * @param bool $required
     *      Make this a required field. If false, a valid $default, $default_callable or $allow_null must be set.
     * @param bool $unique
     *      If true, $value must be unique among all parent Model objects (only for parent Models with $many set to true).
     * @param bool $skip_unique_validation
     *      If true, fields marked as unique will skip enforcement of the unique constraint. This is necessary for
     *      fields that use $unique for schema or indexing purposes, but do not need the constraint enforced.
     * @param mixed $default
     *      The default $value for this field if none is assigned. The default specified here should be static. If the
     *      default for this field is dynamic in nature, use a $default_callable instead.
     * @param string $default_callable
     *      Defines a callable method that should be called to populate the default value for this field. It is
     *      strongly encouraged to use a default callable when the default is variable and may change dynamically.
     * @param array $choices
     *      Defines explicit value choices for $value. Can be a linear array or associative array with verbose descriptions.
     *      After construction, verbose choices move to $verbose_choices.
     * @param string $choices_callable
     * @param bool $allow_empty
     *      If true, accepts empty values in $value (only for string type Fields and empty arrays for $many enabled Fields).
     * @param bool $allow_null
     *      If true, accepts null values in $value.
     * @param bool $editable
     *      If false, $value cannot be changed after initial set.
     * @param bool $read_only
     *      If true, marks this Field as read-only to remote clients.
     * @param bool $write_only
     *      If true, marks this Field as write-only, excluding it from API Responses.
     * @param bool $sensitive
     *      If true, marks this Field as 'sensitive', masking its value in web form input Fields.
     * @param bool $representation_only
     *      If true, excludes this Field when writing the parent Model object to configuration.
     * @param bool $many
     *      If true, $value must be an array of the specified type, with individual values validated separately.
     * @param int $many_minimum
     *      Minimum number of values required for Fields with $many enabled.
     * @param int $many_maximum
     *      Maximum number of values allowed for Fields with $many enabled.
     * @param string|null $delimiter
     *      Specifies the delimiter used to store this value in internal pfSense. Set to `null` for array storage.
     * @param string $verbose_name
     *      Human-friendly name used for input field names in associated \RESTAPI\Core\Form classes.
     * @param string $verbose_name_plural
     *      Plural form of $verbose_name. Defaults to appending 's' or 'es' to $verbose_name.
     * @param string $internal_name
     *      Name of this Field as shown in the pfSense configuration or internal callable.
     * @param string $internal_namespace
     *      If $internal_value is nested, set to the associative array's key.
     * @param array $referenced_by
     *      Array specifying Models and Fields referencing this Field's parent Model using this Field's value.
     * @param array $conditions
     *      Array of conditions for inclusion based on other Fields in the parent Model context.
     * @param array $validators
     *      Array of \RESTAPI\Core\Validator objects for validating this field.
     * @param string $help_text
     *      Help text used to describe this Field in OpenAPI documentation and associated \RESTAPI\Core\Form input Fields.
     */
    public function __construct(
        public string $type = '',
        public bool $required = false,
        public bool $unique = false,
        public bool $skip_unique_validation = false,
        public mixed $default = null,
        public string $default_callable = '',
        public array $choices = [],
        public string $choices_callable = '',
        public bool $allow_empty = false,
        public bool $allow_null = false,
        public bool $editable = true,
        public bool $read_only = false,
        public bool $write_only = false,
        public bool $sensitive = false,
        public bool $representation_only = false,
        public bool $many = false,
        public int $many_minimum = 0,
        public int $many_maximum = Field::MANY_MAXIMUM,
        public string|null $delimiter = ',',
        public string $verbose_name = '',
        public string $verbose_name_plural = '',
        public string $internal_name = '',
        public string $internal_namespace = '',
        public array $referenced_by = [],
        public array $conditions = [],
        public array $validators = [],
        public string $help_text = '',
    ) {
        # Allow null values if this Field is read_only
        $this->allow_null = $this->read_only ? true : $this->allow_null;

        # Set the many_minimum value to 0 if allow_empty and many are enabled
        if ($this->many and $this->allow_empty) {
            $this->many_minimum = 0;
        }

        # Assign choices
        $this->set_choices($choices);

        # Ensure values do not conflict
        $this->check_construct();
    }

    /**
     * Allows the Field's current value to be obtained by invoking the field object.
     * @note This doesn't work in the context of a Model object because you cannot invoke a class property that is
     * assigned an invocable object.
     * @returns mixed The value currently set in $this->value
     */
    final public function __invoke(): mixed {
        return $this->value;
    }

    /**
     * Checks that values passed in during __construct are valid and do not conflict.
     * @throws ServerError When any Field properties have conflicting values.
     */
    private function check_construct(): void {
        # Throw an error if `required` and `default` are both set
        if ($this->required and $this->has_default()) {
            throw new ServerError(
                message: 'Required Fields cannot have a default.',
                response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_DEFAULT',
            );
        }

        # Throw an error if `default` and `default_callable` are both set
        if (isset($this->default) and $this->default_callable) {
            throw new ServerError(
                message: 'Required Fields cannot have a default and default_callable.',
                response_id: 'FIELD_WITH_DEFAULT_AND_DEFAULT_CALLABLE',
            );
        }

        # Throw an error if `required` and `read_only` are both set
        if ($this->required and $this->read_only) {
            throw new ServerError(
                message: 'Required Fields cannot be read only.',
                response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_READ_ONLY',
            );
        }

        # Throw an error if `required` and `allow_null` are both set
        if ($this->required and $this->allow_null) {
            throw new ServerError(
                message: 'Required Fields cannot allow null values.',
                response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_ALLOW_NULL',
            );
        }

        # Do not allow this field to be `read_only` AND `write_only`
        if ($this->read_only and $this->write_only) {
            throw new ServerError(
                message: 'Fields cannot be both read-only and write-only.',
                response_id: 'FIELD_WITH_READ_ONLY_AND_WRITE_ONLY',
            );
        }

        # Do not allow the `many_minimum` value to greater than the `many_maximum` value
        if ($this->many_minimum > $this->many_maximum) {
            throw new ServerError(
                message: "Fields cannot have a 'many_minimum' value greater than 'many_maximum'",
                response_id: 'FIELD_MANY_MINIMUM_GREATER_THAN_MANY_MAXIMUM',
            );
        }

        # Do not allow `choices` and a `choices_callable` to be set at the same time
        if ($this->choices and $this->choices_callable) {
            throw new ServerError(
                message: "Fields cannot have `choices' and a `choices_callable` assigned",
                response_id: 'FIELD_WITH_CHOICES_AND_CHOICES_CALLABLE',
            );
        }

        # For `many` Fields, automatically set `many_minimum` to 0 if `allow_empty` is enabled and vice versa
        if ($this->many and $this->allow_empty) {
            $this->many_minimum = 0;
        }
        if ($this->many and $this->many_minimum === 0) {
            $this->allow_empty = true;
        }
    }

    /**
     * Sets this Field's name property and assumes defaults based on current name assignments. This field is intended
     * to be called by the parent Model that is assigned this Field objects. Names are based off the name of the
     * property assigned this Field in the parent Model.
     * @param string $name The name to assign this Field.
     */
    public function set_names(string $name): void {
        # Throw an error if this Field name is reserved
        if (in_array($name, RESERVED_FIELD_NAMES)) {
            throw new ServerError(
                message: "Field cannot be assigned name `$name` because it is reserved by the system.",
                response_id: 'FIELD_CANNOT_USE_RESERVED_NAME',
            );
        }

        # Assign the `name` property using the provided name
        $this->name = $name;

        # If no `internal_name` was provided, assume it's the same as the `name`
        if (!$this->internal_name) {
            $this->internal_name = $this->name;
        }

        # If no `verbose_name` was provided, assign it based off the current `name`
        if (!$this->verbose_name) {
            $this->verbose_name = str_replace('_', ' ', $this->name);
        }

        # If no `verbose_name_plural` was provided, assign it based off the current `name`.
        if (!$this->verbose_name_plural) {
            $this->verbose_name_plural = str_ends_with($this->verbose_name, 's') ? 'es' : 's';
        }
    }

    /**
     * Accepts a given array of choices and sets the `choices` and `verbose_choices` properties accordingly.
     * @param array $choices An array of incoming choices to set
     */
    public function set_choices(array $choices): void {
        # Format choices accordingly when they are provided as an associative array
        if (is_assoc_array($choices)) {
            $this->verbose_choices = $choices;
            $this->choices = array_keys($choices);
        }
        # Otherwise, format choices accordingly for indexed arrays
        else {
            # Default verbose choice to the value, replace underscores with spaces and capitalize first letter
            $this->choices = $choices;
            foreach ($choices as $choice) {
                $this->verbose_choices[$choice] = ucfirst(str_replace('_', ' ', $choice));
            }
        }
    }

    /**
     * Sets choices by calling the `choices_callable` method if present.
     * @throws ServerError When the choices_callable could not be found in either this Field object or its parent Model
     */
    public function set_choices_from_callable(): void {
        # If a choices callable is assigned, run the callable to populate the choices if allowed
        if ($this->choices_callable) {
            # Set a variable we can use to dynamic invoke the callable
            $callable = $this->choices_callable;

            # First check if the parent Model context has a callable with this name. If so, run it and assign choices
            if ($this->context and in_array($callable, get_class_methods($this->context))) {
                $this->set_choices($this->context->$callable());
            }
            # Next, check if this Field object has a callable with this name. If so, run it and assign choices
            elseif (in_array($callable, get_class_methods($this))) {
                $this->set_choices($this->$callable());
            }
            # Otherwise, throw an error because we could not find a callable with this name
            else {
                throw new ServerError(
                    message: "Could not find choices callable method `$callable` in Field object nor its parent Model.",
                    response_id: 'FIELD_CHOICES_CALLABLE_NOT_FOUND',
                );
            }
        }
    }

    /**
     * Checks if this field has a default or default_callable assigned.
     * @return bool Returns true if this field has a default or default_callable assigned, returns false otherwise.
     */
    final protected function has_default(): bool {
        return isset($this->default) or $this->default_callable;
    }

    /**
     * Obtains the default value for this field.
     * @return mixed When the $default property is set, its value will be returned. If the $default_callable is set,
     * the return value of that callable will be returned. If no default has been specified, null will be returned.
     */
    final public function get_default(): mixed {
        if (isset($this->default)) {
            return $this->default;
        }
        if ($this->default_callable) {
            return $this->get_default_from_callable();
        }

        return null;
    }

    /**
     * Obtains this Field's default value by calling the `default_callable` method if defined.
     * @return mixed The return value of the default_callable if defined. Otherwise, returns null.
     * @throws ServerError When the default_callable could not be found in either this Field object or its parent Model
     */
    private function get_default_from_callable(): mixed {
        # Only do this if a default_callable is assigned
        if ($this->default_callable) {
            # Set a variable we can use to dynamic invoke the callable
            $callable = $this->default_callable;

            # First check if the parent Model context has a callable with this name. If so, run it and assign choices
            if ($this->context and in_array($callable, get_class_methods($this->context))) {
                return $this->context->$callable();
            }
            # Next, check if this Field object has a callable with this name. If so, run it and assign choices
            if (in_array($callable, get_class_methods($this))) {
                return $this->$callable();
            }
            # Otherwise, throw an error because we could not find a callable with this name
            throw new ServerError(
                message: "Could not find default callable method `$callable` in Field object nor its parent Model.",
                response_id: 'FIELD_DEFAULT_CALLABLE_NOT_FOUND',
            );
        }

        return null;
    }

    /**
     * Checks if this Field object has a specific label assigned. Labels are specific attributes that assigned
     * Validator objects found during validation that may need to be referenced again after validation.
     * @param string $label_name The name of the label to check for (e.g. `is_ipaddrv4`)
     * @return bool Returns `true` if this Field object is assigned the $label_name, `false` if it is not.
     */
    public function has_label(string $label_name): bool {
        return in_array($label_name, $this->labels);
    }

    /**
     * Runs all validations for this field.
     * @param ModelSet|null $modelset Sets a specific ModelSet to use when validations require the use of all existing
     * objects for this Field's $context Model. The only current use case for this is in \RESTAPI\Fields\NestedModelField
     * where we need to validate many objects before they are created.
     * @return bool `true` if all validation succeeded.
     */
    public function validate(?ModelSet $modelset = null): bool {
        # Ensure model context exists when needed
        $this->check_model_context();

        # Do not validate this field if conditions are not met, unset any existing value.
        if (!$this->are_conditions_met()) {
            $this->value = null;
            return true;
        }

        # Ensure this field object has a name set
        $this->check_field_names();

        # Ensure a value is specified if field is `required`
        $this->check_field_required();

        # Ensure a value is editable before allow values to change
        $this->is_value_editable();

        # Ensure a value is not set if field is `read_only`
        $this->check_field_read_only();

        # Ensure an appropriate number of array entries are specified for `many` Fields
        $this->check_many_value_length();

        # Ensure the value(s) to validate are always in an array, so we can loop.
        $values = $this->get_value_as_array();

        # Ensure value is not an empty array before we start the loop unless allowed
        $this->check_value_empty($this->value);

        # Don't validate any further if this field allows null values and the value is null
        if ($this->allow_null and $this->value === null) {
            return true;
        }

        # Reset Field labels before running Validators so they can be repopulated if needed
        $this->labels = [];

        # Loop through each value to be validated
        foreach ($values as $value) {
            # Ensure the value's type is supported
            $this->check_value_type($value);

            # Ensure this value is an available choice (if `choices` are defined for this field)
            $this->check_value_choice($value);

            # Ensure value is unique if set
            $this->check_field_unique($value, modelset: $modelset);

            # Ensure this value is not empty unless empty values are allowed by this field. If value is empty and
            # empty values are allowed (returns `true`, skip remaining validation as the empty value itself is
            # considered a valid value.
            if ($this->check_value_empty($value)) {
                return true;
            }

            # Loop through each validator and run it's `validate` method against the current value.
            foreach ($this->validators as $validator) {
                # Provide the Validator context to this Field and it's parent Model
                $validator->model_context = $this->context;
                $validator->field_context = $this;
                $validator->validate(value: $value, field_name: $this->name);
            }

            # Run any extra validation this Field may have
            $this->validate_extra($value);
        }
        return true;
    }

    /**
     * This method can be overridden by a child Field class to added extra validation for the Field. This method is
     * executed at the end of the `validate()` method.
     * @param mixed $value The value being validated. In the event that this is a `many` field, this method will
     * receive each value of the array individually, not the array value itself.
     */
    public function validate_extra(mixed $value): void {}

    /**
     * Ensures this field has a parent model context assigned if a parent model context is required.
     * @throws ServerError When `unique` or `conditions` are set, but no Model `context` has been assigned.
     */
    private function check_model_context(): void {
        # Throw an error if this field is unique but no model context was provided
        if ($this->unique and !$this->context) {
            throw new ServerError(
                message: 'Unique Fields must have model context provided.',
                response_id: 'FIELD_UNIQUE_WITH_NO_CONTEXT',
            );
        }

        # Throw an error if this field is a conditional field, but no model context was provided
        if ($this->conditions and !$this->context) {
            throw new ServerError(
                message: 'Conditional Fields must have model context provided.',
                response_id: 'FIELD_CONDITIONAL_WITH_NO_CONTEXT',
            );
        }
    }

    /**
     * Checks if this Field should allow changed values. If this is not a `editable` Field, and the value given differs
     * from the stored value from the parent Model context an error will be thrown
     * @throws ValidationError When this is not an `editable` Field, but the value given differs from the value stored
     * for the parent Model context
     */
    private function is_value_editable(): void {
        # Only proceed with validation if the parent Model has an initial object set and this value is not editable
        if (is_null($this->context->initial_object) or $this->editable) {
            return;
        }

        # Obtain the current value for this field
        $current_value = $this->context->initial_object->{$this->name}->value;

        # Throw an error if this is a non `editable` field but the values changed.
        if ($this->value !== $current_value) {
            throw new ValidationError(
                message: "Field `$this->name` is not editable but the requested value `$this->value` differs from its " .
                    "current value `$current_value`.",
                response_id: 'FIELD_VALUE_CHANGED_WHEN_NOT_EDITABLE',
            );
        }
    }

    /**
     * Checks if the conditions defined in the `conditions` property are met.
     * @note The context of the condition can be inverted by prefixing ! to the conditions array key.
     * @note The condition can reference a parent's Model Field by prefixing parent__ to the conditions array key.
     * @returns bool `true` if all conditions were met, `false` if any conditions failed.
     */
    private function are_conditions_met(): bool {
        # Variables
        $conditions_met = true;

        # Consider conditions to be met if no conditions were specified
        if (!$this->conditions) {
            return true;
        }

        # Loop through each condition and check if it is met
        foreach ($this->conditions as $condition_field => $condition_value) {
            # Check if this field is inverted, strip the invert character if so
            if (str_starts_with($condition_field, '!')) {
                $is_inverted = true;
                $condition_field = substr($condition_field, 1);
            } else {
                $is_inverted = false;
            }

            # Check if the condition applies to the context model's parent
            if (str_starts_with($condition_field, 'parent__')) {
                $is_parent_field = true;
                $condition_field = str_replace('parent__', '', $condition_field);
            } else {
                $is_parent_field = false;
            }

            # Use the parent model as the context if this is a parent field, otherwise use this field's context model
            $context_model = $is_parent_field ? $this->context->parent_model : $this->context;

            # Safety check to ensure the Model context even has a field with the field name specified in the condition
            if (!in_array($condition_field, $context_model->get_fields())) {
                $assigned_model_class = get_class($context_model);
                throw new ServerError(
                    message: "Field '$this->name' contains a condition for field `$condition_field` from 
                        its assigned Model '$assigned_model_class', but '$assigned_model_class' has no such field. ",
                    response_id: 'FIELD_CONDITIONS_REQUIRE_NON_EXISTENT_MODEL_FIELD',
                );
            }

            $condition_field_value = $context_model->$condition_field->value;

            # For 'value in array' conditions, mark condition as unmet if value is not present in the array
            if (!$is_inverted and is_array($condition_value) and !in_array($condition_field_value, $condition_value)) {
                $conditions_met = false;
                break;
            }
            # For 'value equals' conditions, mark condition as unmet if values are not equal
            if (!$is_inverted and !is_array($condition_value) and $condition_field_value !== $condition_value) {
                $conditions_met = false;
                break;
            }
            # For 'value not in array' conditions, mark condition as unmet if value is present in the array
            if ($is_inverted and is_array($condition_value) and in_array($condition_field_value, $condition_value)) {
                $conditions_met = false;
                break;
            }
            # For 'value does not equal' conditions, mark condition as unmet if values are equal
            if ($is_inverted and !is_array($condition_value) and $condition_field_value === $condition_value) {
                $conditions_met = false;
                break;
            }
        }

        return $conditions_met;
    }

    /**
     * When the field is required, checks if a value is set and checks if mutually exclusive options are both set.
     * @throws ServerError When `required` and `default` are both set OR `required` and `allow_null` are both set.
     * @throws ValidationError If `required` is set but no value was assigned.
     */
    private function check_field_required(): void {
        # Throw an error if this field is required but no value was set.
        if ($this->required and is_null($this->value)) {
            throw new ValidationError(message: "Field `$this->name` is required.", response_id: 'FIELD_IS_REQUIRED');
        }

        # Throw an error if field is not required but no default was specified and `allow_null` is not set.
        if (!$this->required and !$this->has_default() and !$this->allow_null) {
            throw new ServerError(
                message: "Field `$this->name` must have a default or allow null values if not required.",
                response_id: 'FIELD_NEEDS_DEFAULT',
            );
        }
    }

    /**
     * Ensures read-only values do not have a value, default or choices.
     * @throws ServerError When `read_only` is set and `choices` or `default` are also set.
     * @throws ValidationError When `read_only` is set and a non-null value was requested.
     */
    private function check_field_read_only(): void {
        # Throw an error if the field is read only, but choices were defined
        if ($this->read_only and $this->choices) {
            throw new ServerError(
                message: "Read-only field `$this->name` cannot have value choices.",
                response_id: 'FIELD_NO_CHOICES_WITH_READ_ONLY',
            );
        }
    }

    /**
     * Checks the length of the value array when `many` is enabled. This ensures there are at least as many array
     * entries as specified with `many_minimum` but no more array entries than specified with `many_maximum`.
     */
    private function check_many_value_length(): void {
        # Only validate this if 'many' is enabled.
        if ($this->many and is_array($this->value)) {
            # Ensure the value contains an appropriate number of array entries
            $validator = new LengthValidator(minimum: $this->many_minimum, maximum: $this->many_maximum);
            $validator->validate($this->value, $this->name);
        }
    }

    /**
     * Obtains an array of individual values for this field. If `many` is enabled, this method will ensure `value` is
     * already an array and simply return the array value. If `many` is not enabled, the individual `value` will be
     * wrapped in an array and returned. This intended to obtain ALL values that need to validated into
     * an array format for looping purposes.
     * @returns array An array containing all individual values that need validation.
     * @throws ValidationError When `many` is enabled, but `value` is not an array.
     */
    private function get_value_as_array(): array|null {
        # Assume the default value if no value was set and a default was specified
        if ($this->has_default() and is_null($this->value)) {
            $this->value = $this->get_default();
        }

        # Check if many is enabled
        if ($this->many) {
            # Throw an error if the value is not already an array
            if (!is_array($this->value) and !is_null($this->value)) {
                throw new ValidationError(
                    message: "Field `$this->name` must be of type `array`.",
                    response_id: 'FIELD_INVALID_MANY_VALUE',
                );
            }

            # Return the value array here if we didn't throw an error in the previous condition
            return $this->value;
        }

        # When many is not enabled, wrap the current `value` in an array so it can be used for looping.
        return [$this->value];
    }

    /**
     * Checks if this Field object has a valid `name` and `internal_name`. In the event that a `name` exists, but an
     * `internal_name` was not specified, the `internal_name` will be assigned the same value as `name` by this method.
     * @throws ServerError When this object's `name` has not been set.
     */
    public function check_field_names(): void {
        # Throw an error if no `name` has been set.
        if (empty($this->name)) {
            throw new ServerError(
                message: 'Field object must be assigned a `name` value after it is constructed.',
                response_id: 'FIELD_UNNAMED',
            );
        }
        # Throw an error if the name contains double underscores (`__`) as this can conflict with queries
        if (str_contains($this->name, '__')) {
            throw new ServerError(
                message: "Field name '$this->name' is invalid because it contains double underscores. Double " .
                    'underscores are reserved for queries only.',
                response_id: 'FIELD_NAME_WITH_DOUBLE_UNDERSCORE_NOT_ALLOWED',
            );
        }
    }

    /**
     * Checks if a 'unique' field is actually unique from all other objects related to the $context Model's config path.
     * The $context Model must have a `config_path` set AND have it's `many` value set to `true` to support unique
     * validation.
     * @param mixed $value The value to being checked for uniqueness.
     * @param ModelSet|null $modelset Sets a specific ModelSet to use when checking uniqueness. If set, this Field's
     * value must be unique from all other Models in this ModelSet. If no value is specified here, a ModelSet will
     * automatically be obtained for all existing Model objects for this Field's $context Model. Note: This is rarely
     * needed. The only current use case is \RESTAPI\Fields\NestedModelField as it needs to check the uniqueness of
     * a set of Model objects before they are created.
     * @throws ValidationError When the assigned $context Model does not have a `config_path` set OR the assigned
     * $context Model does not have it's $many value to `true`.
     * @throws ServerError When $unique is enabled but a $config_path is not assigned to the parent model $context
     * @throws ServerError When $unique is enabled, but the parent model $context does not have $many enabled
     */
    private function check_field_unique(mixed $value, ?ModelSet $modelset = null): void {
        # Only check this if `unique` is enabled and the unique constraint is enforced
        if ($this->unique and !$this->skip_unique_validation) {
            # Throw an error if unique is set but the model context is not a 'many' model.
            if (!$this->context->many) {
                throw new ServerError(
                    message: "Field '$this->name' cannot be 'unique' when its model context does not have 'many' enabled.",
                    response_id: 'FIELD_UNIQUE_WITHOUT_MANY_MODEL_CONTEXT',
                );
            }

            # Use $modelset if provided. Otherwise, use all existing $context model objects as the $modelset
            $modelset = $modelset ?: $this->context->query(parent_id: $this->context->parent_id);

            # If this is a $many field, query for any other object has this value in the array
            if ($this->many) {
                $query = $modelset->query(
                    query_params: ['id__except' => $this->context->id, "{$this->name}__contains" => $value],
                );
            }
            # Otherwise, query for any other of the parent context's Model objects with the same exact field value
            else {
                $query = $modelset->query(query_params: ['id__except' => $this->context->id, $this->name => $value]);
            }

            # If we found another object with matching Fields, throw a validation error
            if ($query->exists()) {
                $model_object = $query->first();
                throw new ValidationError(
                    message: "Field '$this->name' must be unique. Value is in use by object with ID '$model_object->id'.",
                    response_id: 'FIELD_MUST_BE_UNIQUE',
                );
            }
        }
    }

    /**
     * Checks if a given value's type matches this Field object's primary `type`
     * @param mixed $value The value to check against assigned `type`.
     * @throws ValidationError If the specified value is not a supported type.
     */
    protected function check_value_type(mixed $value) {
        # Don't validate type for `read_only` Fields
        if ($this->read_only) {
            return;
        }

        # Allow value to be NULL if `allow_null` is set
        if ($this->allow_null and is_null($value)) {
            return;
        }

        # Raise an exception if the value is not the supported type.
        if ($this->type !== gettype($value)) {
            throw new ValidationError(
                message: "Field `$this->name` must be of type `$this->type`",
                response_id: 'FIELD_INVALID_TYPE',
            );
        }
    }

    /**
     * Checks if a given `value` is a valid `choice` option when `choices` are specified.
     * @param mixed $value The value to check against available choices.
     * @returns ValidationError When the $value is not an available choice.
     */
    private function check_value_choice(mixed $value): void {
        # Skip choice validation if no choices were specified and no choice callable was assigned
        if (!$this->choices and !$this->choices_callable) {
            return;
        }

        # Assign choices
        $this->set_choices_from_callable();

        # If choices are set, ensure this value is one of the choices
        if ($this->choices and !in_array($value, $this->choices)) {
            # Convert the choices array to a string format and throw a choice not supported
            $choices_str = implode(', ', $this->choices);
            throw new ValidationError(
                message: "Field `$this->name` must be one of [$choices_str]",
                response_id: 'FIELD_INVALID_CHOICE',
            );
        }
    }

    /**
     * Checks if a given `value` is an empty array or string and checks if empty values are allowed.
     * @param mixed $value The value to check for emptiness.
     * @return bool Returns `true` if value is empty and empty values are allowed. Returns `false` otherwise.
     * @throws  ValidationError When the $value is empty but empty empty values are not allowed
     */
    private function check_value_empty(mixed $value): bool {
        # Skip this validation for read_only Fields
        if ($this->read_only) {
            return true;
        }

        # Check if value is an empty string or array. These are the only two types we consider possible to be empty
        # to avoid having to enable `allow_empty` for booleans, 0 integers, and null values
        # (which are checked separately).
        if (!$this->allow_empty and ($value === '' or $value === [])) {
            throw new ValidationError(
                message: "Field `$this->name` cannot be empty.",
                response_id: 'FIELD_EMPTY_NOT_ALLOWED',
            );
        }
        # Return true if empty values are allowed and the value was empty
        elseif ($this->allow_empty and ($value === '' or $value === [])) {
            return true;
        }

        return false;
    }

    /**
     * Converts the stored internal config value to a represented value. This method must be overridden by a child
     * Field class with the necessary steps to convert the internal value to the representation value. This method must
     * return value in it's represented form! In the case of a `many` field, this method will be called for each
     * individual value after it's been converted to an array by from_internal(). You do not need to worry about
     * converting the $internal_value to an array in this method!
     * @param string $internal_value The internal config value to convert to a representation value.
     * @throws ServerError When the child Field class does not override this method.
     */
    protected function _from_internal(string $internal_value) {
        throw new ServerError(
            message: "Field '$this->name' does not have method of converting internal value to representation.",
            response_id: 'FIELD_CANNOT_REPRESENT_INTERNAL_VALUE',
        );
    }

    /**
     * Sets a public version of _from_internal() that calls the Field's _from_internal() method. In the case
     * of a `many` Field, the internal value will be expanded into it's array form and _from_internal() will run
     * against each item within that array. This method will automatically assign the representation value to the
     * `value` property for this object.
     * @param string|null $internal_value The raw internal config value to convert to a representation value.
     */
    public function from_internal(mixed $internal_value): void {
        # Keep null values for non-boolean values and representation only values
        if ($this->representation_only or is_null($internal_value) and $this->type !== 'boolean') {
            # Check if this field has a default, if so, use the default value
            if ($this->default and $this->are_conditions_met()) {
                $this->value = $this->default;
                return;
            }
            # Otherwise, just assume the value is null
            else {
                $this->value = null;
                return;
            }
        }

        # For `many` type Fields, convert all values to representation.
        if ($this->many) {
            # Assume the internal value is already an array if the delimiter is `null`
            if ($this->delimiter === null and !empty($internal_value)) {
                $values = $internal_value;
            }
            # Assume an empty array if the internal value is empty
            elseif (empty($internal_value)) {
                $values = [];
            }
            # Otherwise, split the internal value into it's array representation
            else {
                $values = explode($this->delimiter, $internal_value);
            }

            # Run the field's `to_representation()` method for value to represent
            $representation = [];
            foreach ($values as $value) {
                $representation[] = $this->_from_internal($value);
            }

            $this->value = $representation;
        }
        # For boolean values that do not have their conditions met, set to null.
        elseif ($this->type === 'boolean' and !$this->are_conditions_met()) {
            $this->value = null;
        }
        # Otherwise, just convert the value to representation directly if conditions are met
        else {
            $this->value = $this->_from_internal($internal_value);
        }
    }

    /**
     * Converts the represented value into the internal pfSense value. In most cases child Field classes do not need to
     * override this method as the default behavior of converting the value into a string is sufficient. However, this
     * method can be overridden by a child Field class with custom steps to convert the represented value back into
     * the internal value if needed. This method must return the value in its internal form! In the case of a `many`
     * field, this method will be called for each `$this->value` before it's been joined into it's internal string by
     * to_internal(). If `$this->value` is an array, you do not need to worry about joining `$this->value` into a
     * string. Leave it as an array.
     * @param mixed $representation_value The value to convert into it's internal form.
     * @return array|string|null The internal value(s) suitable for writing to the pfSense configuration.
     */
    protected function _to_internal(mixed $representation_value): array|string|null {
        return is_null($representation_value) ? null : strval($representation_value);
    }

    /**
     * Sets a public version of _to_internal() that calls the Field's _to_internal() method. In the case
     * of a `many` Field, _to_internal() method will run against all current values and will then be joined back into
     * the internal string value suitable for writing to the pfSense configuration.
     * @return array|string|null The internal array or string value suitable for writing the pfSense configuration.
     */
    public function to_internal(): array|string|null {
        # Do not convert 'representation_only' Fields to internal
        if ($this->representation_only) {
            return null;
        }

        # For `many` Fields, allow for individual value conversions before joining array back into internal string value
        if ($this->many) {
            $internal_values = [];

            # Run the field's `to_representation()` method for value to represent
            foreach ($this->value as $value) {
                $internal_values[] = $this->_to_internal($value);
            }

            # Return the values as an array if the delimiter is null, otherwise piece the array back into a string
            return $this->delimiter === null ? $internal_values : implode($this->delimiter, $internal_values);
        }
        # Otherwise, just convert the individual value to it's internal value
        else {
            return $this->_to_internal($this->value);
        }
    }

    /**
     * Retrieves the related Model object based on this Field's value. Child Field classes must override this method
     * to define the necessary steps for retrieving related objects. This method can only be used on Fields with
     * $many set to false.
     * @returns Model Returns the single Model object related to this Field's current value or `null` if no related
     * Model was found.
     */
    public function get_related_model(): Model|null {
        return null;
    }

    /**
     * Retrieves all related Model object based on this Field's value. Child Field classes must override this method
     * to define the necessary steps for retrieving related objects. This method can only be used on Fields with
     * $many set to true.
     * @returns ModelSet Returns a ModelSet of all related objects associated with this Fields current values.
     */
    public function get_related_models(): ModelSet {
        return new ModelSet();
    }

    /**
     * Uses this Field's $referenced_by property to search for existing Model objects that reference this Field's
     * current value.
     * @return ModelSet
     */
    public function is_referenced_by(): ModelSet {
        # Use a variable to keep track of Model objects that reference this field
        $references = new ModelSet();

        # Loop through each Model specified in `referenced_by`
        foreach ($this->referenced_by as $reference_model => $reference_fields) {
            # Ensure the reference Fields are always an array and reference model is FQN
            $reference_fields = is_array($reference_fields) ? $reference_fields : [$reference_fields];
            $reference_model = "\\RESTAPI\\Models\\$reference_model";

            # Loop through each referenced field and query for matches.
            foreach ($reference_fields as $reference_field) {
                # Query for any referenced Model objects that have this field value assigned to the referenced field
                $reference_model = new $reference_model();
                $reference_query = $reference_model->query(
                    query_params: [$reference_field => $this->value],
                    parent_id: $this->context->parent_id,
                );
                $references->model_objects = $references->model_objects + $reference_query->model_objects;
            }
        }

        return $references;
    }

    /**
     * Converts this Field object to a PHP array representation of an OpenAPI schema property configuration. This is
     * used when auto-generating API documentation. This method can be extended to add additional options to the OpenAPI
     * schema property.
     * @link https://swagger.io/docs/specification/data-models/
     * @return array A PHP array containing this field as an OpenAPI schema property configuration.
     */
    public function to_openapi_property(): array {
        # Assign primitive property attributes
        $openapi_property = [];
        $openapi_property['description'] = $this->help_text . '<br>' . $this->get_conditions_text();
        $openapi_property['nullable'] = $this->allow_null;
        $openapi_property['readOnly'] = $this->read_only;
        $openapi_property['writeOnly'] = $this->write_only;

        # Set the default if defined. Note: We purposefully do not include defaults via a default_callable here as
        # they may be dynamic and make the documented default inaccurate.
        if ($this->default) {
            $openapi_property['default'] = $this->default;
        }

        # Set values that are unique to `many` Fields.
        if ($this->many) {
            $openapi_property['type'] = 'array';
            $openapi_property['minItems'] = $this->many_minimum;
            $openapi_property['maxItems'] = $this->many_maximum;
            $openapi_property['items'] = ['type' => $this->type];

            # Assign choices if defined
            if ($this->choices) {
                $openapi_property['items']['enum'] = $this->choices;
            }
        } else {
            $openapi_property['type'] = $this->type;

            # Assign choices if defined
            if ($this->choices and !$this->choices_callable) {
                $openapi_property['enum'] = $this->choices;
            }
        }

        return $openapi_property;
    }

    /**
     * Formats a descriptive string to explain the `conditions` for this Field. This is intended to be used for
     * OpenAPI documentation generation.
     * @return string A descriptive string explaining the `conditions` for this Field.
     */
    public function get_conditions_text(): string {
        # Variables
        $conditions_text = '';

        # Only proceed when this field has conditions
        if ($this->conditions) {
            $conditions_text = '<br>This field is only available when the following conditions are met:<br>';

            # Loop through required conditions
            foreach ($this->conditions as $field => $value) {
                # Stored a string representation of the $value
                $value_str = var_export($value, true);

                # Determine if the context of the condition is inverted or applies to a parent model
                $is_inverted = str_starts_with($field, '!');
                $is_parent_field = str_starts_with($field, 'parent__');

                # Remove the invert prefix if this condition is inverted
                $field = $is_inverted ? substr($field, 1) : $field;

                # Remove the parent prefix if applicable set a string that indicates the field applies to a parent
                $field = $is_parent_field ? str_replace('parent__', '', $field) : $field;
                $conditions_text_prefix = $is_parent_field ? '- Parent field' : '-';

                # Add descriptive text for field value equals conditions
                if (!$is_inverted and !is_array($value)) {
                    $conditions_text .= "$conditions_text_prefix `$field` must be equal to `$value_str`<br>";
                }
                # Add descriptive text for field value does not equal conditions
                elseif ($is_inverted and !is_array($value)) {
                    $conditions_text .= "$conditions_text_prefix `$field` must not be equal to `$value_str`<br>";
                }
                # Add descriptive text for field value in array conditions
                elseif (!$is_inverted and is_array($value)) {
                    $options = implode(', ', $value);
                    $conditions_text .= "$conditions_text_prefix `$field` must be one of [ $options ]<br>";
                }
                # Add descriptive text for field value not in array conditions
                elseif ($is_inverted and is_array($value)) {
                    $options = implode(', ', $value);
                    $conditions_text .= "$conditions_text_prefix `$field` must not be one of [ $options ]<br>";
                }
            }
        }

        return $conditions_text;
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
        # Format the title to include an asterisk (*) if it is required.
        $title = $this->required ? ucwords($this->verbose_name) . '*' : ucwords($this->verbose_name);

        # Add placeholder attribute for fields with a default
        if ($this->default) {
            $attributes['placeholder'] = $this->default;
        }

        # Return a static text class if this is a read-only field
        if ($this->read_only) {
            $input = new \Form_StaticText(title: $title, text: $this->value);
        }
        # Return a select input for Fields with `choices`
        elseif ($this->choices or $this->choices_callable) {
            $this->set_choices_from_callable();
            $input = new \Form_Select(
                name: $this->name,
                title: $title,
                value: $this->value,
                values: $this->verbose_choices,
                allowMultiple: $this->many,
            );
        }
        # Do not include the current value in input field if this is a sensitive field
        elseif ($this->sensitive) {
            $input = new \Form_Input(
                name: $this->name,
                title: $title,
                type: $type,
                value: $this->value ? self::SENSITIVE_MASK : '',
                attributes: $attributes,
            );
        }
        # Return a normal input for other Fields
        else {
            $input = new \Form_Input(
                name: $this->name,
                title: $title,
                type: $type,
                value: $this->many ? $this->to_internal() : $this->value,
                attributes: $attributes,
            );
        }

        # Enhance help text to include instructions on formatting
        $help_text = $this->help_text;
        $help_text .= $this->unique ? ' Value must be unique from all other objects of its kind.' : '';

        $input->setHelp($help_text);
        return $input;
    }
}
