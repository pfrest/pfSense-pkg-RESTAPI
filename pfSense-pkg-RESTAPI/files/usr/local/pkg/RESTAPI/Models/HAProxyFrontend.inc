<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model that interacts with HAProxy frontends.
 */
class HAProxyFrontend extends Model {
    public StringField $name;
    public StringField $descr;
    public StringField $status;
    public NestedModelField $a_extaddr;
    public IntegerField $max_connections;
    public StringField $type;
    public NestedModelField $ha_acls;
    public NestedModelField $a_actionitems;
    public ForeignModelField $backend_serverpool;
    public BooleanField $socket_stats;
    public BooleanField $dontlognull;
    public BooleanField $dontlog_normal;
    public BooleanField $log_separate_errors;
    public BooleanField $log_detailed;
    public NestedModelField $a_errorfiles;
    public IntegerField $client_timeout;
    public BooleanField $forwardfor;
    public StringField $httpclose;
    public StringField $advanced_bind;
    public Base64Field $advanced;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/haproxy/ha_backends/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Frontend';

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            minimum_length: 2,
            validators: [new RegexValidator(pattern: '/[^a-zA-Z0-9\.\-_]/', invert: true)],
            help_text: 'The unique name for this HAProxy frontend.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'The description for this HAProxy frontend.',
        );
        $this->status = new StringField(
            default: 'active',
            choices: ['active', 'disabled'],
            help_text: 'The activation status for this HAProxy frontend.',
        );
        $this->a_extaddr = new NestedModelField(
            model_class: 'HAProxyFrontendAddress',
            default: [],
            allow_empty: true,
            help_text: 'The external addresses assigned to this frontend.',
        );
        $this->max_connections = new IntegerField(
            default: null,
            allow_null: true,
            help_text: 'The maximum number of connections allowed by this frontend.',
        );
        $this->type = new StringField(
            required: true,
            choices: ['http', 'https', 'tcp'],
            help_text: 'The processing type for this frontend.',
        );
        $this->ha_acls = new NestedModelField(
            model_class: 'HAProxyFrontendACL',
            default: [],
            allow_empty: true,
            help_text: 'The ACLs to apply to this frontend.',
        );
        $this->a_actionitems = new NestedModelField(
            model_class: 'HAProxyFrontendAction',
            default: [],
            allow_empty: true,
            help_text: 'The actions to take when an ACL match is found.',
        );
        $this->backend_serverpool = new ForeignModelField(
            model_name: 'HAProxyBackend',
            model_field: 'name',
            default: null,
            allow_null: true,
            help_text: 'The default backend to use for this frontend.',
        );
        $this->ssloffloadcert = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            default: null,
            allow_null: true,
            help_text: 'The default SSL/TLS certificate refid to use for this frontend.',
        );
        $this->ha_certificates = new NestedModelField(
            model_class: 'HAProxyFrontendCertificate',
            default: [],
            allow_empty: true,
            help_text: 'The additional SSL/TLS certificates to use on this frontend.',
        );
        $this->socket_stats = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'socket-stats',
            help_text: 'Enables or disables collecting and providing separate statistics for each socket.',
        );
        $this->dontlognull = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables logging connections with no data transferred.',
        );
        $this->dontlog_normal = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'dontlog-normal',
            help_text: 'Enables or disables only logging anomalous (not normal) connection.',
        );
        $this->log_separate_errors = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'log-separate-errors',
            help_text: 'Enables or disables changing the log level from info to err on potentially interesting info.',
        );
        $this->log_detailed = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'log-detailed',
            help_text: 'Enables or disables more detailed logging.',
        );
        $this->a_errorfiles = new NestedModelField(
            model_class: 'HAProxyFrontendErrorFile',
            default: [],
            allow_empty: true,
            help_text: 'The custom error files to use for this frontend.',
        );
        $this->client_timeout = new IntegerField(
            default: 30000,
            allow_null: true,
            help_text: 'The amount of time (in milliseconds) to wait for data from the client.',
        );
        $this->forwardfor = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['type' => 'http'],
            help_text: "Enables or disables the HTTP X-Forwarded-For header which contains the client's IP address.",
        );
        $this->httpclose = new StringField(
            default: 'http-keep-alive',
            choices: ['http-keep-alive', 'http-tunnel', 'httpclose', 'http-server-close', 'forceclose'],
            help_text: 'The `httpclose` option this frontend will operate.',
        );
        $this->advanced_bind = new StringField(
            default: null,
            allow_null: true,
            help_text: 'Custom value to pass behind each bind option.',
        );
        $this->advanced = new Base64Field(
            default: null,
            allow_empty: true,
            allow_null: true,
            help_text: 'Custom configuration to pass to this frontend.',
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
