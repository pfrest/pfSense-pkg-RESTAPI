<?php

namespace RESTAPI\Tests;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\TestCase;
use RESTAPI\Fields\InterfaceAddressField;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Models\RESTAPISettings;

/**
 * Tests represented interface address references and literal values.
 */
class APIFieldsInterfaceAddressFieldTestCase extends TestCase {
    /**
     * Checks choices, validation, round-trip conversion, and relations in every representation mode.
     */
    public function test_interface_representations(): void {
        $settings = new RESTAPISettings();
        foreach (['id', 'if', 'descr'] as $mode) {
            $settings->represent_interfaces_as->value = $mode;
            $settings->update();

            foreach (['wan', 'lan', 'opt1'] as $id) {
                $interface = NetworkInterface::query(query_params: ['id' => $id])->first();
                $represented = $mode === 'id' ? $id : $interface->$mode->value;
                foreach (['_ipv4', '_ipv6'] as $suffix) {
                    $field = $this->make_field(required: true);
                    $field->value = $represented . $suffix;
                    $this->assert_is_true(in_array($field->value, $field->choices, true));
                    $this->assert_is_true($field->validate());
                    $this->assert_equals($field->to_internal(), $id . $suffix);
                    $field->from_internal($id . $suffix);
                    $this->assert_equals($field->value, $represented . $suffix);
                    $this->assert_equals($field->get_related_model()->id, $id);
                    $this->assert_is_empty($field->get_related_models()->model_objects);
                }
            }
        }
    }

    /**
     * Checks that literal values are preserved and never identify related interfaces.
     */
    public function test_static_choices(): void {
        $settings = new RESTAPISettings();
        foreach (['id', 'if', 'descr'] as $mode) {
            $settings->represent_interfaces_as->value = $mode;
            $settings->update();
            $keywords = ['custom', 'any_ipv4', 'any_ipv6', 'localhost_ipv4', 'localhost_ipv6', 'WAN_ipv4'];
            $field = $this->make_field(allow_custom: $keywords);
            foreach ($keywords as $keyword) {
                $field->value = $keyword;
                $this->assert_is_true($field->validate());
                $this->assert_equals($field->to_internal(), $keyword);
                $field->from_internal($keyword);
                $this->assert_equals($field->value, $keyword);
                $this->assert_equals($field->get_related_model(), null);
                $this->assert_is_false(in_array($keyword . '_ipv4', $field->choices, true));
            }
        }
    }

    /**
     * Checks that bare, unknown, and malformed interface references are rejected.
     */
    public function test_invalid_references(): void {
        $settings = new RESTAPISettings();
        $settings->represent_interfaces_as->value = 'descr';
        $settings->update();
        $field = $this->make_field();

        foreach (['WAN', 'wan_ipv4', 'missing_ipv6', 'WAN_ipv5', 'WAN:ip', 'WAN_ipv4_extra', 'any_ipv4'] as $value) {
            $field->value = $value;
            $this->assert_throws_response(
                response_id: 'FIELD_INVALID_CHOICE',
                code: 400,
                callable: fn() => $field->validate(),
            );
        }
    }

    /**
     * Checks that interface queries constrain both address families and related models.
     */
    public function test_interface_query(): void {
        $settings = new RESTAPISettings();
        $settings->represent_interfaces_as->value = 'id';
        $settings->update();
        $field = $this->make_field(interface_query: ['id' => 'lan']);
        $this->assert_equals(array_keys($field->get_interface_choices()), ['lan_ipv4', 'lan_ipv6']);
        $field->value = 'wan_ipv4';
        $this->assert_equals($field->get_related_model(), null);
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: fn() => $field->validate(),
        );
    }

    /**
     * Checks many-valued conversion and HATEOAS discovery without treating static values as interfaces.
     */
    public function test_many_values(): void {
        $settings = new RESTAPISettings();
        $settings->represent_interfaces_as->value = 'descr';
        $settings->update();

        foreach ([',', null] as $delimiter) {
            $field = $this->make_field(many: true, delimiter: $delimiter, allow_custom: ['any_ipv4']);
            $field->value = ['WAN_ipv4', 'LAN_ipv6', 'any_ipv4'];
            $internal = ['wan_ipv4', 'lan_ipv6', 'any_ipv4'];
            $internal = $delimiter === null ? $internal : implode($delimiter, $internal);
            $this->assert_is_true($field->validate());
            $this->assert_equals($field->to_internal(), $internal);
            $field->from_internal($internal);
            $this->assert_equals($field->value, ['WAN_ipv4', 'LAN_ipv6', 'any_ipv4']);
            $this->assert_equals($field->get_related_model(), null);
            $related = $field->get_related_models()->model_objects;
            $this->assert_equals(count($related), 2);
            $this->assert_equals($related[0]->id, 'wan');
            $this->assert_equals($related[1]->id, 'lan');
        }
    }

    /**
     * Checks that optional null and empty values do not create interface references.
     */
    public function test_optional_values(): void {
        $field = $this->make_field(allow_null: true, allow_empty: true);
        foreach ([null, ''] as $value) {
            $field->value = $value;
            $this->assert_is_true($field->validate());
            $this->assert_equals($field->to_internal(), $value);
            $field->from_internal($value);
            $this->assert_equals($field->value, $value);
            $this->assert_equals($field->get_related_model(), null);
        }
    }

    /**
     * Constructs a field with the context and dynamic choices required for validation.
     * @param mixed ...$options Additional field options.
     * @return InterfaceAddressField The field to test.
     */
    private function make_field(mixed ...$options): InterfaceAddressField {
        $field = new InterfaceAddressField(...$options);
        $field->name = 'extaddr';
        $field->context = new RESTAPISettings();
        $field->set_choices_from_callable();
        return $field;
    }
}
