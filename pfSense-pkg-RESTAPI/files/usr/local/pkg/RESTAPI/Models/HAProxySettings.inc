<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\EmailAddressValidator;
use RESTAPI\Validators\HostnameValidator;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that interacts with HAProxy settings.
 */
class HAProxySettings extends Model {
    public BooleanField $enable;
    public IntegerField $maxconn;
    public IntegerField $nbthread;
    public BooleanField $terminate_on_reload;
    public StringField $hard_stop_after;
    public ForeignModelField $carpdev;
    public PortField $localstatsport;
    public IntegerField $localstats_refreshtime;
    public IntegerField $localstats_sticktable_refreshtime;
    public StringField $remotesyslog;
    public StringField $logfacility;
    public StringField $loglevel;
    public StringField $log_send_hostname;
    public NestedModelField $dns_resolvers;
    public IntegerField $resolver_retries;
    public StringField $resolver_timeoutretry;
    public StringField $resolver_holdvalid;
    public NestedModelField $email_mailers;
    public StringField $email_level;
    public StringField $email_myhostname;
    public StringField $email_from;
    public StringField $email_to;
    public StringField $sslcompatibilitymode;
    public IntegerField $ssldefaultdhparam;
    public Base64Field $advanced;
    public BooleanField $enablesync;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/haproxy';
        $this->many = false;
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->verbose_name = 'HAProxy Settings';

