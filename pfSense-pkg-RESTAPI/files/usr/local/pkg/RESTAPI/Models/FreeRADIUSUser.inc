<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\RegexValidator;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that represents FreeRADIUS Users
 */
class FreeRADIUSUser extends Model {
    public StringField $username;
    public StringField $password;
    public StringField $password_encryption;
    public BooleanField $motp_enable;
    public StringField $motp_authmethod;
    public StringField $motp_secret;
    public StringField $motp_pin;
    public IntegerField $motp_offset;
    public StringField $description;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->packages = ['pfSense-pkg-freeradius3'];
        $this->package_includes = ['freeradius.inc'];
        $this->config_path = 'installedpackages/freeradius/config';
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'FreeRADIUS User';

        # Set model fields
        $this->username = new StringField(
            required: true,
            unique: true,
            internal_name: 'varusersusername',
            help_text: 'The username for this user.',
        );
        $this->password = new StringField(
            required: true,
            allow_empty: false,
            allow_null: false,
            sensitive: true,
            internal_name: 'varuserspassword',
            conditions: ['motp_enable' => false],
            help_text: 'The password for this username.',
        );
        $this->password_encryption = new StringField(
            required: false,
            default: 'Cleartext-Password',
            choices: ['Cleartext-Password', 'MD5-Password', 'MD5-Password-hashed', 'NT-Password-hashed'],
            internal_name: 'varuserspasswordencryption',
            conditions: ['motp_enable' => false],
            help_text: 'The encryption method for the password.',
        );
        $this->motp_enable = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            internal_name: 'varusersmotpenable',
            help_text: 'Enable or disable the use of Mobile One-Time Password (MOTP) for this user.',
        );
        $this->motp_authmethod = new StringField(
            required: false,
            default: 'googleauth',
            choices: ['motp', 'googleauth'],
            internal_name: 'varusersauthmethod',
            conditions: ['motp_enable' => true],
            help_text: 'The authentication method for the Mobile One-Time Password (MOTP).',
        );
        $this->motp_secret = new StringField(
            required: true,
            allow_null: false,
            sensitive: true,
            internal_name: 'varusersmotpinitsecret',
            conditions: ['motp_enable' => true],
            help_text: 'The secret for the Mobile One-Time Password (MOTP).',
        );
        $this->motp_pin = new StringField(
            required: true,
            allow_null: false,
            sensitive: true,
            minimum_length: 4,
            maximum_length: 4,
            internal_name: 'varusersmotppin',
            conditions: ['motp_enable' => true],
            help_text: 'The PIN for the Mobile One-Time Password (MOTP). It must be exactly 4 digits.',
        );
        $this->motp_offset = new IntegerField(
            required: false,
            default: 0,
            allow_null: false,
            internal_name: 'varusersmotpoffset',
            conditions: ['motp_enable' => true],
            help_text: 'The timezone offset for this user.',
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
            help_text: 'A description for this user.',
        );
        $this->framed_ip_address = new StringField(
            required: false,
            default: '',
            internal_name: 'varusersframedipaddress',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            help_text: 'Framed-IP-Address MUST be supported by NAS. ' .
                'If the OpenVPN server uses a subnet style Topology the RADIUS server MUST ' .
                'also send back an appropriate Framed-IP-Netmask value matching the VPN Tunnel Network.',
        );
        $this->framed_ip_netmask = new StringField(
            required: false,
            default: '',
            internal_name: 'varusersframedipnetmask',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            help_text: 'Framed-IP-Netmask MUST be supported by NAS',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Perform additional validation on the Model's fields and data.
     */
    public function validate_extra(): void {
        # Run service level validations
        $input_errors = [];
        $user = $this->to_internal();
        freeradius_validate_users($user, $input_errors);

        # If there were validation errors that were not caught by the model fields, throw a ValidationError.
        # Ideally the Model should catch all validation errors itself so prompt the user to report this error
        if (!empty($input_errors)) {
            throw new ValidationError(
                message: "An unexpected validation error has occurred: $input_errors[0]. Please report this issue at " .
                    'https://github.com/pfrest/pfSense-pkg-RESTAPI/issues/new',
                response_id: 'FREERADIUS_USER_UNEXPECTED_VALIDATION_ERROR',
            );
        }
    }

    /**
     * Apply the changes made to this Model to the FreeRADIUS configuration.
     */
    public function apply(): void {
        freeradius_users_resync();
    }
}
