<?php

namespace RESTAPI\Fields;

require_once 'RESTAPI/Fields/StringField.inc';
require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Field;
use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Responses\NotFoundError;

/**
 * Defines an Field object for storing interface values in the pfSense configuration. This field will also be responsible
 * for representing the interface according to the REST API `represent_interfaces_as` setting.
 */
class InterfaceField extends StringField {
    /**
     * @var string $represented_as The interface field that interfaces are represented as by this field. This value
     * directly corresponds with the REST API setting's `represent_interfaces_as` field.
     */
    public string $represented_as;

    /**
     * Defines the StringField object and sets its options.
     * @param bool $required If `true`, this field is required to have a value at all times.
     * @param bool $unique If `true`, this field must be unique from all other parent model objects. Enabling this
     * option requires the Model $context to be set AND the Model $context must have a `config_path` set.
     * @param mixed|null $default Assign a default string value to assign this Field if no value is present.
     * @param string $default_callable Defines a callable method that should be called to populate the default value
     * for this field. It is strongly encouraged to use a default callable when the default is variable and may change
     * dynamically.
     * @param array $interface_query Set query parameters to limit the scope of NetworkInterface lookups for this field.
     * Leave as an empty array to include all NetworkInterface objects in lookups.
     * @param bool $allow_interface_groups If `true`, all existing interface groups will be included as valid choices.
     * @param bool $allow_ipsec_interface If `true`, IPsec's `enc0` interface will included as a valid choice.
     * @param bool $allow_openvpn_interface If `true`, the `openvpn` interface will be included as valid choice.
     * @param bool $allow_carp_interface If `true`, all CARP unique IDs will be included as valid choices.
     * @param bool $allow_localhost_interface If `true`, the localhost interface will be included as a valid choice.
     * @param bool $allow_empty If `true`, empty strings will be allowed by this field.
     * @param bool $allow_null If `true`, null values will be allowed by this field.
     * @param array $allow_custom An array of keywords that are allowed to be used as interface values.
     * @param bool $read_only If `true`, this field can only read its value and cannot write its value to config.
     * @param bool $write_only Set to `true` to make this field write-only. This will prevent the field's current value
     * from being displayed in the representation data. This is ideal for potentially sensitive Fields like passwords,
     * keys, and hashes.
     * @param bool $representation_only Set to `true` to make this field only present in its representation form. This
     * effectively prevents the Field from being converted to an internal value which is saved to the pfSense config.
     * This should only be used for Fields that do not relate directly to a configuration value.
     * @param bool $editable Set to `false` to prevent this field's value from being changed after its initial creation.
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
        string $default_callable = '',
        public array $interface_query = [],
        public bool $allow_interface_groups = false,
        public bool $allow_ipsec_interface = false,
        public bool $allow_openvpn_interface = false,
        public bool $allow_carp_interface = false,
        public bool $allow_localhost_interface = false,
        public array $allow_custom = [],
        bool $allow_empty = false,
        bool $allow_null = false,
        bool $read_only = false,
        bool $write_only = false,
        bool $representation_only = false,
        bool $editable = true,
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
        $this->represented_as = RESTAPI\Models\RESTAPISettings::get_pkg_config()['represent_interfaces_as'];

        # Construct the parent Field object
        parent::__construct(
            required: $required,
            unique: $unique,
            default: $default,
            default_callable: $default_callable,
            choices_callable: 'get_interface_choices',
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
    }

    /**
     * Converts the field value to its representation form from it's internal pfSense configuration value.
     * @param mixed $internal_value The internal value from the pfSense configuration.
     * @return string The field value in its representation form.
     */
    public function _from_internal(mixed $internal_value): string {
        # Variables
        $interfaces = Model::get_config('interfaces', []);

        # Check if this value is an existing interface ID
        if (array_key_exists($internal_value, $interfaces)) {
            switch ($this->represented_as) {
                case 'id':
                    return $internal_value;
                case 'if':
                    return $interfaces[$internal_value]['if'];
                case 'descr':
                    return $interfaces[$internal_value]['descr'] ?: strtoupper($internal_value);
            }
        }

        # Otherwise, just return the internal value
        return $internal_value;
    }

    /**
     * Converts the field's representation value into it's internal pfSense value.
     */
    public function _to_internal(mixed $representation_value): array|null|string {
        # Query to find this interface using the interfaces representation field and value
        $interface_query = NetworkInterface::query(query_params: [$this->represented_as => $representation_value]);

        # Return the first matches' ID if found
        if ($interface_query->exists()) {
            return $interface_query->first()->id;
        }

        # As a failsafe, return the representation value if the internal value could not be found
        return $representation_value;
    }

