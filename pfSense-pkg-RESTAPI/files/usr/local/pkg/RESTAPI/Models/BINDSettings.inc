<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a model that interacts the general BIND settings.
 */
class BINDSettings extends Model {
    public BooleanField $enable_bind;
    public StringField $bind_ip_version;
    public InterfaceField $listenon;
    public BooleanField $bind_notify;
    public BooleanField $bind_hide_version;
    public StringField $bind_ram_limit;
    public BooleanField $bind_logging;
    public StringField $log_severity;
    public StringField $log_options;
    public BooleanField $rate_enabled;
    public IntegerField $rate_limit;
    public BooleanField $log_only;
    public BooleanField $bind_forwarder;
    public StringField $bind_forwarder_ips;
    public StringField $bind_dnssec_validation;
    public PortField $listenport;
    public PortField $controlport;
    public Base64Field $bind_custom_options;
    public Base64Field $bind_global_settings;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/bind/config/0';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->many = false;
        $this->always_apply = true;
        $this->verbose_name = 'BIND Settings';

        # Set model fields
        $this->enable_bind = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'Enable BIND',
            help_text: 'Enables the BIND service.',
        );
        $this->bind_ip_version = new StringField(
            default: '',
            choices: ['', '-4', '-6'],
            allow_empty: true,
            verbose_name: 'BIND IP Version',
            help_text: 'The IP version to use for the BIND service. Leave empty to use both IPv4 and IPv6.',
        );
        $this->listenon = new InterfaceField(
            default: ['All'],
            allow_localhost_interface: true,
            allow_custom: ['All'],
            many: true,
            verbose_name: 'Listen On',
            help_text: 'The interfaces to listen on for DNS requests.',
        );
        $this->bind_notify = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'BIND Notify',
            help_text: 'Notify slave server after any update on master.',
        );
        $this->bind_hide_version = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'BIND Hide Version',
            help_text: 'Hide the BIND version in responses.',
        );
        $this->bind_ram_limit = new StringField(
            default: '256M',
            verbose_name: 'BIND RAM Limit',
            help_text: 'The maximum amount of RAM to use for the BIND service.',
        );
        $this->bind_logging = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'BIND Logging',
            help_text: 'Enable logging for the BIND service.',
        );
        $this->log_severity = new StringField(
            default: 'critical',
            choices: ['critical', 'error', 'warning', 'notice', 'info', 'debug 1', 'debug 3', 'debug 5', 'dynamic'],
            verbose_name: 'Log Severity',
            help_text: 'The minimum severity of events to log.',
        );
        $this->log_options = new StringField(
            default: ['default'],
            choices: [
                'default',
                'general',
                'database',
                'security',
                'config',
                'resolver',
                'xfer-in',
                'xfer-out',
                'notify',
                'client',
                'unmatched',
                'queries',
                'network',
                'update',
                'dispatch',
                'dnssec',
                'lame-servers',
            ],
            many: true,
            verbose_name: 'Log Options',
            help_text: 'The categories to log.',
        );
        $this->rate_enabled = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'Rate Enabled',
            help_text: 'Enable rate limiting for the BIND service.',
        );
        $this->log_only = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Log Only',
            help_text: 'When rate limiting, only log that the query limit has been exceeded. If disabled, the query ' .
                'will be dropped instead.',
        );
        $this->rate_limit = new IntegerField(
            default: 15,
            conditions: ['rate_enabled' => true],
            verbose_name: 'Rate Limit',
            help_text: 'The maximum number of queries per second to allow.',
        );
        $this->bind_forwarder = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'BIND Forwarder',
            help_text: 'Enable forwarding queries to other DNS servers listed below rather than this server ' .
                'performing its own recursion.',
        );
        $this->bind_forwarder_ips = new StringField(
            required: true,
            many: true,
            delimiter: ';',
            conditions: ['bind_forwarder' => true],
            validators: [new IPAddressValidator()],
            verbose_name: 'BIND Forwarder IPs',
            help_text: 'The IP addresses of the DNS servers to forward queries to.',
        );
        $this->bind_dnssec_validation = new StringField(
            default: 'auto',
            choices: ['auto', 'on', 'off'],
            verbose_name: 'BIND DNSSEC Validation',
            help_text: 'Enable DNSSEC validation when BIND is acting as a recursive resolver.',
        );
        $this->listenport = new PortField(
            default: '53',
            verbose_name: 'Listen Port',
            help_text: 'The TCP and UDP port to listen on for DNS requests.',
        );
        $this->controlport = new PortField(
            default: '953',
            verbose_name: 'Control Port',
            help_text: 'The TCP port to listen on for control requests (localhost only).',
        );
        $this->bind_custom_options = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'BIND Custom Options',
            help_text: 'Custom BIND options to include in the configuration file.',
        );
        $this->bind_global_settings = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'BIND Global Settings',
            help_text: 'Global BIND settings to include in the configuration file.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the BIND settings.
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
