<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model that interacts with HAProxy frontends.
 */
class HAProxyFrontend extends Model {
    public StringField $name;
    public StringField $descr;
    public StringField $status;
    public BooleanField $secondary;
    public StringField $primary_frontend;
    public BooleanField $ssloffload;
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

    /**
     * Defines the frontend fields and native HAProxy configuration mapping.
     * @param mixed $id The frontend ID, if reading or updating an existing frontend.
     * @param mixed $parent_id The parent ID accepted by the Model interface.
     * @param mixed $data The frontend representation.
     * @param mixed ...$options Additional Model options.
     */
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
            verbose_name: 'Name',
            help_text: 'The unique name for this HAProxy frontend.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The description for this HAProxy frontend.',
        );
        $this->status = new StringField(
            default: 'active',
            choices: ['active', 'disabled'],
            verbose_name: 'Status',
            help_text: 'The activation status for this HAProxy frontend.',
        );
        $this->secondary = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'Shared Frontend',
            help_text: 'Merge this frontend into an existing primary frontend, sharing its listening addresses and mode.',
        );
        # ForeignModelField eagerly constructs its target Model, so a self-reference would recurse.
        $this->primary_frontend = new StringField(
            default: null,
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Primary Frontend',
            help_text: 'The name of an existing non-shared frontend. Required when secondary is true. ' .
                'When replacing all frontends, include the primary before its shared frontends.',
        );
        $this->ssloffload = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'Shared Frontend SSL Offloading',
            help_text: 'Use this shared frontend\'s additional certificate settings. ' .
                'This does not enable TLS on the primary frontend\'s listening addresses.',
        );
        $this->a_extaddr = new NestedModelField(
            model_class: 'HAProxyFrontendAddress',
            default: [],
            allow_empty: true,
            verbose_name: 'External Addresses',
            help_text: 'The external addresses assigned to this frontend.',
        );
        $this->max_connections = new IntegerField(
            default: null,
            allow_null: true,
            verbose_name: 'Max Connections',
            help_text: 'The maximum number of connections allowed by this frontend.',
        );
        $this->type = new StringField(
            default: null,
            allow_null: true,
            conditions: ['secondary' => false],
            choices: ['http', 'https', 'tcp'],
            verbose_name: 'Type',
            help_text: 'The processing type for this frontend. Shared frontends inherit their primary frontend\'s type.',
        );
        $this->ha_acls = new NestedModelField(
            model_class: 'HAProxyFrontendACL',
            default: [],
            allow_empty: true,
            verbose_name: 'ACLs',
            help_text: 'The ACLs to apply to this frontend.',
        );
        $this->a_actionitems = new NestedModelField(
            model_class: 'HAProxyFrontendAction',
            default: [],
            allow_empty: true,
            verbose_name: 'Action Items',
            help_text: 'The actions to take when an ACL match is found.',
        );
        $this->backend_serverpool = new ForeignModelField(
            model_name: 'HAProxyBackend',
            model_field: 'name',
            default: null,
            allow_null: true,
            verbose_name: 'Backend Server Pool',
            help_text: 'The default backend to use for this frontend.',
        );
        $this->ssloffloadcert = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            default: null,
            allow_null: true,
            verbose_name: 'SSL Offload Certificate',
            help_text: 'The default SSL/TLS certificate refid to use for this frontend.',
        );
        $this->ha_certificates = new NestedModelField(
            model_class: 'HAProxyFrontendCertificate',
            default: [],
            allow_empty: true,
            verbose_name: 'Certificates',
            help_text: 'The additional SSL/TLS certificates to use on this frontend.',
        );
        $this->socket_stats = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'socket-stats',
            verbose_name: 'Socket Stats',
            help_text: 'Enables or disables collecting and providing separate statistics for each socket.',
        );
        $this->dontlognull = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            verbose_name: 'Don\'t Log Null',
            help_text: 'Enables or disables logging connections with no data transferred.',
        );
        $this->dontlog_normal = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'dontlog-normal',
            verbose_name: 'Don\'t Log Normal',
            help_text: 'Enables or disables only logging anomalous (not normal) connection.',
        );
        $this->log_separate_errors = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'log-separate-errors',
            verbose_name: 'Log Separate Errors',
            help_text: 'Enables or disables changing the log level from info to err on potentially interesting info.',
        );
        $this->log_detailed = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            internal_name: 'log-detailed',
            verbose_name: 'Log Detailed',
            help_text: 'Enables or disables more detailed logging.',
        );
        $this->a_errorfiles = new NestedModelField(
            model_class: 'HAProxyFrontendErrorFile',
            default: [],
            allow_empty: true,
            verbose_name: 'Error Files',
            help_text: 'The custom error files to use for this frontend.',
        );
        $this->client_timeout = new IntegerField(
            default: 30000,
            allow_null: true,
            verbose_name: 'Client Timeout',
            help_text: 'The amount of time (in milliseconds) to wait for data from the client.',
        );
        $this->forwardfor = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            conditions: ['type' => 'http'],
            verbose_name: 'Forward For',
            help_text: "Enables or disables the HTTP X-Forwarded-For header which contains the client's IP address.",
        );
        $this->httpclose = new StringField(
            default: 'http-keep-alive',
            choices: ['http-keep-alive', 'http-tunnel', 'httpclose', 'http-server-close', 'forceclose'],
            verbose_name: 'HTTP Close',
            help_text: 'The `httpclose` option this frontend will operate.',
        );
        $this->advanced_bind = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'Advanced Bind Config',
            help_text: 'Custom value to pass behind each bind option.',
        );
        $this->advanced = new Base64Field(
            default: null,
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Advanced',
            help_text: 'Custom configuration to pass to this frontend.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Validates shared relationships without changing other frontends during validation or dry runs.
     * @throws ValidationError When a shared frontend has an invalid primary or an update would orphan its children.
     */
    public function validate_extra(): void {
        $original_name = $this->initial_object?->name->value;
        if (!$this->secondary->value and $this->type->value === null) {
            throw new ValidationError(message: 'Field `type` is required.', response_id: 'FIELD_IS_REQUIRED');
        }
        if ($this->secondary->value) {
            if (
                !$this->primary_frontend->value or
                $this->primary_frontend->value === $this->name->value or
                $original_name !== null and $this->primary_frontend->value === $original_name or
                !$this->query(query_params: ['name' => $this->primary_frontend->value, 'secondary' => false])->exists()
            ) {
                throw new ValidationError(
                    message: 'A shared frontend must reference an existing, distinct, non-shared primary frontend.',
                    response_id: 'HAPROXY_FRONTEND_INVALID_PRIMARY_FRONTEND',
                );
            }
        }

        # Reparent shared frontends before renaming their primary or converting it into another shared frontend.
        if ($original_name and ($this->name->value !== $original_name or $this->secondary->value)) {
            $children = $this->query(query_params: ['secondary' => true, 'primary_frontend' => $original_name]);
            if ($children->exists()) {
                throw new ValidationError(
                    message: 'A primary frontend referenced by shared frontends cannot be renamed or made shared.',
                    response_id: 'HAPROXY_FRONTEND_PRIMARY_IN_USE',
                );
            }
        }
    }

    /**
     * Includes shared frontends in the standard protection against deleting referenced objects.
     * @return ModelSet The objects referencing this frontend.
     */
    public function get_model_references(): ModelSet {
        $references = parent::get_model_references();
        # Primaries created in the GUI may retain their own name in primary_frontend; these are not references.
        $name = $this->initial_object?->name->value ?? $this->name->value;
        $children = $this->query(query_params: ['secondary' => true, 'primary_frontend' => $name]);
        $references->model_objects = array_merge($references->model_objects, $children->model_objects);
        return $references;
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
