<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DHCPServerApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a model that configures custom DHCP options.
 */
class DHCPServerCustomOption extends Model {
    public IntegerField $number;
    public StringField $type;
    public Base64Field $value;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'DHCPServer';
        $this->config_path = 'numberoptions/item';
        $this->subsystem = 'dhcpd';
        $this->many = true;

        # Define model fields
        $this->number = new IntegerField(
            required: true,
            unique: true,
            minimum: 1,
            maximum: 254,
            verbose_name: 'Number',
            help_text: 'The DHCP option number to configure.',
        );
        $this->type = new StringField(
            required: true,
            choices: [
                'text',
                'string',
                'boolean',
                'unsigned integer 8',
                'unsigned integer 16',
                'unsigned integer 32',
                'signed integer 8',
                'signed integer 16',
                'signed integer 32',
                'ip-address',
            ],
            verbose_name: 'Type',
            help_text: 'The type of value to configure for the option.',
        );
        $this->value = new Base64Field(
            required: true,
            verbose_name: 'Value',
            help_text: 'The value to configure for the option.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `value` field.
     * @param string $value The incoming value to validate.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $value contains quotes and type is set to 'text'.
     * @throws ValidationError When $value is not a string enclosed in quotes or a hex string and type is set to 'string'.
     * @throws ValidationError When $value is not one of the boolean options and type is set to 'boolean'.
     * @throws ValidationError When $value is not a number between 0 and 255 and type is set to 'unsigned integer 8'.
     * @throws ValidationError When $value is not a number between 0 and 65535 and type is set to 'unsigned integer 16'.
     * @throws ValidationError When $value is not a number between 0 and 4294967295 and type is set to 'unsigned integer 32'.
     * @throws ValidationError When $value is not a number between -128 and 127 and type is set to 'signed integer 8'.
     * @throws ValidationError When $value is not a number between -32768 and 32767 and type is set to 'signed integer 16'.
     * @throws ValidationError When $value is not a number between -2147483648 and 2147483647 and type is set to 'signed integer 32'.
     * @throws ValidationError When $value is not a valid IPv4 address or hostname and type is set to 'ip-address'.
     */
    public function validate_value(string $value): string {
        # Define regular expressions for string validation
        $str_rgx_1 = '/^"[^"]*"$/'; // Regex to use when checking if string type value is enclosed in quotes
        $str_rgx_2 = '/^[0-9a-f]{2}(?:\:[0-9a-f]{2})*$/i'; // Regex to use when checking if string type value is hex
        $bool_opts = ['true', 'false', 'on', 'off']; // Boolean options

        # Validate this field when the type is set to 'text'
        if ($this->type->value === 'text' and strstr($value, '"')) {
            throw new ValidationError(
                message: "Field `value` must not contain quotes when the type is set to 'text'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_NOT_CONTAIN_QUOTES_WITH_TEXT_TYPE',
            );
        }
        # Validate this field when the type is set to 'string'
        elseif (
            $this->type->value === 'string' and
            !preg_match($str_rgx_1, $value) and
            !preg_match($str_rgx_2, $value)
        ) {
            throw new ValidationError(
                message: "Field `value` must be a string enclosed in quotes or a hex string when the type is set to 'string'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_STRING_OR_HEX_STRING_WITH_STRING_TYPE',
            );
        }
        # Validate this field when the type is set to 'boolean'
        elseif ($this->type->value === 'boolean' and !in_array($value, $bool_opts, true)) {
            throw new ValidationError(
                message: 'Field `value` must be one of the following: [' .
                    implode(', ', $bool_opts) .
                    '] ' .
                    "when the type is set to 'boolean'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_ONE_OF_BOOLEAN_OPTIONS_WITH_BOOLEAN_TYPE',
            );
        }
        # Validate this field when the type is set to 'unsigned integer 8'
        elseif ($this->type->value === 'unsigned integer 8' and (!is_numeric($value) or $value < 0 or $value > 255)) {
            throw new ValidationError(
                message: "Field `value` must be a number between 0 and 255 when the type is set to 'unsigned integer 8'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_IN_RANGE_WITH_UNSIGNED_INTEGER_8_TYPE',
            );
        }
        # Validate this field when the type is set to 'unsigned integer 16'
        elseif (
            $this->type->value === 'unsigned integer 16' and
            (!is_numeric($value) or $value < 0 or $value > 65535)
        ) {
            throw new ValidationError(
                message: "Field `value` must be a number between 0 and 65535 when the type is set to 'unsigned integer 16'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_IN_RANGE_WITH_UNSIGNED_INTEGER_16_TYPE',
            );
        }
        # Validate this field when the type is set to 'unsigned integer 32'
        elseif (
            $this->type->value === 'unsigned integer 32' and
            (!is_numeric($value) or $value < 0 or $value > 4294967295)
        ) {
            throw new ValidationError(
                message: "Field `value` must be a number between 0 and 4294967295 when the type is set to 'unsigned integer 32'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_IN_RANGE_WITH_UNSIGNED_INTEGER_32_TYPE',
            );
        }
        # Validate this field when the type is set to 'signed integer 8'
        elseif ($this->type->value === 'signed integer 8' and (!is_numeric($value) or $value < -128 or $value > 127)) {
            throw new ValidationError(
                message: "Field `value` must be a number between -128 and 127 when the type is set to 'signed integer 8'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_IN_RANGE_WITH_SIGNED_INTEGER_8_TYPE',
            );
        }
        # Validate this field when the type is set to 'signed integer 16'
        elseif (
            $this->type->value === 'signed integer 16' and
            (!is_numeric($value) or $value < -32768 or $value > 32767)
        ) {
            throw new ValidationError(
                message: "Field `value` must be a number between -32768 and 32767 when the type is set to 'signed integer 16'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_IN_RANGE_WITH_SIGNED_INTEGER_16_TYPE',
            );
        }
        # Validate this field when the type is set to 'signed integer 32'
        elseif (
            $this->type->value === 'signed integer 32' and
            (!is_numeric($value) or $value < -2147483648 or $value > 2147483647)
        ) {
            throw new ValidationError(
                message: "Field `value` must be a number between -2147483648 and 2147483647 when the type is set to 'signed integer 32'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_IN_RANGE_WITH_SIGNED_INTEGER_32_TYPE',
            );
        }
        # Validate this field when the type is set to 'ip-address'
        elseif ($this->type->value === 'ip-address' and !is_ipaddrv4($value) and !is_fqdn($value)) {
            throw new ValidationError(
                message: "Field `value` must be a valid IPv4 address or hostname when the type is set to 'ip-address'.",
                response_id: 'DHCP_SERVER_CUSTOM_OPTION_VALUE_MUST_BE_VALID_IPV4_WITH_IP_ADDRESS_TYPE',
            );
        }

        # Otherwise, input is valid. Save a Base64 representation of the data as expected by pfSense
        return $value;
    }

    /**
     * Applies changes the DHCP Server.
     */
    public function apply(): void {
        (new DHCPServerApplyDispatcher(async: $this->async))->spawn_process();
    }
}
