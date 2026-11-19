<?php

namespace RESTAPI\Validators;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Validator that checks if a given value is a valid MAC address.
 */
class MACAddressValidator extends RESTAPI\Core\Validator {
    public array $allow_keywords;
    public bool $allow_partial;

    /**
     * Constructs the MACAddressValidator with requested parameters.
     * @param array $allow_keywords An array of specified non-MAC address keywords that should be considered valid.
     * @param bool $allow_partial Whether partial MAC addresses (OUI prefixes such as `EC:71:DB`) are considered
     * valid. This mirrors the pfSense GUI, which accepts partial MACs in DHCP `mac_allow`/`mac_deny` fields to
     * match all devices of a vendor. Defaults to false to preserve strict full-MAC validation.
     */
    public function __construct(array $allow_keywords = [], bool $allow_partial = false) {
        $this->allow_keywords = $allow_keywords;
        $this->allow_partial = $allow_partial;
    }

    /**
     * Checks if a given value is a valid MAC address.
     * @param mixed $value The value to validate.
     * @param string $field_name The field name of the value being validated. This is used for error messages.
     * @throws ValidationError When the value is not a valid MAC address.
     */
    public function validate(mixed $value, string $field_name = ''): void {
        # Throw a ValidationError if this value is not a MAC address or an allowed keyword
        if (!is_macaddr($value, $this->allow_partial) and !in_array($value, $this->allow_keywords)) {
            throw new ValidationError(
                message: "Field `$field_name` must be a valid MAC address, received `$value`.",
                response_id: 'MAC_ADDRESS_VALIDATOR_FAILED',
            );
        }
    }
}
