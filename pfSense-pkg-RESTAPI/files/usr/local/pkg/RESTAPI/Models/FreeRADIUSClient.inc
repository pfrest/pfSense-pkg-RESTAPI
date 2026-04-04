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

/**
 * Defines a Model that represents FreeRADIUS Clients
 */
class FreeRADIUSClient extends Model {
    public StringField $addr;
    public StringField $type;
    public StringField $ip_version;
    public StringField $description;
    public StringField $shortname;
    public StringField $secret;
    public StringField $proto;
    public StringField $nastype;
    public BooleanField $msgauth;
    public IntegerField $maxconn;
    public StringField $naslogin;
    public StringField $naspassword;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->packages = ['pfSense-pkg-freeradius3'];
        $this->package_includes = ['freeradius.inc'];
        $this->config_path = 'installedpackages/freeradiusclients/config';
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'FreeRADIUS Client';

        # Set model fields
        $this->addr = new StringField(
            required: true,
            unique: true,
            internal_name: 'varclientip',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The IP address or network of the RADIUS client(s) in CIDR notation. This is the IP of the ' .
                'NAS (switch, access point, firewall, router, etc.)',
        );
        $this->ip_version = new StringField(
            default: 'ipaddr',
            choices: ['ipaddr', 'ipv6addr'],
            internal_name: 'varclientipversion',
            help_text: 'The IP version of the this Client.',
        );
        $this->shortname = new StringField(
            required: true,
            internal_name: 'varclientshortname',
            help_text: 'A short name for the client. This is generally the hostname of the NAS.',
        );
        $this->secret = new StringField(
            required: true,
            sensitive: true,
            maximum_length: 31,
            internal_name: 'varclientsharedsecret',
            help_text: 'This is the shared secret (password) which the NAS (switch, accesspoint, etc.) needs to ' .
                'communicate with the RADIUS server.',
        );

        $this->proto = new StringField(
            default: 'udp',
            choices: ['udp', 'tcp'],
            internal_name: 'varclientproto',
            help_text: 'The protocol the client uses.',
        );
        $this->nastype = new StringField(
            default: 'other',
            choices: [
                'cisco',
                'cvx',
                'computone',
                'digitro',
                'livingston',
                'juniper',
                'max40xx',
                'mikrotik',
                'mikrotik_snmp',
                'dot1x',
                'other',
            ],
            allow_empty: true,
            internal_name: 'varclientnastype',
            help_text: 'The NAS type of the client. This is used by checkrad.pl for simultaneous use checks.',
        );
        $this->msgauth = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            internal_name: 'varrequiremessageauthenticator',
            help_text: 'RFC5080 requires Message-Authenticator in Access-Request. But older NAS (switches or ' .
                'accesspoints) do not include that.',
        );
        $this->maxconn = new IntegerField(
            default: 16,
            minimum: 1,
            maximum: 32,
            internal_name: 'varclientmaxconnections',
            help_text: 'Takes only effect if you use TCP as protocol. Limits the number of simultaneous TCP 
                connections from a client.',
        );
        $this->naslogin = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'varclientlogininput',
            help_text: 'If supported by your NAS, you can use SNMP or finger for simultaneous-use checks instead of ' .
                '(s)radutmp file and accounting. Leave empty to choose (s)radutmp.',
        );
        $this->naspassword = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'varclientpasswordinput',
            help_text: 'If supported by your NAS, you can use SNMP or finger for simultaneous-use checks instead of ' .
                '(s)radutmp file and accounting. Leave empty to choose (s)radutmp.',
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
        # Do not allow the value to be an IPv4 address if ip_version is 'ipv6addr'
        if ($this->addr->has_label('is_ipaddrv4') and $this->ip_version->value === 'ipv6addr') {
            throw new ValidationError(
                message: "Field `addr` cannot be an IPv4 address when `ip_version` is set to `ipv6addr`, received `$value`.",
                response_id: 'FREERADIUS_CLIENT_ADDR_IPV4_NOT_ALLOWED',
            );
        }

        # Do not allow the value to be an IPv6 address if ip_version is 'ipaddr'
        if ($this->addr->has_label('is_ipaddrv6') and $this->ip_version->value === 'ipaddr') {
            throw new ValidationError(
                message: "Field `addr` cannot be an IPv6 address when `ip_version` is set to `ipaddr`, received `$value`.",
                response_id: 'FREERADIUS_CLIENT_ADDR_IPV6_NOT_ALLOWED',
            );
        }

        return $value;
    }

    /**
     * Perform additional validation on the Model's fields and data.
     */
    /**
     * Perform additional validation on the Model's fields and data.
     */
    public function validate_extra(): void {
        # If there were validation errors that were not caught by the model fields, throw a ValidationError.
        # Ideally the Model should catch all validation errors itself so prompt the user to report this error
        $input_errors = [];
        freeradius_validate_clients($this->to_internal(), $input_errors);
        if (!empty($input_errors)) {
            throw new ValidationError(
                message: "An unexpected validation error has occurred: $input_errors[0]. Please report this issue at " .
                    'https://github.com/pfrest/pfSense-pkg-RESTAPI/issues/new',
                response_id: 'FREERADIUS_CLIENTS_UNEXPECTED_VALIDATION_ERROR',
            );
        }
    }

    /**
     * Apply specific action on Client(s)
     */
    public function apply(): void {
        freeradius_clients_resync();
    }
}
