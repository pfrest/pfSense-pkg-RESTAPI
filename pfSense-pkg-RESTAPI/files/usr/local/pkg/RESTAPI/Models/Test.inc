<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Field;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Test model object that can be used for unit testing. This model does not define any real object in pfSense
 * and should only be used for testing purposes!
 */
class Test extends Model {
    public Field $test_bool;
    public Field $test_integer;
    public Field $test_string_conditional;
    public Field $test_string_unique;
    public Field $test_string_many;
    public Field $test_string_namespace;
    public StringField $test_string_choices;
    public StringField $test_string_choices_callable;

    public function __construct(mixed $id = null, mixed $parent_id = null, array $data = [], mixed ...$options) {
        # Mark this model as a `many` model indicating multiple Test model objects can be created. This is also
        # required for the `unique` field to work.
        $this->many = true;

        # pfSense won't let us use an arbitrary path so use filter/rule instead
        $this->config_path = 'filter/rule';

        # Assign Fields and field parameters for this model.
        $this->test_bool = new BooleanField(
            default: true,
            indicates_true: 'on',
            indicates_false: 'off',
            verbose_name: 'Test Bool',
            help_text: "Demonstrates a boolean field that internally uses 'on' to indicate 'true' or 'off' 
                to indicate 'false'.",
        );
        $this->test_integer = new IntegerField(
            default: 5,
            allow_null: true,
            verbose_name: 'Test Integer',
            help_text: 'Demonstrates an integer field that defaults to 5.',
        );
        $this->test_string_conditional = new StringField(
            required: false,
            default: 'Conditions were met!',
            allow_empty: true,
            allow_null: true,
            conditions: ['test_bool' => true],
            verbose_name: 'Test String Conditional',
        );
        $this->test_string_unique = new StringField(
            required: false,
            unique: true,
            default: uniqid(),
            verbose_name: 'Test String Unique',
            help_text: "Demonstrates a string field that must be unique from all other Test model objects stored
                in configuration.",
        );
        $this->test_string_many = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: '/',
            verbose_name: 'Test String Many',
            help_text: "Demonstrates a string field that allows many values to be passed in as an array. When 'many'
                is set, values must be passed in as an array of strings. Also demonstrates that the internal version
                of this value will be the array of strings joined together by '/'.",
        );
        $this->test_string_namespace = new StringField(
            required: false,
            default: 'test namespace field value',
            internal_namespace: 'test_namespace',
            verbose_name: 'Test String Namespace',
            help_text: 'Demonstrates a string field that is nested under the namespace ',
        );
        $this->test_string_choices = new StringField(
            default: 'a',
            choices: ['a' => 'Choice A', 'b' => 'Choice B', 'c' => 'Choice C'],
            allow_empty: true,
            verbose_name: 'Test String Choices',
            help_text: 'Demonstrates a string field with static choices defined.',
        );
        $this->test_string_choices_callable = new StringField(
            default: 'x',
            choices_callable: 'test_choices_callable',
            allow_empty: true,
            verbose_name: 'Test String Choices Callable',
            help_text: 'Demonstrates a string field with static choices defined.',
        );

        # Parent base 'Model' object must be constructed very last to ensure the Model has all configured attributes.
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Defines a test 'internal_callable' method that can be assigned to this Model
     */
    public function test_internal_callable(): array {
        return [];
    }

    /**
     * Defines a test choices callable for the test_string_choices_callable field
     */
    public function test_choices_callable(): array {
        return [
            'x' => 'Choice X',
            'y' => 'Choice Y',
            'z' => 'Choice Z',
        ];
    }

    /**
     * Override the default apply method to write a file to disk. This is used for testing to determine if the
     * `apply` method did or didn't run when expected.
     */
    public function apply() {
        file_put_contents(
            filename: '/tmp/.api_models_test_apply.txt',
            data: "This file is written by \RESTAPI\Models\Test to demonstrate a specific action is taken when the Model's
                `apply()` method is executed.",
        );
    }
}
