<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\RegexValidator;
use RESTAPI\Validators\URLValidator;

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
    public StringField $framed_ip_address;
    public StringField $framed_ip_netmask;
    public StringField $framed_route;
    public StringField $framed_ipv6_address;
    public StringField $framed_ipv6_route;
    public StringField $vlan_id;
    public StringField $wispr_redirection_url;
    public IntegerField $simultaneous_connect;
    public StringField $expiration;
    public IntegerField $session_timeout;
    public StringField $login_time;
    public IntegerField $amount_of_time;
    public StringField $point_of_time;
    public IntegerField $max_total_octets;
    public StringField $max_total_octets_time_range;
    public IntegerField $max_bandwidth_down;
    public IntegerField $max_bandwidth_up;
    public IntegerField $acct_interim_interval;
    public StringField $top_additional_options;
    public StringField $check_items_additional_options;
    public StringField $reply_items_additional_options;

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
            verbose_name: 'Username',
            internal_name: 'varusersusername',
            help_text: 'The username for this user.',
        );
        $this->password = new StringField(
            required: true,
            allow_empty: false,
            allow_null: false,
            sensitive: true,
            verbose_name: 'Password',
            internal_name: 'varuserspassword',
            conditions: ['motp_enable' => false],
            help_text: 'The password for this username.',
        );
        $this->password_encryption = new StringField(
            required: false,
            default: 'Cleartext-Password',
            choices: ['Cleartext-Password', 'MD5-Password', 'MD5-Password-hashed', 'NT-Password-hashed'],
            verbose_name: 'Password Encryption',
            internal_name: 'varuserspasswordencryption',
            conditions: ['motp_enable' => false],
            help_text: 'The encryption method for the password.',
        );
        $this->motp_enable = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'MOTP Enable',
            internal_name: 'varusersmotpenable',
            help_text: 'Enable or disable the use of Mobile One-Time Password (MOTP) for this user.',
        );
        $this->motp_authmethod = new StringField(
            required: false,
            default: 'googleauth',
            choices: ['motp', 'googleauth'],
            verbose_name: 'MOTP Auth Method',
            internal_name: 'varusersauthmethod',
            conditions: ['motp_enable' => true],
            help_text: 'The authentication method for the Mobile One-Time Password (MOTP).',
        );
        $this->motp_secret = new StringField(
            required: true,
            allow_null: false,
            sensitive: true,
            verbose_name: 'MOTP Secret',
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
            verbose_name: 'MOTP PIN',
            internal_name: 'varusersmotppin',
            conditions: ['motp_enable' => true],
            help_text: 'The PIN for the Mobile One-Time Password (MOTP). It must be exactly 4 digits.',
        );
        $this->motp_offset = new IntegerField(
            required: false,
            default: 0,
            allow_null: false,
            verbose_name: 'MOTP Offset',
            internal_name: 'varusersmotpoffset',
            conditions: ['motp_enable' => true],
            help_text: 'The timezone offset for this user.',
        );
        $this->description = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
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
            allow_empty: true,
            verbose_name: 'Framed IP Address',
            internal_name: 'varusersframedipaddress',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            help_text: 'Framed-IP-Address MUST be supported by NAS. ' .
                'If the OpenVPN server uses a subnet style Topology the RADIUS server MUST ' .
                'also send back an appropriate Framed-IP-Netmask value matching the VPN Tunnel Network.',
        );
        $this->framed_ip_netmask = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'Framed IP Netmask',
            internal_name: 'varusersframedipnetmask',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            help_text: 'Framed-IP-Netmask MUST be supported by NAS',
        );
        $this->framed_route = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'IPv4 Gateway',
            internal_name: 'varusersframedroute',
            help_text: 'Framed-Route must be supported by NAS. Required format: ' .
                'Subnet Gateway Metric(s) (e.g. 192.168.10.0/24 192.168.10.1 1).',
        );
        $this->framed_ipv6_address = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'IPv6 Address',
            internal_name: 'varusersframedip6address',
            help_text: 'When the IPv6 prefix part is empty it uses Framed-IPv6-Address. ' .
                'When the prefix part is filled in, it uses Framed-IPv6-Prefix. ' .
                'Example: 2001:db8:abab::5 or 2001:db8:abab::/64',
        );
        $this->framed_ipv6_route = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'IPv6 Gateway',
            internal_name: 'varusersframedip6route',
            help_text: 'Framed-IPv6-Route must be supported by NAS. Required format: ' .
                'Prefix Gateway Metric(s) (e.g. 2001:db8:0:16::/64 2001:db8::16:a0:20ff:fe99:a998 1).',
        );
        $this->vlan_id = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'VLAN ID',
            internal_name: 'varusersvlanid',
            help_text: 'The VLAN ID (integer from 1-4095) or the VLAN name that this user should be assigned to. ' .
                'Must be supported by the NAS.',
        );
        $this->wispr_redirection_url = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'Redirection URL',
            internal_name: 'varuserswisprredirectionurl',
            validators: [new URLValidator()],
            help_text: 'The URL the user should be redirected to after successful login. ' .
                'Example: http://www.google.com',
        );
        $this->simultaneous_connect = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 0,
            verbose_name: 'Simultaneous Connections',
            internal_name: 'varuserssimultaneousconnect',
            help_text: 'The maximum number of simultaneous connections with this username. Leave null for no limit. ' .
                'If using FreeRADIUS with Captive Portal you should leave this null.',
        );
        $this->expiration = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'Expiration Date',
            internal_name: 'varusersexpiration',
            help_text: 'The date when this account should expire. Required format: Mmm dd yyyy (e.g. Jan 01 2030).',
        );
        $this->session_timeout = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 0,
            verbose_name: 'Session Timeout',
            internal_name: 'varuserssessiontimeout',
            help_text: 'The time this user has until relogin (in seconds).',
        );
        $this->login_time = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'Possible Login Times',
            internal_name: 'varuserslogintime',
            validators: [
                new RegexValidator(
                    pattern: '/^[a-zA-Z0-9,|-]*$/',
                    error_msg: 'Value may only contain a-z, A-Z, 0-9, comma, vertical bar and hyphen.',
                ),
            ],
            help_text: 'The time when this user should have access. Empty for no time restriction. ' .
                'Example: Wk0855-2305,Sa|Su2230-0230 ' .
                '(weekdays after 8:55 AM and before 11:05 PM | any time on Saturday | ' .
                'Sunday after 10:30 PM and before 02:30 AM).',
        );
        $this->amount_of_time = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 0,
            verbose_name: 'Amount of Time',
            internal_name: 'varusersamountoftime',
            help_text: 'The amount of time this user is allowed (in minutes) within the configured time period.',
        );
        $this->point_of_time = new StringField(
            required: false,
            default: 'Daily',
            choices: ['Daily', 'Weekly', 'Monthly', 'Forever'],
            verbose_name: 'Time Period',
            internal_name: 'varuserspointoftime',
            help_text: "The time period after which the 'Amount of Time' is reset.",
        );
        $this->max_total_octets = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 0,
            verbose_name: 'Max Total Octets',
            internal_name: 'varusersmaxtotaloctets',
            help_text: 'The amount of download and upload traffic (summarized) in megabytes (MB) for this user. ' .
                'If using captive portal without periodic reauthentication enabled, this value must not exceed ' .
                '4095 due to protocol limitations.',
        );
        $this->max_total_octets_time_range = new StringField(
            required: false,
            default: 'daily',
            choices: ['daily', 'weekly', 'monthly', 'forever'],
            verbose_name: 'Max Total Octets Time Range',
            internal_name: 'varusersmaxtotaloctetstimerange',
            help_text: 'The time period for the amount of download and upload traffic. ' .
                'This does not automatically reset the counter; you must configure a cronjob to reset it.',
        );
        $this->max_bandwidth_down = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 0,
            maximum: 4294967,
            verbose_name: 'Max Bandwidth Down',
            internal_name: 'varusersmaxbandwidthdown',
            help_text: 'The maximum bandwidth for download in kilobits (1000 bits) per second (Kbit/s).',
        );
        $this->max_bandwidth_up = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 0,
            maximum: 4294967,
            verbose_name: 'Max Bandwidth Up',
            internal_name: 'varusersmaxbandwidthup',
            help_text: 'The maximum bandwidth for upload in kilobits (1000 bits) per second (Kbit/s).',
        );
        $this->acct_interim_interval = new IntegerField(
            required: false,
            default: null,
            allow_null: true,
            minimum: 61,
            verbose_name: 'Accounting Interim Interval',
            internal_name: 'varusersacctinteriminterval',
            help_text: 'The interval in seconds which should elapse between interim-updates. ' .
                'Must be more than 60s and should not be less than 600s.',
        );
        $this->top_additional_options = new StringField(
            required: false,
            default: [],
            allow_empty: true,
            many: true,
            delimiter: '|',
            verbose_name: 'Additional Top Options',
            internal_name: 'varuserstopadditionaloptions',
            help_text: 'Additional RADIUS attributes placed at the TOP of this user entry. ' .
                'Each list entry becomes a separate line. For experts only.',
        );
        $this->check_items_additional_options = new StringField(
            required: false,
            default: [],
            allow_empty: true,
            many: true,
            delimiter: '|',
            verbose_name: 'Additional Check-Item Options',
            internal_name: 'varuserscheckitemsadditionaloptions',
            help_text: 'Additional RADIUS check-item attributes for this user. ' .
                'Each list entry becomes a separate attribute. For experts only.',
        );
        $this->reply_items_additional_options = new StringField(
            required: false,
            default: [],
            allow_empty: true,
            many: true,
            delimiter: '|',
            verbose_name: 'Additional Reply-Item Options',
            internal_name: 'varusersreplyitemsadditionaloptions',
            help_text: 'Additional RADIUS reply-item attributes for this user. ' .
                'Each list entry becomes a separate attribute. For experts only.',
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
