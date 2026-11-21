<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Validators\MACAddressValidator;

class APIValidatorsMACAddressValidatorTestCase extends TestCase {
    /**
     * Checks that validation fails if a value is not a valid MAC address.
     */
    public function test_throw_response_for_non_mac_value() {
        $this->assert_throws_response(
            response_id: 'MAC_ADDRESS_VALIDATOR_FAILED',
            code: 400,
            callable: function () {
                $test_validator = new MACAddressValidator();
                $test_validator->validate('not a MAC!');
            },
        );
    }

    /**
     * Check that the MACAddressValidator does not throw any errors for values that are MAC addresses.
     */
    public function test_mac_address_validator() {
        $this->assert_does_not_throw(function () {
            $test_validator = new MACAddressValidator();
            $test_validator->validate('00:13:10:00:00:00');
            $test_validator->validate('00:13:10:00:00:0a');
            $test_validator->validate('00:13:10:00:00:0A');
        });
    }

    /**
     * Checks that partial MAC addresses (OUI prefixes) are still rejected when the default strict mode is used.
     */
    public function test_throw_response_for_partial_mac_by_default() {
        $this->assert_throws_response(
            response_id: 'MAC_ADDRESS_VALIDATOR_FAILED',
            code: 400,
            callable: function () {
                $test_validator = new MACAddressValidator();
                $test_validator->validate('EC:71:DB');
            },
        );
    }

    /**
     * Checks that partial MAC addresses (OUI prefixes) are accepted when allow_partial is enabled, matching the
     * pfSense GUI behavior for DHCP mac_allow/mac_deny vendor prefix matching.
     */
    public function test_allow_partial_accepts_oui_prefixes() {
        $this->assert_does_not_throw(function () {
            $test_validator = new MACAddressValidator(allow_partial: true);
            $test_validator->validate('EC:71:DB');
            $test_validator->validate('00:60');
            $test_validator->validate('B8:27:EB:AA');
            $test_validator->validate('00:13:10:00:00:00');
        });
    }

    /**
     * Checks that garbage values are still rejected when allow_partial is enabled.
     */
    public function test_allow_partial_still_rejects_garbage() {
        $this->assert_throws_response(
            response_id: 'MAC_ADDRESS_VALIDATOR_FAILED',
            code: 400,
            callable: function () {
                $test_validator = new MACAddressValidator(allow_partial: true);
                $test_validator->validate('EC:71:ZZ');
                $test_validator->validate('not a MAC!');
                $test_validator->validate('00:13:10:00:00:00:00');
            },
        );
    }
}
