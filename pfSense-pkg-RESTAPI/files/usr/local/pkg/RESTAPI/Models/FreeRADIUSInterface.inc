<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model that represents FreeRADIUS Interfaces
 */
class FreeRADIUSInterface extends Model {
    public StringField $addr;
    public PortField $port;
    public StringField $type;
    public StringField $ip_version;
    public StringField $description;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->packages = ['pfSense-pkg-freeradius3'];
        $this->package_includes = ['freeradius.inc'];
        $this->config_path = 'installedpackages/freeradiusinterfaces/config';
        $this->many = true;
        $this->always_apply = true;
        $this->unique_together_fields = ['addr', 'port', 'ip_version'];
        $this->verbose_name = 'FreeRADIUS Interface';

        # Set model fields
        $this->addr = new StringField(
            required: true,
            internal_name: 'varinterfaceip',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_keywords: ['*'])],
            verbose_name: 'Address',
            help_text: 'The IP address of the listening interface. If you choose * then it means all interfaces.',
        );
        $this->port = new PortField(
            required: false,
            default: '1812',
            allow_alias: false,
            allow_range: false,
            internal_name: 'varinterfaceport',
            verbose_name: 'Port',
            help_text: 'The port number of the listening interface. Different interface types need different ports.',
        );
        $this->ip_version = new StringField(
            required: true,
            choices: ['ipaddr', 'ipv6addr'],
            internal_name: 'varinterfaceipversion',
            verbose_name: 'IP Version',
            help_text: 'The IP version of the listening interface.',
        );
        $this->type = new StringField(
            required: false,
            default: 'auth',
            choices: ['auth', 'acct', 'proxy', 'detail', 'status', 'coa'],
            internal_name: 'varinterfacetype',
            verbose_name: 'Type',
            help_text: 'The type of the listening interface: Authentication/Accounting.',
        );
        $this->description = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            validators: [
                new RegexValidator(
                    pattern: "/^[a-zA-Z0-9 _,.;:+=()-]*$/",
                    error_msg: 'Value contains invalid characters.',
                ),
            ],
            verbose_name: 'Description',
            help_text: 'The description for this interface.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Perform extra validation on the Model's 'addr' field.
     * @param string $value The value to validate.
     * @returns string The validated value.
     * @throws ValidationError If the value does not match IP version specified in the 'ip_version' field.
     */
    public function validate_addr(string $value): string {
        # Asterisk (*) is always a valid value for the addr field, so return it without further validation
        if ($value === '*') {
            return $value;
        }

        # Do not allow the value to be an IPv4 address if ip_version is 'ipv6addr'
        if ($this->addr->has_label('is_ipaddrv4') and $this->ip_version->value === 'ipv6addr') {
            throw new ValidationError(
                message: "Field `addr` cannot be an IPv4 address when `ip_version` is set to `ipv6addr`, received `$value`.",
                response_id: 'FREERADIUS_INTERFACE_ADDR_IPV4_NOT_ALLOWED',
            );
        }

        # Do not allow the value to be an IPv6 address if ip_version is 'ipaddr'
        if ($this->addr->has_label('is_ipaddrv6') and $this->ip_version->value === 'ipaddr') {
            throw new ValidationError(
                message: "Field `addr` cannot be an IPv6 address when `ip_version` is set to `ipaddr`, received `$value`.",
                response_id: 'FREERADIUS_INTERFACE_ADDR_IPV6_NOT_ALLOWED',
            );
        }

        return $value;
    }

    /**
     * Perform additional validation on the Model's fields and data.
     */
    public function validate_extra(): void {
        # If there were validation errors that were not caught by the model fields, throw a ValidationError.
        # Ideally the Model should catch all validation errors itself so prompt the user to report this error
        $input_errors = [];
        freeradius_validate_interfaces($this->to_internal(), $input_errors);
        if (!empty($input_errors)) {
            throw new ValidationError(
                message: "An unexpected validation error has occurred: $input_errors[0]. Please report this issue at " .
                    'https://github.com/pfrest/pfSense-pkg-RESTAPI/issues/new',
                response_id: 'FREERADIUS_INTERFACE_UNEXPECTED_VALIDATION_ERROR',
            );
        }
    }

    /**
     * Apply the action on Interface(s)
     */
    public function apply(): void {
        freeradius_settings_resync();
    }
}
