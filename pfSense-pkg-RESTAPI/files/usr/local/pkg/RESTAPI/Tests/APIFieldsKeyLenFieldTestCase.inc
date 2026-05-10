<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Fields\KeyLenField;

class APIFieldsKeyLenFieldTestCase extends TestCase {
    /**
     * Checks that KeyLenField always includes a NumericRangeValidator.
     */
    public function test_key_len_field_has_numeric_range_validator(): void {
        $field = new KeyLenField();
        $this->assert_equals(get_class($field->validators[0]), 'RESTAPI\Validators\NumericRangeValidator');
    }

    /**
     * Checks that `_from_internal()` converts a numeric string to an integer.
     */
    public function test_from_internal_numeric_string_returns_integer(): void {
        $field = new KeyLenField();
        $field->from_internal('256');
        $this->assert_equals($field->value, 256);
        $this->assert_is_true(is_int($field->value));
    }

    /**
     * Checks that `_from_internal()` converts the 'auto' keyword to 0.
     */
    public function test_from_internal_auto_returns_zero(): void {
        $field = new KeyLenField();
        $field->from_internal('auto');
        $this->assert_equals($field->value, 0);
    }

    /**
     * Checks that `_from_internal()` converts an empty string to null.
     */
    public function test_from_internal_empty_string_returns_null(): void {
        $field = new KeyLenField(allow_null: true);
        $field->from_internal('');
        $this->assert_equals($field->value, null);
    }

    /**
     * Checks that `_from_internal()` throws a ServerError for non-numeric, non-'auto' values.
     */
    public function test_from_internal_invalid_value_throws_server_error(): void {
        $this->assert_throws_response(
            response_id: 'KEYLEN_FIELD_WITH_NON_INTEGER_INTERNAL_VALUE',
            code: 500,
            callable: function () {
                $field = new KeyLenField();
                $field->from_internal('not_valid');
            },
        );
    }

    /**
     * Checks that `keylen` can be assigned the value 0 and that `to_internal()` converts it to 'auto'.
     */
    public function test_to_internal_zero_returns_auto(): void {
        $field = new KeyLenField(default: 0);
        $field->value = 0;
        $this->assert_equals($field->value, 0);
        $this->assert_equals($field->to_internal(), 'auto');
    }

    /**
     * Checks that `to_internal()` passes non-zero integers through normally.
     */
    public function test_to_internal_non_zero_passes_through(): void {
        $field = new KeyLenField(default: 128);
        $field->value = 128;
        $this->assert_equals($field->to_internal(), '128');
    }

    /**
     * Checks that `to_openapi_property()` includes `minimum` and `maximum` for non-many fields.
     */
    public function test_to_openapi_property_includes_min_max(): void {
        $field = new KeyLenField(minimum: 64, maximum: 512);
        $property = $field->to_openapi_property();
        $this->assert_equals($property['minimum'], 64);
        $this->assert_equals($property['maximum'], 512);
    }

    /**
     * Checks that `to_openapi_property()` includes `minimum` and `maximum` under `items` for many fields.
     */
    public function test_to_openapi_property_includes_min_max_for_many(): void {
        $field = new KeyLenField(many: true, minimum: 64, maximum: 512);
        $property = $field->to_openapi_property();
        $this->assert_equals($property['items']['minimum'], 64);
        $this->assert_equals($property['items']['maximum'], 512);
    }
}
