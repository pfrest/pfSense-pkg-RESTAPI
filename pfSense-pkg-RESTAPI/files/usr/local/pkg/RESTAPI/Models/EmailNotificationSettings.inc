<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\EmailAddressValidator;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a model that interacts with email notification settings.
 */
class EmailNotificationSettings extends Model {
    public BooleanField $disable;
    public StringField $ipaddress;
    public PortField $port;
    public IntegerField $timeout;
    public BooleanField $ssl;
    public BooleanField $sslvalidate;
    public StringField $fromaddress;
    public StringField $notifyemailaddress;
    public StringField $authentication_mechanism;
    public StringField $username;
    public StringField $password;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'notifications/smtp';
        $this->many = false;

        # Set model fields
        $this->disable = new BooleanField(default: false, verbose_name: 'Disable', help_text: 'Disables SMTP notifications.');
        $this->ipaddress = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'IP Address',
            help_text: 'The IP address or hostname of the SMTP server.',
        );
        $this->port = new PortField(default: '25', verbose_name: 'Port', help_text: 'The port number of the SMTP server.');
        $this->timeout = new IntegerField(default: 20, verbose_name: 'Timeout', help_text: 'The timeout (in seconds) for the SMTP connection.');
        $this->ssl = new BooleanField(
            default: false,
            verbose_name: 'SSL',
            help_text: 'Enables or disables SSL/TLS for the SMTP connection.',
        );
        $this->sslvalidate = new BooleanField(
            default: true,
            verbose_name: 'SSL Validate',
            help_text: 'Enables or disables SSL/TLS certificate validation for the SMTP connection.',
        );
        $this->fromaddress = new StringField(
            default: '',
            allow_empty: true,
            validators: [new EmailAddressValidator()],
            verbose_name: 'From Address',
            help_text: 'The email address to use as the "From" address in notifications.',
        );
        $this->notifyemailaddress = new StringField(
            default: '',
            allow_empty: true,
            validators: [new EmailAddressValidator()],
            verbose_name: 'Notify Email Address',
            help_text: 'The email address to send notifications to.',
        );
        $this->authentication_mechanism = new StringField(
            default: 'PLAIN',
            choices: ['PLAIN', 'LOGIN'],
            verbose_name: 'Authentication Mechanism',
            help_text: 'The authentication mechanism to use for the SMTP connection.',
        );
        $this->username = new StringField(
            required: true,
            conditions: ['authentication_mechanism' => 'LOGIN'],
            verbose_name: 'Username',
            help_text: 'The username to use for SMTP authentication.',
        );
        $this->password = new StringField(
            required: true,
            conditions: ['authentication_mechanism' => 'LOGIN'],
            verbose_name: 'Password',
            help_text: 'The password to use for SMTP authentication.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
