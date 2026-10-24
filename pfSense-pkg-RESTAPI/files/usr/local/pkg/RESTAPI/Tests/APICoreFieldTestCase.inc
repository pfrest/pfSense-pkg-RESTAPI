<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use ReflectionClass;
use RESTAPI;
use RESTAPI\Core\Field;
use RESTAPI\Core\ModelSet;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines test case for testing RESTAPI\Core\Field.
 */
class APICoreFieldTestCase extends RESTAPI\Core\TestCase {
    /**
     * Checks that a Field cannot contain a `default` value if the Field is `required`.
     */
    public function test_no_default_when_field_is_required(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_DEFAULT',
            code: 500,
            callable: function () {
                $test_field = new Field(required: true, default: '');
            },
        );
    }

    /**
     * Checks that a Field cannot contain a `default_callable` value if the Field is `required`.
     */
    public function test_no_default_callable_when_field_is_required(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_DEFAULT',
            code: 500,
            callable: function () {
                $test_field = new Field(required: true, default_callable: 'test');
            },
        );
    }

    /**
     * Checks that a Field cannot contain a `default_callable` and a `default`.
     */
    public function test_no_default_and_default_callable(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_WITH_DEFAULT_AND_DEFAULT_CALLABLE',
            code: 500,
            callable: function () {
                $test_field = new Field(default: 'test', default_callable: 'test');
            },
        );
    }

    /**
     * Checks that a Field cannot contain be `read_only` if the Field is `required`.
     */
    public function test_no_read_only_when_field_is_required(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_READ_ONLY',
            code: 500,
            callable: function () {
                $test_field = new Field(required: true, read_only: true);
            },
        );
    }

    /**
     * Checks that a Field cannot `allow_null` if the Field is `required`.
     */
    public function test_no_allow_null_when_field_is_required(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_CANNOT_BE_REQUIRED_WITH_ALLOW_NULL',
            code: 500,
            callable: function () {
                $test_field = new Field(allow_null: true, required: true);
            },
        );
    }

    /**
     * Ensures an error is thrown when a Field object has a `default` value that is not specified as a `choice`.
     */
    public function test_no_field_with_read_only_and_write_only(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_WITH_READ_ONLY_AND_WRITE_ONLY',
            code: 500,
            callable: function () {
                new Field(read_only: true, write_only: true);
            },
        );
    }

    /**
     * Checks that a Field object cannot have its `many_minimum` value set to a value higher than `many_maximum`.
     */
    public function test_no_many_minimum_greater_than_many_maximum(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_MANY_MINIMUM_GREATER_THAN_MANY_MAXIMUM',
            code: 500,
            callable: function () {
                new Field(many_minimum: 5, many_maximum: 4);
            },
        );
    }

    /**
     * Ensures Field properties are properly set when the Field object is created.
     */
    public function test_field_construction(): void {
        # Values
        $type = 'string';
        $validators = [new RESTAPI\Core\Validator()];
        $choices = ['a', 'b'];
        $unique = false;
        $allow_empty = false;
        $allow_null = false;
        $required = false;
        $default = 'a';
        $read_only = false;
        $many = false;
        $delimiter = ' ';
        $help_text = 'Test description';
        $internal_name = 'test_name';

        # Create a Field object and ensure all values were properly set during construction
        $test_field = new Field(
            type: $type,
            required: $required,
            unique: $unique,
            default: $default,
            choices: $choices,
            allow_empty: $allow_empty,
            allow_null: $allow_null,
            read_only: $read_only,
            many: $many,
            delimiter: $delimiter,
            internal_name: $internal_name,
            validators: $validators,
            help_text: $help_text,
        );

        # Ensure the values we passed in are the values that are assigned to the object
        $this->assert_equals($test_field->type, $type);
        $this->assert_equals($test_field->validators, $validators);
        $this->assert_equals($test_field->choices, $choices);
        $this->assert_equals($test_field->unique, $unique);
        $this->assert_equals($test_field->allow_empty, $allow_empty);
        $this->assert_equals($test_field->allow_null, $allow_null);
        $this->assert_equals($test_field->required, $required);
        $this->assert_equals($test_field->default, $default);
        $this->assert_equals($test_field->read_only, $read_only);
        $this->assert_equals($test_field->many, $many);
        $this->assert_equals($test_field->delimiter, $delimiter);
        $this->assert_equals($test_field->help_text, $help_text);
        $this->assert_equals($test_field->internal_name, $internal_name);
    }

    /**
     * Checks that every Field subclass supporting unique also forwards skip_unique_validation with a false default.
     */
    public function test_subclass_skip_unique_validation_argument(): void {
        foreach (glob(__DIR__ . '/../Fields/*.inc') as $file) {
            $class = 'RESTAPI\\Fields\\' . basename($file, '.inc');
            $reflection = new ReflectionClass($class);
            $parameters = [];
            foreach ($reflection->getConstructor()->getParameters() as $parameter) {
                $parameters[$parameter->getName()] = $parameter;
            }

            if (!isset($parameters['unique'])) {
                continue;
            }

            $this->assert_is_true(isset($parameters['skip_unique_validation']));
            $this->assert_equals($parameters['skip_unique_validation']->getDefaultValue(), false);
            $this->assert_equals((string) $parameters['skip_unique_validation']->getType(), 'bool');
            $options =
                $class === ForeignModelField::class
                    ? ['model_name' => 'Test', 'model_field' => 'test_string_unique']
                    : [];

            foreach ([false, true] as $unique) {
                $options['unique'] = $unique;
                unset($options['skip_unique_validation']);
                $field = new $class(...$options);
                $this->assert_equals($field->unique, $unique);
                $this->assert_is_false($field->skip_unique_validation);

                foreach ([false, true] as $skip_unique_validation) {
                    $options['skip_unique_validation'] = $skip_unique_validation;
                    $field = new $class(...$options);
                    $this->assert_equals($field->unique, $unique);
                    $this->assert_equals($field->skip_unique_validation, $skip_unique_validation);
                }
            }
        }
    }

    /**
     * Checks that skipping uniqueness permits duplicates without removing metadata or bypassing type validation.
     */
    public function test_skip_unique_validation(): void {
        $existing = new RESTAPI\Models\Test(test_string_unique: 'duplicate');
        $existing->id = 0;
        $modelset = new ModelSet(model_objects: [$existing]);
        $field = new StringField(required: true, unique: true, skip_unique_validation: true);
        $field->set_names('test_string_unique');
        $field->context = new RESTAPI\Models\Test();
        $field->value = 'duplicate';
        $this->assert_is_true($field->validate(modelset: $modelset));
        $this->assert_is_true($field->unique);

        $field->skip_unique_validation = false;
        $this->assert_throws_response(
            response_id: 'FIELD_MUST_BE_UNIQUE',
            code: 400,
            callable: fn() => $field->validate(modelset: $modelset),
        );

        $field->skip_unique_validation = true;
        $field->value = 123;
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_TYPE',
            code: 400,
            callable: fn() => $field->validate(modelset: $modelset),
        );
    }

    /**
     * Checks that a unique Field is required to have a parent Model context assigned.
     */
    public function test_context_required_with_unique(): void {
        # Ensure an error raised when a Field is `unique`, but no parent model context assigned during validation.
        $this->assert_throws_response(
            response_id: 'FIELD_UNIQUE_WITH_NO_CONTEXT',
            code: 500,
            callable: function () {
                $test_field = new Field(unique: true);
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that Fields must have names assigned.
     */
    public function test_field_requires_name(): void {
        # Ensure the field has a name when it's validated.
        $this->assert_throws_response(
            response_id: 'FIELD_UNNAMED',
            code: 500,
            callable: function () {
                $test_field = new Field(required: true);
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that Field names cannot contain double underscores. Double underscores are reserved for queries.
     */
    public function test_field_name_cannot_contain_double_underscore(): void {
        # Ensure an error is thrown for a Field name with double underscoress
        $this->assert_throws_response(
            response_id: 'FIELD_NAME_WITH_DOUBLE_UNDERSCORE_NOT_ALLOWED',
            code: 500,
            callable: function () {
                $test_field = new Field(required: true);
                $test_field->name = 'name__with__double__underscores';
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that a Field actually requires a value when the Field is `required`.
     */
    public function test_require_value_when_field_is_required(): void {
        # Ensure required Fields must have a value set
        $this->assert_throws_response(
            response_id: 'FIELD_IS_REQUIRED',
            code: 400,
            callable: function () {
                $test_field = new Field(required: true);
                $test_field->name = 'test_field';
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that a Field that is not required must have a default or default_callable value.
     */
    public function test_require_default_when_field_is_not_required(): void {
        # Ensure required Fields must have a value set
        $this->assert_throws_response(
            response_id: 'FIELD_NEEDS_DEFAULT',
            code: 500,
            callable: function () {
                $test_field = new Field(required: false);
                $test_field->name = 'test_field';
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that the field cannot contain `choices` when it is `read_only`.
     */
    public function test_no_choices_when_field_is_read_only(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_NO_CHOICES_WITH_READ_ONLY',
            code: 500,
            callable: function () {
                $test_field = new Field(type: 'string', choices: ['test'], read_only: true);
                $test_field->name = 'test_field';
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that a validation error occurs if there are less array entries than the `many_minimum` requires.
     */
    public function test_field_many_minimum(): void {
        $this->assert_throws_response(
            response_id: 'LENGTH_VALIDATOR_MINIMUM_CONSTRAINT',
            code: 400,
            callable: function () {
                # Create a model to use for testing
                $test_model = new RESTAPI\Models\Test();

                # Create a field to test with and ensure the test model is assigned as its parent context
                $test_field = new Field(type: 'string', required: true, many: true, many_minimum: 1);
                $test_field->name = 'test_field';
                $test_field->value = [];
                $test_field->context = $test_model;
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that a validation error occurs if there are more array entries than the `many_maximum` allows.
     */
    public function test_field_many_maximum(): void {
        $this->assert_throws_response(
            response_id: 'LENGTH_VALIDATOR_MAXIMUM_CONSTRAINT',
            code: 400,
            callable: function () {
                # Create a model to use for testing
                $test_model = new RESTAPI\Models\Test();

                # Create a field to test with and ensure the test model is assigned as its parent context
                $test_field = new Field(type: 'string', required: true, many: true, many_maximum: 3);
                $test_field->name = 'test_field';
                $test_field->value = ['too', 'many', 'values', '!'];
                $test_field->context = $test_model;
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that the `many_minimum` value is automatically changed to `0` when `allow_empty` is `true`.
     */
    public function test_field_many_minimum_set_according_to_allow_empty(): void {
        # Ensure that the `many_minimum` value is automatically changed to 0 when `allow_empty` is enabled
        $test_field = new Field(type: 'string', required: true, allow_empty: true, many: true, many_minimum: 3);
        $this->assert_equals($test_field->many_minimum, 0);
    }

    /**
     * Checks that the field's parent model context is a `many` model if the field is `unique`.
     */
    public function test_context_has_many_when_field_is_unique(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_UNIQUE_WITHOUT_MANY_MODEL_CONTEXT',
            code: 500,
            callable: function () {
                # Create a model to use for testing
                $test_model = new RESTAPI\Models\Test();
                $test_model->many = false;

                # Create a field to test with and ensure the test model is assigned as its parent context
                $test_field = new Field(type: 'string', required: true, unique: true);
                $test_field->name = 'test_field';
                $test_field->value = 'test_value';
                $test_field->context = $test_model;
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that a Field with `conditions` is set to `null` when any of its conditions are not met.
     */
    public function test_unmet_conditions_assign_null(): void {
        # Create a Test Model and assign its values
        $test_model = new RESTAPI\Models\Test();
        $test_model->test_bool->value = false;
        $test_model->test_string_conditional->value = 'Set me to null if conditions fail!';
        $test_model->test_string_conditional->conditions = ['test_bool' => true];
        $test_model->validate();

        # Ensure the `test_string_conditional` field's value was set to null after validating
        $this->assert_equals($test_model->test_string_conditional->value, null);
    }

    /**
     * Checks that a conditional Field is properly validated if all it's conditions are met.
     */
    public function test_met_conditions_allow_validation(): void {
        # Create a Test Model and assign its values
        $test_model = new RESTAPI\Models\Test();
        $test_model->test_bool->value = true;
        $test_model->test_string_conditional->value = 'Conditions were met!';
        $test_model->test_string_conditional->conditions = [
            'test_bool' => ["condition is met if one of these values match test_bool's value", true],
        ];
        $test_model->validate();

        # Ensure the `test_string_conditional` field's value was set to the value we specified above
        $this->assert_equals($test_model->test_string_conditional->value, 'Conditions were met!');
    }

    /**
     * Checks that `unique` Fields successfully require the field to be unique from all other objects in the
     * parent Model context's `config_path`.
     */
    public function test_field_unique_constraint(): void {
        # Create a test object
        $test_model = new RESTAPI\Models\Test();
        $test_model->test_string_unique->value = 'This must be unique';
        $test_model->create();

        # Try to create another object with the same value for the unique field and ensure it throws an error
        $this->assert_throws_response(
            response_id: 'FIELD_MUST_BE_UNIQUE',
            code: 400,
            callable: function () {
                $non_unique_test_model = new RESTAPI\Models\Test();
                $non_unique_test_model->test_string_unique->value = 'This must be unique';
                $non_unique_test_model->create();
            },
        );

        # Delete the test object
        $test_model->delete();
    }

    /**
     * Checks that an error is thrown if a value is set with a type that doesn't match this field.
     */
    public function test_field_type_constraint(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_TYPE',
            code: 400,
            callable: function () {
                $test_field = new Field(type: 'string', required: true);
                $test_field->name = 'test_field';
                $test_field->value = 1;
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that empty values are not allowed unless `allow_empty` is set.
     */
    public function test_no_empty_values_without_allow_empty(): void {
        # Check for string
        $this->assert_throws_response(
            response_id: 'FIELD_EMPTY_NOT_ALLOWED',
            code: 400,
            callable: function () {
                $test_field = new Field(type: 'string', required: true, allow_empty: false);
                $test_field->name = 'test_field';
                $test_field->value = '';
                $test_field->validate();
            },
        );

        # Check an array (field has `many` set).
        $this->assert_throws_response(
            response_id: 'LENGTH_VALIDATOR_MINIMUM_CONSTRAINT',
            code: 400,
            callable: function () {
                $test_field = new Field(
                    type: 'string',
                    required: true,
                    allow_empty: false,
                    many: true,
                    many_minimum: 1,
                );
                $test_field->name = 'test_field';
                $test_field->value = [];
                $test_field->validate();
            },
        );
    }

    /**
     * Check that null values are allowed if `allow_null` is set.
     */
    public function test_allow_null_values_with_allow_null(): void {
        $test_field = new Field(type: 'string', required: false, allow_null: true);
        $test_field->name = 'test_field';
        $test_field->value = null;
        $test_field->validate();
    }

    /**
     * Checks that assigned Validators are run against the Field value during validation.
     */
    function test_validators_check_values(): void {
        # Check against single value
        $this->assert_throws_response(
            response_id: 'LENGTH_VALIDATOR_MAXIMUM_CONSTRAINT',
            code: 400,
            callable: function () {
                $test_field = new Field(
                    type: 'string',
                    required: true,
                    validators: [new RESTAPI\Validators\LengthValidator(minimum: 0, maximum: 5)],
                );
                $test_field->name = 'test_field';
                $test_field->value = 'THIS_VALUE_EXCEEDS_OUR_MAX_LENGTH';
                $test_field->validate();
            },
        );

        # Check against many value
        $this->assert_throws_response(
            response_id: 'LENGTH_VALIDATOR_MAXIMUM_CONSTRAINT',
            code: 400,
            callable: function () {
                $test_field = new Field(
                    type: 'string',
                    required: true,
                    many: true,
                    validators: [new RESTAPI\Validators\LengthValidator(minimum: 0, maximum: 5)],
                );
                $test_field->name = 'test_field';
                $test_field->value = ['OK', 'THIS_VALUE_EXCEEDS_OUR_MAX_LENGTH'];
                $test_field->validate();
            },
        );
    }

    /**
     * Checks that the default to_internal() methods converts values to proper types AND joins `many` enabled Fields
     * back into strings using the specified `delimiter`.
     */
    public function test_default_to_internal_conversion(): void {
        # Check for single values
        $test_field = new Field(type: 'integer', required: true);
        $test_field->name = 'test_field';
        $test_field->value = 0;
        $this->assert_equals($test_field->to_internal(), '0');

        # Check for many values
        $test_field = new Field(type: 'integer', required: true, many: true, delimiter: ' ');
        $test_field->name = 'test_field';
        $test_field->value = [0, 1, 2, 3, 4, 5];

        # Ensure the internal value of `many` Fields is joined back into a string using the `delimiter`
        $this->assert_equals($test_field->to_internal(), implode($test_field->delimiter, $test_field->value));

        # Loop through each set value and ensure it is a string
        foreach ($test_field->to_internal() as $internal_value) {
            $this->assert_type($internal_value, 'string');
        }
    }

    /**
     * Checks that the default from_internal() method throws an error for not being able to represent the field.
     */
    public function test_default_from_internal_conversion(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_CANNOT_REPRESENT_INTERNAL_VALUE',
            code: 500,
            callable: function () {
                $test_field = new Field(type: 'integer', required: true);
                $test_field->name = 'test_field';
                $test_field->from_internal('test');
            },
        );
    }

    /**
     * Checks that the `to_openapi_property` correctly converts this Field to an OpenAPI property.
     */
    public function test_field_to_openapi_property(): void {
        # Ensure the `help_text` value is added as the OpenAPI property's `description`
        $test_field = new Field(type: 'string', help_text: 'test');
        $this->assert_is_true(
            $test_field->to_openapi_property()['description'] === 'test<br>' . $test_field->get_conditions_text(),
        );

        # Ensure the `allow_null` value is added as the OpenAPI property's `nullable`
        $test_field = new Field(type: 'string', allow_null: true);
        $this->assert_is_true($test_field->to_openapi_property()['nullable'] === true);

        # Ensure the `read_only` value is added as the OpenAPI property's `readOnly`
        $test_field = new Field(type: 'string', read_only: true);
        $this->assert_is_true($test_field->to_openapi_property()['readOnly'] === true);

        # Ensure the `write_only` value is added as the OpenAPI property's `writeOnly`
        $test_field = new Field(type: 'string', write_only: true);
        $this->assert_is_true($test_field->to_openapi_property()['writeOnly'] === true);

        # Ensure the `default` isn't present in the OpenAPI property if no default is assigned
        $test_field = new Field(type: 'string');
        $this->assert_is_false(array_key_exists('default', $test_field->to_openapi_property()));

        # Ensure the `default` value is added as the OpenAPI property's `default`
        $test_field = new Field(type: 'string', default: 'test');
        $this->assert_is_true($test_field->to_openapi_property()['default'] === 'test');

        # Ensure `many` Fields represent the Field as an array of items in the OpenAPI property
        $test_field = new Field(type: 'string', choices: ['a', 'b'], many: true, many_minimum: 5, many_maximum: 14);
        $this->assert_is_true($test_field->to_openapi_property()['type'] === 'array');
        $this->assert_is_true($test_field->to_openapi_property()['minItems'] === 5);
        $this->assert_is_true($test_field->to_openapi_property()['maxItems'] === 14);
        $this->assert_is_true($test_field->to_openapi_property()['items']['type'] === 'string');
        $this->assert_is_true($test_field->to_openapi_property()['items']['enum'] === ['a', 'b']);

        # Ensure the `type` value is added as the OpenAPI property's `type`
        $test_field = new Field(type: 'string', choices: ['a', 'b']);
        $this->assert_is_true($test_field->to_openapi_property()['type'] === 'string');
        $this->assert_is_true($test_field->to_openapi_property()['enum'] === ['a', 'b']);
    }

    /**
     * Checks that reserved names cannot be assigned to Field objects.
     */
    public function test_field_cannot_use_reserved_name(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_CANNOT_USE_RESERVED_NAME',
            code: 500,
            callable: function () {
                $field = new Field();
                $field->set_names('_links');
            },
        );
    }

    /**
     * Checks that a Field's default property is dynamically populated by default_callable if set.
     */
    public function test_default_callable_populates_default(): void {
        # Call the Field object's __invoke method as the default_callable to test (which returns the field's value)
        $field = new Field(default_callable: '__invoke');
        $field->value = 'test';
        $this->assert_equals($field->get_default(), $field->value);
    }

    /**
     * Checks that a Field with allow_empty set to true automatically sets many_minimum to 0 and vice versa.
     */
    public function test_allow_empty_sets_many_minimum(): void {
        # Ensure that the `many_minimum` value is automatically changed to 0 when `allow_empty` is enabled
        $test_field = new Field(type: 'string', required: true, allow_empty: true, many: true, many_minimum: 3);
        $this->assert_equals($test_field->many_minimum, 0);

        # Ensure that the `allow_empty` value is automatically changed to true when `many_minimum` is set to 0
        $test_field = new Field(type: 'string', required: true, allow_empty: false, many: true, many_minimum: 0);
        $this->assert_equals($test_field->allow_empty, true);
    }
}
