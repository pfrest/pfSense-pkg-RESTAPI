<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\FilterNameValidator;

/**
 * Defines a Model that interacts with firewall aliases.
 */
class FirewallAlias extends Model {
    public StringField $name;
    public StringField $type;
    public StringField $descr;
    public StringField $address;
    public StringField $detail;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        $this->config_path = 'aliases/alias';
        $this->subsystem = 'aliases';
        $this->many = true;

        # TODO: Set `referenced_by` for this field once Model dependents like firewall rules are developed
        $this->name = new StringField(
            required: true,
            unique: true,
            editable: false,
            maximum_length: 31,
            validators: [new FilterNameValidator()],
            verbose_name: 'Name',
            help_text: 'Sets the name for the alias. This name must be unique from all other aliases.',
        );
        $this->type = new StringField(
            required: true,
            choices: ['host', 'network', 'port'],
            verbose_name: 'Type',
            help_text: "Sets the type of alias this object will be. This directly impacts what values can be
                specified in the `address` field.",
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'Sets a description to help specify the purpose or contents of the alias.',
        );
        $this->address = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            many_maximum: 0, // Do not enforce a maximum number of entries
            delimiter: ' ',
            verbose_name: 'Address',
            help_text: "Sets the host, network or port entries for the alias. When `type` is set to `host`, each
                entry must be a valid IP address or FQDN. When `type` is set to `network`, each entry must be a valid
                network CIDR or FQDN. When `type` is set to `port`, each entry must be a valid port or port range. You
                may also specify an existing alias's `name` as an entry to created nested aliases.",
        );
        $this->detail = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            many_maximum: 0, // Do not enforce a maximum number of entries
            delimiter: '||',
            verbose_name: 'Detail',
            help_text: "Sets descriptions for each alias `address`. Values must match the order of the `address` 
                value it relates to. For example, the first value specified here is the description for the first
                value specified in the `address` field. This value cannot contain ",
        );

        # Needs to be constructed after Fields are defined
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation to the `name` field.
     * @param string $name The alias name to validate.
     * @returns string The validated $name to set.
     * @throws ValidationError When the `name` value is invalid.
     */
    public function validate_name(string $name): string {
        # Throw an exception if this name is not a valid alias name
        if (!is_validaliasname($name)) {
            throw new ValidationError(
                message: "Invalid firewall alias name '$name'.",
                response_id: 'INVALID_FIREWALL_ALIAS_NAME',
            );
        }

        return $name;
    }

    /**
     * Adds custom validation to the `address` field.
     * @param string The incoming value to be validated.
     * @returns string The validated value to set.
     * @throws ValidationError When the `address` value is invalid.
     */
    public function validate_address(string $address): string {
        # Variables
        $aliases = $this->read_all();
        $type = $this->type->value;

        # Ensure value is a port, port range or port alias when `type` is `port`
        $port_alias_q = $aliases->query(name: $address, type: 'port');
        if ($type === 'port' and !is_port_or_range($address) and !$port_alias_q->exists()) {
            throw new ValidationError(
                message: "Port alias 'address' value '$address' is not a valid port, range, or alias.",
                response_id: 'INVALID_PORT_ALIAS_ADDRESS',
            );
        }

        # Ensure value is an IP, FQDN or alias when `type` is `host`
        $host_alias_q = $aliases->query(name: $address, type__except: 'port');
        if ($type === 'host' and !is_ipaddr($address) and !is_fqdn($address) and !$host_alias_q->exists()) {
            throw new ValidationError(
                message: "Host alias 'address' value '$address' is not a valid IP, FQDN, or alias.",
                response_id: 'INVALID_HOST_ALIAS_ADDRESS',
            );
        }

        # Ensure value is a CIDR, FQDN or non-port alias when `type` is `network`
        $network_alias_q = $aliases->query(name: $address, type__except: 'port');
        if ($type === 'network' and !is_subnet($address) and !is_fqdn($address) and !$network_alias_q->exists()) {
            throw new ValidationError(
                message: "Network alias 'address' value '$address' is not a valid CIDR, FQDN, or alias.",
                response_id: 'INVALID_NETWORK_ALIAS_ADDRESS',
            );
        }

        return $address;
    }

    /**
     * Adds custom validation to `detail` field
     * @param string $detail The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When the `detail` value is invalid.
     */
    public function validate_detail(string $detail): string {
        # Throw an error if there are more `detail` items than `address` items
        if (count($this->detail->value) > count($this->address->value)) {
            throw new ValidationError(
                message: "Aliases cannot contain more 'detail' items than 'address' items.",
                response_id: 'TOO_MANY_ALIAS_DETAILS',
            );
        }

        # Automatically generate detail items if there are less details than addresses
        while (count($this->detail->value) < count($this->address->value)) {
            $this->detail->value[] = 'Entry added ' . date('r');
        }

        return $detail;
    }

    /**
     * Set the apply method to reload the firewall filter after successfully validating and writing firewall alias
     * changes to config.
     */
    public function apply() {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
