<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Models\NetworkInterface;

/**
 * Defines an interface address reference with an `_ipv4` or `_ipv6` suffix. Interface names follow
 * `represent_interfaces_as`, while configuration stores interface IDs. The inherited `allow_custom`
 * argument accepts additional literal values that are never converted or treated as references.
 */
class InterfaceAddressField extends InterfaceField {
    /**
     * Enumerates interface address choices and any allowed literal values.
     * @return array The represented choices mapped to their verbose names.
     */
    public function get_interface_choices(): array {
        $choices = [];

        foreach (parent::get_interface_choices() as $interface => $description) {
            if (in_array($interface, $this->allow_custom, true)) {
                continue;
            }

            $choices[$interface . '_ipv4'] = "$description (IPv4)";
            $choices[$interface . '_ipv6'] = "$description (IPv6)";
        }

        foreach ($this->allow_custom as $keyword) {
            $choices[$keyword] = $keyword;
        }

        if ($this->allow_empty) {
            $choices[''] = 'None';
        }

        return $choices;
    }

    /**
     * Converts the represented interface to its internal ID, retaining the address family suffix.
     * @param mixed $representation_value The represented interface address or literal value.
     * @return string|null The value suitable for storing in configuration.
     */
    public function _to_internal(mixed $representation_value): string|null {
        $interface = $this->get_interface_name($representation_value);
        if ($interface === null) {
            return $representation_value;
        }

        return parent::_to_internal($interface) . substr($representation_value, -5);
    }

    /**
     * Represents an internal interface ID according to the API settings, retaining its suffix.
     * @param mixed $internal_value The configured interface address or literal value.
     * @return string The represented value.
     */
    public function _from_internal(mixed $internal_value): string {
        $interface = $this->get_interface_name($internal_value);
        if ($interface === null) {
            return $internal_value;
        }

        return parent::_from_internal($interface) . substr($internal_value, -5);
    }

    /**
     * Obtains the interface referenced by a single address for HATEOAS discovery.
     * @return Model|null The referenced interface, or null for literal or unset values.
     */
    public function get_related_model(): Model|null {
        return $this->many ? null : $this->get_interface_model($this->value);
    }

    /**
     * Obtains the interfaces referenced by a many-valued field for HATEOAS discovery.
     * @return ModelSet The referenced interfaces, excluding literal values.
     */
    public function get_related_models(): ModelSet {
        $models = new ModelSet(model_objects: []);
        foreach ($this->many ? $this->value ?? [] : [] as $value) {
            $interface = $this->get_interface_model($value);
            if ($interface !== null) {
                $models->model_objects[] = $interface;
            }
        }

        return $models;
    }

    /**
     * Removes only a trailing address family suffix, leaving allowed literal values untouched.
     * @param mixed $value The internal or represented value.
     * @return string|null The interface name without its suffix, or null for non-reference values.
     */
    private function get_interface_name(mixed $value): string|null {
        if (!is_string($value) or in_array($value, $this->allow_custom, true)) {
            return null;
        }

        if (str_ends_with($value, '_ipv4') or str_ends_with($value, '_ipv6')) {
            return substr($value, 0, -5);
        }

        return null;
    }

    /**
     * Looks up an assigned interface using a represented address reference.
     * @param mixed $value The represented address reference.
     * @return Model|null The referenced interface, or null when no interface matches.
     */
    private function get_interface_model(mixed $value): Model|null {
        $interface = $this->get_interface_name($value);
        if ($interface === null) {
            return null;
        }

        $query = NetworkInterface::query(
            query_params: array_merge($this->interface_query, [$this->represented_as => $interface]),
        );
        return $query->exists() ? $query->first() : null;
    }
}