        # Set model fields
        $this->enable = new BooleanField(default: false, verbose_name: 'Enable', help_text: 'Enables or disable HAProxy on the system.');
        $this->maxconn = new IntegerField(
            default: null,
            allow_null: true,
            verbose_name: 'Max Connections',
            help_text: 'The maximum per-process number of concurrent connections',
        );
        $this->nbthread = new IntegerField(
            default: 1,
            verbose_name: 'Number of Threads',
            help_text: 'The number of threads to start per process. This setting is experimental.',
        );
        $this->terminate_on_reload = new BooleanField(
            default: false,
            verbose_name: 'Terminate On Reload',
            help_text: 'Enables or disables an immediate stop of old process on reload. (closes existing connections)',
        );
        $this->hard_stop_after = new StringField(
            default: '15m',
            allow_empty: true,
            verbose_name: 'Hard Stop After',
            help_text: 'The maximum time allowed to perform a clean soft-stop. This can be represented as different ' .
                'time values such as 30s, 15m, 3h or 1d.',
        );
        $this->carpdev = new ForeignModelField(
            model_name: 'VirtualIP',
            model_field: 'subnet',
            model_query: ['mode' => 'carp'],
            allow_null: true,
            verbose_name: 'CARP Device',
            help_text: 'The CARP interface IP to monitor. HAProxy will only run on the firewall ' .
                'whose interface is MASTER.',
        );
        $this->localstatsport = new PortField(
            default: null,
            allow_alias: false,
            allow_range: false,
            allow_null: true,
            verbose_name: 'Local Stats Port',
            help_text: 'The internal port to be used for the stats tab. Set to null to disable local stats.',
        );
        $this->localstats_refreshtime = new IntegerField(
            default: null,
            allow_null: true,
            verbose_name: 'Local Stats Refresh Time',
            help_text: 'The internal (in seconds) in which local stats will be refreshed.',
        );
        $this->localstats_sticktable_refreshtime = new IntegerField(
            default: null,
            allow_null: true,
            verbose_name: 'Stick Table Refresh Time',
            help_text: 'The internal (in seconds) in which the sticktable stats will be refreshed.',
        );
        $this->remotesyslog = new StringField(
            default: null,
            allow_null: true,
            validators: [
                new IPAddressValidator(
                    allow_ipv4: true,
                    allow_ipv6: true,
                    allow_fqdn: true,
                    allow_keywords: ['/var/run/log'],
                ),
            ],
            verbose_name: 'Remote Syslog',
            help_text: 'The IP address or hostname of the remote syslog server to send logs to. Use `/var/run/log` to ' .
                'to log to the local pfSense system log.',
        );
        $this->logfacility = new StringField(
            default: 'syslog',
            choices: [
                'kern',
                'user',
                'mail',
                'daemon',
                'auth',
                'syslog',
                'lpr',
                'news',
                'uucp',
                'cron',
                'auth2',
                'ftp',
                'ntp',
                'audit',
                'cron2',
                'local0',
                'local1',
                'local2',
                'local3',
                'local4',
                'local5',
                'local6',
                'local7',
            ],
            verbose_name: 'Log Facility',
            help_text: 'The logging facility to log to.',
        );
        $this->loglevel = new StringField(
            default: 'warning',
            choices: ['emerg', 'alert', 'crit', 'err', 'warning', 'notice', 'info', 'debug'],
            verbose_name: 'Log Level',
            help_text: 'The log level to begin logging events. Only events of this level or higher will be logged.',
        );
        $this->log_send_hostname = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'log-send-hostname',
            validators: [new HostnameValidator()],
            verbose_name: 'Log Send Hostname',
            help_text: 'The hostname field to include in the syslog header. Leave empty to use the system hostname.',
        );
        $this->dns_resolvers = new NestedModelField(
            model_class: 'HAProxyDNSResolver',
            default: [],
            allow_empty: true,
            verbose_name: 'DNS Resolvers',
            help_text: 'The DNS resolvers HAProxy will use for DNS queries.',
        );
        $this->resolver_retries = new IntegerField(
            default: 3,
            verbose_name: 'Resolver Retries',
            help_text: 'The number of queries to send to resolve a server name before giving up.',
        );
        $this->resolver_timeoutretry = new StringField(
            default: '1s',
            verbose_name: 'Resolver Timeout Retry',
            help_text: 'The time between two DNS queries, when no response have been received.',
        );
        $this->resolver_holdvalid = new StringField(
            default: '1s',
            verbose_name: 'Resolver Hold Valid',
            help_text: 'The interval between two successive name resolution when the last answer was valid.',
        );
        $this->email_mailers = new NestedModelField(
            model_class: 'HAProxyEmailMailer',
            default: [],
            allow_empty: true,
            verbose_name: 'Email Mailers',
            help_text: 'The email servers HAProxy will use to send SMTP alerts.',
        );
        $this->email_level = new StringField(
            default: '',
            choices: ['', 'emerg', 'alert', 'crit', 'err', 'warning', 'notice', 'info', 'debug'],
            allow_empty: true,
            verbose_name: 'Email Level',
            help_text: 'The maximum log level to send emails for. Leave empty to disable sending email alerts.',
        );
        $this->email_myhostname = new StringField(
            default: '',
            allow_empty: true,
            validators: [new HostnameValidator()],
            verbose_name: 'Email Hostname',
            help_text: 'The hostname to use as the origin of the email.',
        );
        $this->email_from = new StringField(
            default: '',
            allow_empty: true,
            validators: [new EmailAddressValidator()],
            verbose_name: 'Email From',
            help_text: 'The email address to be used as the sender of the emails.',
        );
        $this->email_to = new StringField(
            default: '',
            allow_empty: true,
            validators: [new EmailAddressValidator()],
            verbose_name: 'Email To',
            help_text: 'The email address to send emails to.',
        );
        $this->sslcompatibilitymode = new StringField(
            default: 'auto',
            choices: ['auto', 'modern', 'intermediate', 'old'],
            verbose_name: 'SSL Compatibility Mode',
            help_text: 'The SSL/TLS compatibility mode which determines the cipher suites and TLS versions supported.',
        );
        $this->ssldefaultdhparam = new IntegerField(
            default: 1024,
            minimum: 1024,
            verbose_name: 'SSL Default DH Param',
            help_text: 'The maximum size of the Diffie-Hellman parameters used for generating the ephemeral/temporary ' .
                'Diffie-Hellman key in case of DHE key exchange',
        );
        $this->advanced = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'Advanced',
            help_text: 'Additional HAProxy options to include in the global settings area.',
        );
        $this->enablesync = new BooleanField(
            default: false,
            verbose_name: 'Enable Sync',
            help_text: 'Enables or disables including HAProxy configurations in HA sync if configured.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Ensures haproxy is marked as dirty before applying.
     */
    public function pre_apply(): void {
        touch('/var/run/haproxy.conf.dirty');
    }

    /**
     * Applies changes to the HAProxy configuration.
     */
    public function apply(): void {
        (new HAProxyApplyDispatcher(async: $this->async))->spawn_process();
    }
}
