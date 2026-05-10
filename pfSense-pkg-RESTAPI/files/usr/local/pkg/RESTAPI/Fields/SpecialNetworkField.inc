<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/autoloader.inc';
require_once 'RESTAPI/Fields/InterfaceField.inc';

use RESTAPI\Core\Field;
use RESTAPI\Models\FirewallAlias;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Field that adds validation and representation for address and network Fields that support special
 * networks like interfaces, aliases, etc. This is similar to FilterAddressField except the values do not get
 * nested into arrays internally.
 */
class SpecialNetworkField extends InterfaceField {
    /**
     * Defines the SpecialNetworkField object and sets its options.
     * @param bool $required If `true`, this field is required to have a value at all times.
     * @param bool $unique If `true`, this field must be unique from all other parent model objects. Enabling this
     * option requires the Model $context to be set AND the Model $context must have a `config_path` set.
     * @param mixed|null $default Assign a default string value to assign this Field if no value is present.
     * execute to populate choices for this field. This callable must be a method assigned on this Field object OR the
     * parent Model object that returns an array of valid choices in the same format as $choices. This is helpful when
     * choices are dynamic and must be populated at runtime instead of pre-determined sets of values.
     * @param bool $allow_ipaddr Allow this field's value to be a valid IP address
     * @param bool $allow_subnet Allow this field's value to be a valid subnet CIDR
     * @param bool $allow_alias Allow this field's value to be an existing host or network firewall alias
     * @param bool $allow_interface Allow this field's value to be an existing interface
     * @param bool $allow_interface_ip Allow this field's value to be suffixed with the `:ip` modifier. Ths modifier
     * indicates the filter should use the assigned interface's current IP address. This is only applicable if
     * $allow_interface is `true` and the assigned value is an existing interface.
     * @param bool $allow_interface_groups Allow this field's value to be an existing interface group name.
     * @param bool $allow_self Allow this field's value to be set to `(self)`. This is used to indicate the address
     * is any of the current firewall's existing IP addresses.
     * @param bool $allow_l2tp Allow this field's value to be `l2tp`. This is used to indicate the address is any
     * L2TP client.
     * @param bool $allow_pppoe Allow this field's value to be `pppoe`. This is used to indicate the address is any
     * PPPoE client.
     * @param array $allowed_keywords Additional keyword strings that should be considered valid.
     * @param bool $allow_empty If `true`, empty strings will be allowed by this field.
     * @param bool $allow_null If `true`, null values will be allowed by this field.
     * @param array $interface_query Assigns query parameters to limit which interfaces should be accepted by this
     * Field. This works for interface values with and without the :ip modifier.
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
        mixed $default = null,
        public bool $allow_ipaddr = true,
        public bool $allow_subnet = true,
        public bool $allow_alias = true,
        public bool $allow_interface = true,
        public bool $allow_interface_ip = true,
        public bool $allow_interface_groups = true,
        public bool $allow_self = true,
        public bool $allow_l2tp = true,
        public bool $allow_pppoe = true,
        public array $allowed_keywords = [],
        bool $allow_empty = false,
        bool $allow_null = false,
        array $interface_query = [],
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
        # Construct the parent object for this field
        parent::__construct(
            required: $required,
            unique: $unique,
            default: $default,
            interface_query: $interface_query,
            allow_interface_groups: $allow_interface_groups,
            allow_empty: $allow_empty,
            allow_null: $allow_null,
            read_only: $read_only,
            write_only: $write_only,
            representation_only: $representation_only,
            editable: $editable,
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

        # To prevent the need to add redundant documentation to the help text, append a standard set of help text.
        $this->help_text .=
            ' Valid value options are: ' .
            $this->get_valid_opts_str() .
            '. For interface values, the `:ip` ' .
            " modifier can be appended to the value to use the interface's IP address instead of its entire subnet.";

        # Because this Field extends the InterfaceField class, we need to overwrite the parent `choices_callable`.
        # Otherwise, this Field will throw a "not a valid choice" error for non interface values.
        $this->choices_callable = '';
    }

    /**
     * Ensure any value assigned to this Field is allowed by the current field settings.
     * @param mixed $value The incoming value to be validated.
     */
    public function validate_extra(mixed $value): void {
        # Allow this value if it is an IPv4 address and this Field allows IP address values
        if ($this->allow_ipaddr and is_ipaddrv4($value)) {
            $this->labels[] = 'is_ipaddrv4';
            $this->labels[] = 'is_ipaddr';
            return;
        }

        # Allow this value if it is an IPv6 address and this Field allows IP address values
        if ($this->allow_ipaddr and is_ipaddrv6($value)) {
            $this->labels[] = 'is_ipaddrv6';
            $this->labels[] = 'is_ipaddr';
            return;
        }

        # Allow this value if it is a v4 subnet and this Field allows subnet values
        if ($this->allow_subnet and is_subnetv4($value)) {
            $this->labels[] = 'is_subnetv4';
            $this->labels[] = 'is_subnet';
            return;
        }

        # Allow this value if it is a v6 subnet and this Field allows subnet values
        if ($this->allow_subnet and is_subnetv6($value)) {
            $this->labels[] = 'is_subnetv6';
            $this->labels[] = 'is_subnet';
            return;
        }

        # Allow this value if it is a network or host alias and this Field allows alias values
        if ($this->allow_alias and FirewallAlias::query(['type__except' => 'port', 'name' => $value])->exists()) {
            $this->labels[] = 'is_alias';
            return;
        }

        # Allow this value if it is literal `(self)` and self values are allowed. `(self)` refers to any IP assigned to
        # this firewall.
        if ($this->allow_self and $value === '(self)') {
            return;
        }

        # Allow this value if it is literal `l2tp` and L2TP values are allowed
        if ($this->allow_l2tp and $value === 'l2tp') {
            return;
        }

        # Allow this value if it is literal `pppoe` and PPPoE values are allowed
        if ($this->allow_pppoe and $value === 'pppoe') {
            return;
        }

        # Allow this value if it is an allowed keyword
        if ($this->allowed_keywords and in_array($value, $this->allowed_keywords)) {
            return;
        }

        # Remove the trailing :ip value if interface IP values are allowed and set, so we can continue validating.
        if ($this->allow_interface_ip and str_ends_with($value, ':ip')) {
            # Remove the :ip modifier, so we can ensure the interface exists
            $value = substr($value, 0, -3);

            # Allow this value if the interface exists
            if (in_array($value, array_keys($this->get_interface_choices()))) {
                return;
            }
        }

        # Allow this value if it is an existing interface and interface values are allowed
        if ($this->allow_interface and in_array($value, array_keys($this->get_interface_choices()))) {
            $this->labels[] = 'is_interface';
            return;
        }

        # If we did not match any of the previous conditions, the value is invalid. Throw a ValidationError.
        throw new ValidationError(
            message: "Field `$this->name` value `$value` is invalid. Valid options are: " . $this->get_valid_opts_str(),
            response_id: 'SPECIAL_NETWORK_FIELD_INVALID',
        );
    }

