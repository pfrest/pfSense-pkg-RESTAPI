<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Fields\UIDField;

class APIFieldsUIDFieldTestCase extends TestCase {
    /**
     * Checks that the UIDField defaults to a uniqid
     */
    public function test_default(): void {
        $field1 = new UIDField();
        $field2 = new UIDField();
        $this->assert_not_equals($field1->get_default(), $field2->get_default());
    }

    /**
     * Checks that the 'prefix' attributes works as intended.
     */
    public function test_prefix(): void {
        $field = new UIDField(prefix: 'test-');
        $this->assert_is_true(str_starts_with($field->get_default(), 'test-'));
    }

    /**
     * Checks that the generate_uid method correctly generates a UID
     */
    public function test_generate_uid(): void {
        $field = new UIDField();
        $this->assert_is_not_empty($field->generate_uid());
        $this->assert_not_equals($field->generate_uid(), $field->generate_uid());
    }
}
