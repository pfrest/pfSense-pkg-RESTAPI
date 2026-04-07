<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\LengthValidator;

/**
 * Defines a Model for interacting with NTP server settings.
 */
class NTPSettings extends Model {
    public BooleanField $enable;
    public InterfaceField $interface;
    public IntegerField $ntpmaxpeers;
    public IntegerField $orphan;
    public StringField $ntpminpoll;
    public StringField $ntpmaxpoll;
    public StringField $dnsresolv;
    public BooleanField $logpeer;
    public BooleanField $logsys;
    public BooleanField $clockstats;
    public BooleanField $loopstats;
    public BooleanField $peerstats;
    public BooleanField $statsgraph;
    public Base64Field $leapsec;
    public BooleanField $serverauth;
    public Base64Field $serverauthkey;
    public StringField $serverauthalgo;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'ntpd';
        $this->update_strategy = 'merge';
        $this->always_apply = true;
        $this->many = false;

        # Set model fields
        $this->enable = new BooleanField(
            default: true,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'Enable',
            help_text: 'Enables or disabled the NTP server.',
        );
        $this->interface = new InterfaceField(
            default: [],
            allow_localhost_interface: true,
            allow_empty: true,
            many: true,
            verbose_name: 'Interface',
            help_text: 'The interfaces the NTP server will listen on.',
        );
        $this->ntpmaxpeers = new IntegerField(
            default: 5,
            minimum: 4,
            maximum: 25,
            verbose_name: 'NTP Max Peers',
            help_text: 'The maximum number of candidate peers in the NTP pool.',
        );
        $this->orphan = new IntegerField(
            default: 12,
            minimum: 1,
            maximum: 15,
            verbose_name: 'Orphan',
            help_text: 'The orphan mode stratum to set. Orphan mode allows the system clock to be used when no other ' .
                'clocks are available. The number here specifies the stratum reported during orphan mode and should ' .
                'normally be set to a number high enough to ensure that any other servers available to clients are ' .
                'preferred over this server',
        );
        $this->ntpminpoll = new StringField(
            default: '',
            choices: ['', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', 'omit'],
            allow_empty: true,
            verbose_name: 'NTP Min Poll',
            help_text: 'The minimum poll interval for NTP messages. Use empty string to assume system default, and ' .
                '`omit` to not set this value.',
        );
        $this->ntpmaxpoll = new StringField(
            default: '',
            choices: ['', '3', '4', '5', '6', '7', '8', '9', '10', '11', '12', '13', '14', '15', '16', '17', 'omit'],
            allow_empty: true,
            verbose_name: 'NTP Max Poll',
            help_text: 'The maximum poll interval for NTP messages. Use empty string to assume system default, and ' .
                '`omit` to not set this value. This value must be greater than the `ntpminpoll`.',
        );
        $this->dnsresolv = new StringField(
            default: 'auto',
            choices: ['auto', 'inet', 'inet6'],
            verbose_name: 'DNS Resolve',
            help_text: 'The IP protocol peers are forced to use for DNS resolution.',
        );
        $this->logpeer = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Log Peer',
            help_text: 'Enable or disable logging peer messages.',
        );
        $this->logsys = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Logsys',
            help_text: 'Enable or disable logging system messages.',
        );
        $this->clockstats = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Clock Stats',
            help_text: 'Enable or disable logging reference clock statistics.',
        );
        $this->loopstats = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Loop Stats',
            help_text: 'Enable or disable logging clock discipline statistics.',
        );
        $this->peerstats = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Peer Stats',
            help_text: 'Enable or disable logging peer statistics.',
        );
        $this->statsgraph = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Statistics Graph',
            help_text: 'Enable or disable RRD graphs for NTP statistics.',
        );
        $this->leapsec = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'Leap Second',
            help_text: 'The Leap second configuration as text.',
        );
        $this->serverauth = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Server Auth',
            help_text: 'Enable or disable NTPv3 server authentication. (RFC 1305)',
        );
        $this->serverauthkey = new Base64Field(
            required: true,
            sensitive: true,
            conditions: ['serverauth' => true],
            validators: [new LengthValidator(minimum: 1, maximum: 64)],
            verbose_name: 'Server Auth Key',
            help_text: 'The NTP server authentication key.',
        );
        $this->serverauthalgo = new StringField(
            default: 'md5',
            choices: ['md5', 'sha1', 'sha256'],
            verbose_name: 'Server Auth Algorithm',
            help_text: 'The digest algorithm for the server authentication key.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the entire model.
     */
    public function validate_extra(): void {
        # Ensure the `ntpminpoll` value is not greater than `ntpmaxpoll`
        $min_poll = $this->ntpminpoll->value;
        $max_poll = $this->ntpmaxpoll->value;
        if (is_numeric($min_poll) and is_numeric($max_poll) and $min_poll > $max_poll) {
            throw new ValidationError(
                message: 'Field `ntpminpoll` cannot be greater than the `ntpmaxpoll` value.',
                response_id: 'NTP_SERVER_NTPMINPOLL_GREATER_THAN_NTPMAXPOLL',
            );
        }
    }

    /**
     * Adds extra validation to the `serverauthkey` value.
     * @param string $serverauthkey The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $serverauthkey is greater than 20 characters and `serverauthalgo` is `md5`
     * @throws ValidationError When $serverauthkey is not exactly 40 characters and `serverauthalgo` is `sha1`
     * @throws ValidationError When $serverauthkey is not exactly 64 characters and `serverauthalgo` is `sha256`
     * @throws ValidationError When $serverauthkey is not a hex encoded value and `serverauthalgo` is `sha1` or `sha256`
     */
    public function validate_serverauthkey(string $serverauthkey): string {
        # Throw an error if $serverauthkey is greater than 20 characters and `serverauthalgo` is md5
        if ($this->serverauthalgo->value === 'md5' and strlen($serverauthkey) > 20) {
            throw new ValidationError(
                message: 'Field `serverauthkey` cannot contain more than 20 characters when `serverauthalgo` is `md5`.',
                response_id: 'NTP_SERVER_SERVERAUTHKEY_EXCEEDS_MAXIMUM_LENGTH_FOR_MD5',
            );
        }

        # Throw an error if $serverauthkey is not exactly 40 characters and `serverauthalgo` is sha1
        if ($this->serverauthalgo->value === 'sha1' and strlen($serverauthkey) !== 40) {
            throw new ValidationError(
                message: 'Field `serverauthkey` must be exactly 40 characters when `serverauthalgo` is set to `sha1`.',
                response_id: 'NTP_SERVER_SERVERAUTHKEY_SHA1_CHARACTER_REQUIREMENT_UNMET',
            );
        }

        # Throw an error if $serverauthkey is not a hex value and `serverauthalgo` is sha1
        if ($this->serverauthalgo->value === 'sha1' and !ctype_xdigit($serverauthkey)) {
            throw new ValidationError(
                message: 'Field `serverauthkey` must be a hex-encoded value when `serverauthalgo` is set to `sha1`.',
                response_id: 'NTP_SERVER_SERVERAUTHKEY_MUST_BE_HEX_VALUE_SHA1',
            );
        }

        # Throw an error if $serverauthkey is not exactly of 64 characters and `serverauthalgo` is sha256
        if ($this->serverauthalgo->value === 'sha256' and strlen($serverauthkey) !== 64) {
            throw new ValidationError(
                message: 'Field `serverauthkey` must be exactly 40 characters when `serverauthalgo` is set to `sha1`.',
                response_id: 'NTP_SERVER_SERVERAUTHKEY_SHA256_CHARACTER_REQUIREMENT_UNMET',
            );
        }

        # Throw an error if $serverauthkey is not a hex value and `serverauthalgo` is sha256
        if ($this->serverauthalgo->value === 'sha256' and !ctype_xdigit($serverauthkey)) {
            throw new ValidationError(
                message: 'Field `serverauthkey` must be a hex-encoded value when `serverauthalgo` is set to `sha256`.',
                response_id: 'NTP_SERVER_SERVERAUTHKEY_MUST_BE_HEX_VALUE_SHA256',
            );
        }

        return $serverauthkey;
    }

    /**
     * Applies changes to NTP server.
     */
    public function apply(): void {
        system_ntp_configure();
    }
}