    /**
     * Format a string of valid value options this Field's value can be. This is used to dynamically generate
     * help messages depending on this Field's current settings.
     * @return string A string representation of values that are considered valid for this Field.
     */
    private function get_valid_opts_str(): string {
        # Piece together our options in string format using this Field's current settings
        $opts_str = $this->allow_interface ? 'an existing interface, ' : '';
        $opts_str .= $this->allow_ipaddr ? 'an IP address, ' : '';
        $opts_str .= $this->allow_subnet ? 'a subnet CIDR, ' : '';
        $opts_str .= $this->allow_alias ? 'an existing alias, ' : '';
        $opts_str .= $this->allow_self ? '`(self)`, ' : '';
        $opts_str .= $this->allow_l2tp ? '`l2tp`, ' : '';
        $opts_str .= $this->allow_pppoe ? '`pppoe`, ' : '';

        # Add any specified keywords
        foreach ($this->allowed_keywords as $keyword) {
            $opts_str .= "`$keyword`, ";
        }

        # Return the value but replace the trailing ", " characters
        return substr($opts_str, 0, -2);
    }

    /**
     * Converts the represented filter address value into internal pfSense value.
     * @param mixed $representation_value The value to convert into it's internal form.
     * @return array|null|string The internal value(s) suitable for writing to the pfSense configuration.
     */
    public function _to_internal(mixed $representation_value): string {
        # Keep track if the interface IP modifier was set
        $if_ip_modifier_set = false;

        # Remove the suffixed `:ip` if present and track that it was initially set
        if (str_ends_with($representation_value, ':ip')) {
            $representation_value = substr($representation_value, 0, -3);
            $if_ip_modifier_set = true;
        }

        # Check if the value is an existing interface, if so, convert it's internal ID
        $if_query = NetworkInterface::query([$this->represented_as => $representation_value]);
        if ($if_query->exists()) {
            $internal_value = $if_query->first()->id;
        }
        # Otherwise, just use the representation value as is
        else {
            $internal_value = $representation_value;
        }

        # Append `ip` if the IP modifier was originally set
        $internal_value .= $if_ip_modifier_set ? 'ip' : '';

        return $internal_value;
    }

    /**
     * Converts the stored internal config value to its Base64 decoded representation value.
     * @param string $internal_value The internal config value to convert to a representation value.
     * @throws ServerError When the child Field class does not override this method.
     */
    public function _from_internal(mixed $internal_value): string {
        # Assign a variable to use when piecing back together our representation value
        $representation_value = '';

        # If there is a `network` internal value that ends with `ip`, represent it as `:ip`
        if (str_ends_with($internal_value, 'ip')) {
            $representation_value .= parent::_from_internal(substr($internal_value, 0, -2)) . ':ip';
            return $representation_value;
        }

        # When loading the regular internal value (from InterfaceField), if the value is different from the
        # internal value, it means it was an interface that got converted from its internal ID. So return it.
        $representation_value = parent::_from_internal($internal_value);
        if ($representation_value !== $internal_value) {
            return $representation_value;
        }

        # Otherwise, just return the internal value as it is (interface ID, alias, IP, subnet, keyword, etc.)
        return $internal_value;
    }
}