    /**
     * Enumerates a list of every interface choice available to this Field.
     * @return array A list of every interface option available to this Field.
     */
    public function get_interface_choices(): array {
        # Variables
        $choices = [];
        $interface_query = NetworkInterface::query($this->interface_query);
        $interface_groups = Model::get_config('ifgroups/ifgroupentry', []);
        $virtual_ips = Model::get_config('virtualip/vip', []);

        # Loop through each configured interface and check for a match
        foreach ($interface_query->model_objects as $interface) {
            $represented_as = $this->represented_as;
            # Use the interface's ID if interfaces are represented by `id`
            if ($this->represented_as === 'id') {
                $choices[$interface->id] = $interface->descr->value;
            }
            # Otherwise, use the field interfaces are represented by
            else {
                $choices[$interface->$represented_as->value] = $interface->descr->value;
            }
        }

        # Include interface groups if requested
        # TODO: Replace this with a query when the InterfaceGroup model is developed
        if ($this->allow_interface_groups) {
            # Loop through all existing interface groups and add their names
            foreach ($interface_groups as $interface_group) {
                $choices[$interface_group['ifname']] = $interface_group['ifname'];
            }
        }

        # Include CARP interfaces if requested
        # TODO: Replace this with a query when the VirtualIP model is developed
        if ($this->allow_carp_interface) {
            # Loop through all existing interface groups and add their names
            foreach ($virtual_ips as $virtual_ip) {
                # Only include CARP virtual iPs with unique IDs
                if ($virtual_ip['mode'] === 'carp' and $virtual_ip['uniqid']) {
                    $uniqid = '_vip' . $virtual_ip['uniqid'];
                    $choices[$uniqid] = $uniqid;
                }
            }
        }

        # Allow the IPsec `enc0` interface if requested
        if ($this->allow_ipsec_interface) {
            $choices['enc0'] = 'IPsec';
        }

        # Allow the OpenVPN `openvpn` interface if requested
        if ($this->allow_openvpn_interface) {
            $choices['openvpn'] = 'OpenVPN';
        }

        # Allow the Localhost `lo0` interface if requested
        if ($this->allow_localhost_interface) {
            $choices['lo0'] = 'Localhost';
        }

        # Allow specified keywords if specified
        if ($this->allow_custom) {
            foreach ($this->allow_custom as $keyword) {
                $choices[$keyword] = $keyword;
            }
        }

        return $choices;
    }

    /**
     * Obtains this Fields current interface values as an array of their internal values.
     * @return array|string An array of the internal interfaces assigned to this Field (e.g. ["wan", "lan", "opt1"])
     * when this is a `many` Field, or the internal interface value as a string
     */
    public function to_internal_interfaces(): array|string {
        # Return the internal interface values for each interface value when this is a `many` Field
        if ($this->many) {
            return explode($this->delimiter, $this->to_internal());
        }

        # Otherwise, just return the internal value
        return $this->to_internal();
    }

    /**
     * Fetches the NetworkInterface Model object associated with this field's current value
     * @return Model|null The NetworkInterface Model object associated with this Field's current
     * value for non-many Fields.
     * @throws NotFoundError when a NetworkInterface object could not be found using one or more of this Field's
     * assigned values.
     */
    public function get_related_model(): Model|null {
        # Query for the NetworkInterface object that matches our current value
        $query = NetworkInterface::query([$this->represented_as => $this->value]);

        # Return the Model object if it exists
        if ($query->exists()) {
            return NetworkInterface::query([$this->represented_as => $this->value])->first();
        }

        # Otherwise return null
        return null;
    }

    /**
     * Fetches all NetworkInterface objects associated with this Field's current values when $many is enabled.
     * @return ModelSet Returns a ModelSet containing the NetworkInterface Model's associated with this Field's
     * current values, or `null` if no
     */
    public function get_related_models(): ModelSet {
        # Create a ModelSet object to store our located NetworkInterface objects in
        $modelset = new ModelSet(model_objects: []);

        # Loop through each of our current values
        foreach ($this->value as $value) {
            # Query for the NetworkInterface object that matches the current value
            $query = NetworkInterface::query([$this->represented_as => $value]);

            # Only add the related Model if it exists
            if ($query->exists()) {
                # Add the model object to the ModelSet
                $modelset->model_objects[] = $query->first();
            }
        }

        return $modelset;
    }
}
