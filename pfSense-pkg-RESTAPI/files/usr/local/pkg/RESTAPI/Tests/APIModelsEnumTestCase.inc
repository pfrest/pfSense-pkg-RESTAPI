<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\CARP;
use RESTAPI\Models\Enum;
use RESTAPI\Models\VirtualIP;

class APIModelsEnumTestCase extends TestCase {
    /**
     * Ensure we can calculate an enum for model fields with static choices
     */
    public function test_choices_enum(): void {
        # Execute an enumeration of the Test models' test_string_choices field which has static choices defined
        $enum = new Enum(model_name: 'Test', model_field: 'test_string_choices');
        $enum->create();
        $this->assert_equals($enum->model_name->value, 'Test');
        $this->assert_equals($enum->model_field->value, 'test_string_choices');
        $this->assert_equals($enum->choices->value, [
            'a' => 'Choice A',
            'b' => 'Choice B',
            'c' => 'Choice C',
        ]);
    }

    /**
     * Ensure we can calculate an enum for model fields with dynamic choices (choice_callable)
     */
    public function test_choices_callable_enum(): void {
        # Execute an enumeration of the Test models' test_string_choices_callable field with dynamic choices defined
        $enum = new Enum(model_name: 'Test', model_field: 'test_string_choices_callable');
        $enum->create();
        $this->assert_equals($enum->model_name->value, 'Test');
        $this->assert_equals($enum->model_field->value, 'test_string_choices_callable');
        $this->assert_equals($enum->choices->value, [
            'x' => 'Choice X',
            'y' => 'Choice Y',
            'z' => 'Choice Z',
        ]);
    }

    /**
     * Ensure we can calculate an enum for ForeignModelFields
     */
    public function test_foreign_model_field_enum(): void {
        $enum = new Enum(model_name: 'StaticRoute', model_field: 'gateway');
        $enum->create();
        $this->assert_equals($enum->model_name->value, 'StaticRoute');
        $this->assert_equals($enum->model_field->value, 'gateway');
        $this->assert_equals($enum->choices->value, [
            'WAN_DHCP' => 'WAN_DHCP',
            'WAN_DHCP6' => 'WAN_DHCP6',
        ]);
    }
}
