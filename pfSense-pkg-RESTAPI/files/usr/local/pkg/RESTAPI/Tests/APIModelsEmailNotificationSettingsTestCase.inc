<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\EmailNotificationSettings;
use RESTAPI\Models\RESTAPISettings;

class APIModelsEmailNotificationSettingsTestCase extends TestCase {
    /**
     * Checks if we can correctly read and update the email notification settings.
     */
    public function test_read_and_update(): void {
        $email_settings = new EmailNotificationSettings(
            disable: false,
            ipaddress: 'example.com',
            port: '465',
            timeout: 24,
            ssl: true,
            sslvalidate: false,
            fromaddress: 'from@example.com',
            notifyemailaddress: 'to@example.com',
            authentication_mechanism: 'LOGIN',
            username: 'user',
            password: 'password',
        );
        $email_settings->update();
        $this->assert_is_not_empty(EmailNotificationSettings::read_all()->to_representation());
        $this->assert_type(EmailNotificationSettings::read_all()->to_representation(), type: 'array');
    }

    /**
     * Checks that the `password` field is hidden from representations unless explicitly overridden.
     */
    public function test_password_is_sensitive(): void {
        # Ensure the password is hidden by default
        $email_settings = new EmailNotificationSettings(authentication_mechanism: 'LOGIN', password: 'password');
        $this->assert_is_false(array_key_exists('password', $email_settings->to_representation()));

        # Ensure the password can be exposed using the `override_sensitive_fields` setting
        $api_settings = new RESTAPISettings(override_sensitive_fields: ['EmailNotificationSettings:password']);
        $api_settings->update();
        $this->assert_equals($email_settings->to_representation()['password'], 'password');

        # Reset the override_sensitive_fields setting
        $api_settings = new RESTAPISettings(override_sensitive_fields: []);
        $api_settings->update();
    }
}
