<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for HAProxy Frontend Access Control Lists.
 */
class HAProxyFrontendACL extends Model {
    public StringField $name;
    public StringField $expression;
    public StringField $value;
    public BooleanField $casesensitive;
    public BooleanField $not;

    /**
     * Constructs the HAProxy frontend ACL Model and its fields.
     */
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyFrontend';
        $this->config_path = 'ha_acls/item';
        $this->verbose_name = 'HAProxy Frontend Access Control List';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            minimum_length: 2,
            verbose_name: 'Name',
            validators: [new RegexValidator(pattern: '/[^a-zA-Z0-9\.\-_]/', invert: true)],
            help_text: 'The unique name for this frontend ACL.',
        );
        $this->expression = new StringField(
            required: true,
            choices: [
                'host_starts_with',
                'host_ends_with',
                'host_matches',
                'host_regex',
                'host_contains',
                'path_starts_with',
                'path_ends_with',
                'path_matches',
                'path_regex',
                'path_contains',
                'path_dir',
                'url_parameter',
                'ssl_c_verify_code',
                'ssl_c_verify',
                'ssl_c_ca_commonname',
                'source_ip',
                'backendservercount',
                'traffic_is_http',
                'traffic_is_ssl',
                'ssl_sni_matches',
                'ssl_sni_contains',
                'ssl_sni_starts_with',
                'ssl_sni_ends_with',
                'ssl_sni_regex',
                'custom',
            ],
            verbose_name: 'Expression',
            help_text: 'The expression to use to determine the match for this ACL.',
        );
        $this->value = new StringField(
            required: true,
            allow_empty: true,
            verbose_name: 'Value',
            help_text: 'The value which indicates a match for this ACL.',
        );
        $this->casesensitive = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Case Sensitive',
            help_text: 'Enables or disables case-sensitive matching for this ACL.',
        );
        $this->not = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Not',
            help_text: 'Enables or disables inverting the context of this ACL.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Ensures the expression is supported by the parent frontend's processing type.
     * @param string $expression The incoming ACL expression.
     * @return string The validated expression.
     * @throws ValidationError When the expression is incompatible with the parent frontend type.
     */
    public function validate_expression(string $expression): string {
        $type = $this->parent_model->type->value;
        $supported_expressions = [
            'http' => [
                'host_starts_with',
                'host_ends_with',
                'host_matches',
                'host_regex',
                'host_contains',
                'path_starts_with',
                'path_ends_with',
                'path_matches',
                'path_regex',
                'path_contains',
                'path_dir',
                'url_parameter',
                'ssl_c_verify_code',
                'ssl_c_verify',
                'ssl_c_ca_commonname',
                'source_ip',
                'backendservercount',
                'custom',
            ],
            'https' => [
                'source_ip',
                'backendservercount',
                'ssl_sni_matches',
                'ssl_sni_contains',
                'ssl_sni_starts_with',
                'ssl_sni_ends_with',
                'ssl_sni_regex',
                'custom',
            ],
            'tcp' => ['source_ip', 'backendservercount', 'traffic_is_http', 'traffic_is_ssl', 'custom'],
        ];

        if (!in_array($expression, $supported_expressions[$type] ?? [], true)) {
            throw new ValidationError(
                message: "Expression `$expression` is not supported by parent frontend type `$type`.",
                response_id: 'HAPROXY_FRONTEND_ACL_EXPRESSION_TYPE_MISMATCH',
            );
        }

        return $expression;
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
