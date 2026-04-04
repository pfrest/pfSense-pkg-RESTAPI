<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\EmailAddressValidator;
use RESTAPI\Validators\HostnameValidator;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for interacting with HAProxy backends.
 */
class HAProxyBackend extends Model {
    public StringField $name;
    public NestedModelField $servers;
    public StringField $balance;
    public IntegerField $balance_urilen;
    public IntegerField $balance_uridepth;
    public BooleanField $balance_uriwhole;
    public NestedModelField $acls;
    public NestedModelField $actions;
    public IntegerField $connection_timeout;
    public IntegerField $server_timeout;
    public IntegerField $retries;
    public StringField $check_type;
    public IntegerField $checkinter;
    public BooleanField $log_health_checks;
    public StringField $httpcheck_method;
    public StringField $monitor_uri;
    public StringField $monitor_httpversion;
    public StringField $monitor_username;
    public StringField $monitor_domain;
    public BooleanField $agent_checks;
    public PortField $agent_port;
    public IntegerField $agent_inter;
    public BooleanField $persist_cookie_enabled;
    public StringField $persist_cookie_name;
    public StringField $persist_cookie_mode;
    public BooleanField $persist_cookie_cachable;
    public BooleanField $persist_cookie_postonly;
    public BooleanField $persist_cookie_httponly;
    public BooleanField $persist_cookie_secure;
    public IntegerField $haproxy_cookie_maxidle;
    public IntegerField $haproxy_cookie_maxlife;
    public StringField $haproxy_cookie_domains;
    public StringField $haproxy_cookie_dynamic_cookie_key;
    public StringField $persist_sticky_type;
    public StringField $persist_stick_expire;
    public StringField $persist_stick_tablesize;
    public StringField $persist_stick_cookiename;
    public IntegerField $persist_stick_length;
    public StringField $email_level;
    public StringField $email_to;
    public BooleanField $stats_enabled;
    public StringField $stats_uri;
    public StringField $stats_scope;
    public StringField $stats_realm;
    public StringField $stats_username;
    public StringField $stats_password;
    public StringField $stats_admin;
    public StringField $stats_node;
    public StringField $stats_desc;
    public IntegerField $stats_refresh;
    public IntegerField $strict_transport_security;
    public NestedModelField $errorfiles;
    public BooleanField $cookie_attribute_secure;
    public StringField $advanced;
    public Base64Field $advanced_backend;
    public BooleanField $transparent_clientip;
    public InterfaceField $transparent_interface;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/haproxy/ha_pools/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Backend';

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [new RegexValidator(pattern: '/[^a-zA-Z0-9\.\-_]/', invert: true)],
            help_text: 'The unique name for this backend.',
        );
        $this->servers = new NestedModelField(
            model_class: 'HAProxyBackendServer',
            default: [],
            allow_empty: true,
            help_text: 'The pool of servers this backend will use.',
        );
        $this->balance = new StringField(
            default: '',
            choices: ['', 'roundrobin', 'static-rr', 'leastconn', 'source', 'uri'],
            allow_empty: true,
            help_text: 'The load balancing option to use for servers assigned to this backend.',
        );
        $this->balance_urilen = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['balance' => 'uri'],
            help_text: 'The number of URI characters the algorithm should consider when hashing.',
        );
        $this->balance_uridepth = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['balance' => 'uri'],
            help_text: 'The maximum directory depth to be used to compute the hash. One level is counted for each ' .
                'slash in the request.',
        );
        $this->balance_uriwhole = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['balance' => 'uri'],
            help_text: 'Enables or disables allowing the use of whole URIs, including URL parameters.',
        );
        $this->acls = new NestedModelField(
            model_class: 'HAProxyBackendACL',
            default: [],
            allow_empty: true,
            help_text: 'The ACLs to apply to this backend.',
        );
        $this->actions = new NestedModelField(
            model_class: 'HAProxyBackendAction',
            default: [],
            allow_empty: true,
            help_text: 'The actions to apply to this backend.',
        );
        $this->connection_timeout = new IntegerField(
            default: 30000,
            allow_null: true,
            help_text: 'The amount of time (in milliseconds) to wait before giving up on connections.',
        );
        $this->server_timeout = new IntegerField(
            default: 30000,
            allow_null: true,
            help_text: 'The amount of time (in milliseconds) to wait for data transferred to or from the server.',
        );
        $this->retries = new IntegerField(
            default: null,
            allow_null: true,
            help_text: 'The number of retry attempts to allow after a connection failure to the server.',
        );
        $this->check_type = new StringField(
            default: 'none',
            choices: ['none', 'Basic', 'HTTP', 'LDAP', 'MySQL', 'PostgreSQL', 'Redis', 'SMTP', 'ESMTP', 'SSL'],
            help_text: 'The health check method to use when checking the health of backend servers.',
        );
        $this->checkinter = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['!check_type' => 'none'],
            help_text: 'The interval (in milliseconds) in which health checks will be performed.',
        );
        $this->log_health_checks = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['!check_type' => 'none'],
            help_text: 'Enables or disables logging changes to the health check status',
        );
        $this->httpcheck_method = new StringField(
            default: 'OPTIONS',
            choices: ['OPTIONS', 'HEAD', 'GET', 'POST', 'PUT', 'DELETE', 'TRACE'],
            conditions: ['check_type' => 'HTTP'],
            help_text: 'The HTTP method to use for HTTP health checks.',
        );
        $this->monitor_uri = new StringField(
            default: '/',
            allow_empty: true,
            conditions: ['check_type' => 'HTTP'],
            help_text: 'The URL to use for HTTP health checks.',
        );
        $this->monitor_httpversion = new StringField(
            default: 'HTTP/1.0',
            allow_empty: true,
            conditions: ['check_type' => 'HTTP'],
            help_text: 'The HTTP version to use for HTTP health checks.',
        );
        $this->monitor_username = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['check_type' => ['MySQL', 'PostgreSQL']],
            help_text: 'The username to use for MySQL or PostgreSQL health checks.',
        );
        $this->monitor_domain = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['check_type' => ['SMTP', 'ESMTP']],
            validators: [new HostnameValidator(allow_hostname: true, allow_domain: true, allow_fqdn: true)],
            help_text: 'The domain to use for SMTP or ESMTP health checks.',
        );
        $this->agent_checks = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables using a TCP connection to read an ASCII string of the form.',
        );
        $this->agent_port = new PortField(
            required: true,
            allow_alias: false,
            allow_range: false,
            conditions: ['agent_checks' => true],
        );
        $this->agent_inter = new IntegerField(
            default: 2000,
            allow_null: true,
            conditions: ['agent_checks' => true],
            help_text: 'The interval (in milliseconds) between agent checks.',
        );
        $this->persist_cookie_enabled = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables cookie based persistence.',
        );
        $this->persist_cookie_name = new StringField(
            required: true,
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'The string name to track in Set-Cookie and Cookie HTTP headers.',
        );
        $this->persist_cookie_mode = new StringField(
            default: 'passive',
            choices: [
                'passive',
                'passive-silent',
                'reset',
                'set',
                'set-silent',
                'insert-only',
                'insert-only-silent',
                'session-prefix',
                'passive-session-prefix',
            ],
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'The mode HAProxy uses to insert/prefix/replace or examine cookie and set-cookie headers.',
        );
        $this->persist_cookie_cachable = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'Enables or disables allowing shared caches to cache the server response.',
        );
        $this->persist_cookie_postonly = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'Enables or disables only inserting cookies on POST requests.',
        );
        $this->persist_cookie_httponly = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'Enables or disables preventing the use of cookies with non-HTTP components.',
        );
        $this->persist_cookie_secure = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'Enables or disables prevention of cookie usage over non-secure channels.',
        );
        $this->haproxy_cookie_maxidle = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'The max-idle time to allow. This option only applies to insert mode cookies.',
        );
        $this->haproxy_cookie_maxlife = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'The max-life time to allow. This option only applies to insert mode cookies.',
        );
        $this->haproxy_cookie_domains = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: ' ',
            conditions: ['persist_cookie_enabled' => true],
            validators: [new HostnameValidator(allow_hostname: true, allow_domain: true, allow_fqdn: true)],
            help_text: 'The domains to set the cookies for.',
        );
        $this->haproxy_cookie_dynamic_cookie_key = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['persist_cookie_enabled' => true],
            help_text: 'The dynamic cookie secret key. This is will be used to generate dynamic cookies ' .
                'for this backend.',
        );
        $this->persist_sticky_type = new StringField(
            default: 'none',
            choices: [
                'none',
                'stick_sslsessionid',
                'stick_sourceipv4',
                'stick_sourceipv6',
                'stick_cookie_value',
                'stick_rdp_cookie',
            ],
            help_text: 'The sticky table mode to use for this backend. These options are used to make sure subsequent ' .
                'requests from a single client go to the same backend.',
        );
        $this->persist_stick_expire = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['!persist_sticky_type' => 'none'],
            help_text: 'The maximum duration of an entry in the stick-table since it was last created, ' .
                'refreshed or matched.',
        );
        $this->persist_stick_tablesize = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['!persist_sticky_type' => 'none'],
            help_text: 'The maximum number of entries allowed in the table. This value directly impacts memory usage.',
        );
        $this->persist_stick_cookiename = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['persist_sticky_type' => ['stick_cookie_value', 'stick_rdp_cookie']],
            help_text: 'The cookie name to use for stick table.',
        );
        $this->persist_stick_length = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['persist_sticky_type' => ['stick_cookie_value', 'stick_rdp_cookie']],
            help_text: 'The maximum number of characters allowed in a string type stick table',
        );
        $this->email_level = new StringField(
            default: '',
            choices: ['', 'dontlog', 'emerg', 'alert', 'crit', 'err', 'warning', 'notice', 'info', 'debug'],
            allow_empty: true,
            help_text: 'The maximum log level to send emails for. Leave empty to disable sending email alerts. If ' .
                'left empty, the value set in the global settings will be used.',
        );
        $this->email_to = new StringField(
            default: '',
            allow_empty: true,
            validators: [new EmailAddressValidator()],
            help_text: 'The email address to send emails to. If left empty, the value set in the ' .
                'global settings will be used.',
        );
        $this->stats_enabled = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables the HAProxy statistics page for this backend.',
        );
        $this->stats_uri = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The statistics URL for this backend.',
        );
        $this->stats_scope = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: ',',
            conditions: ['stats_enabled' => true],
            help_text: 'The frontends and backends stats to be shown, leave empty to show all.',
        );
        $this->stats_realm = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The realm that is shown when authentication is requested by HAProxy.',
        );
        $this->stats_username = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The stats page username',
        );
        $this->stats_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The stats page password.',
        );
        $this->stats_admin = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The admin to make use of the options disable/enable/softstop/softstart/killsessions ' .
                'from the stats page.',
        );
        $this->stats_node = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The short name displayed in stats and helps differentiate which server in the ' .
                'cluster is actually serving clients.',
        );
        $this->stats_desc = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['stats_enabled' => true],
            help_text: 'The verbose description for this node.',
        );
        $this->stats_refresh = new IntegerField(
            default: 10,
            conditions: ['stats_enabled' => true],
            help_text: 'The interval (in seconds) in which the stats page is refreshed.',
        );
        $this->errorfiles = new NestedModelField(
            model_class: 'HAProxyBackendErrorFile',
            default: [],
            allow_empty: true,
            help_text: 'The HAProxy error file mappings to use for this backend.',
        );
        $this->strict_transport_security = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            maximum: 1000000000,
            help_text: 'The HSTS validity period for this backend. Leave empty to disable HSTS.',
        );
        $this->cookie_attribute_secure = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables assigning the secure attributes on cookies for this backend.',
        );
        $this->advanced = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'The per server pass thru to apply to each server line.',
        );
        $this->advanced_backend = new Base64Field(
            default: '',
            allow_empty: true,
            help_text: 'The backend pass thru to apply to the backend section.',
        );
        $this->transparent_clientip = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables using the client-IP to connect to backend servers.',
        );
        $this->transparent_interface = new InterfaceField(
            default: null,
            allow_null: true,
            conditions: ['transparent_clientip' => true],
            help_text: 'The interface that will connect to the backend server.',
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
