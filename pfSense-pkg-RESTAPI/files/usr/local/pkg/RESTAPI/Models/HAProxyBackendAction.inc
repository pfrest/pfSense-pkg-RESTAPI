<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interacting with HAProxy backend actions.
 */
class HAProxyBackendAction extends Model {
    public StringField $action;
    public StringField $acl;
    public StringField $server;
    public StringField $customaction;
    public StringField $deny_status;
    public StringField $realm;
    public StringField $rule;
    public StringField $lua_function;
    public StringField $name;
    public StringField $fmt;
    public StringField $find;
    public StringField $replace;
    public StringField $path;
    public StringField $status;
    public StringField $reason;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyBackend';
        $this->config_path = 'a_actionitems/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Backend Action';

        # Set model fields
        $this->action = new StringField(
            required: true,
            choices: [
                'use_server',
                'custom',
                'http-request_allow',
                'http-request_deny',
                'http-request_tarpit',
                'http-request_auth',
                'http-request_redirect',
                'http-request_lua',
                'http-request_use-service',
                'http-request_add-header',
                'http-request_set-header',
                'http-request_del-header',
                'http-request_replace-header',
                'http-request_replace-path',
                'http-request_replace-value',
                'http-request_set-method',
                'http-request_set-path',
                'http-request_set-query',
                'http-request_set-uri',
                'http-response_allow',
                'http-response_deny',
                'http-response_lua',
                'http-response_add-header',
                'http-response_set-header',
                'http-response_del-header',
                'http-response_replace-header',
                'http-response_replace-value',
                'http-response_set-status',
                'http-after-response_add-header',
                'http-after-response_set-header',
                'http-after-response_del-header',
                'http-after-response_replace-header',
                'http-after-response_replace-value',
                'http-after-response_set-status',
                'tcp-request_connection_accept',
                'tcp-request_connection_reject',
                'tcp-request_content_accept',
                'tcp-request_content_reject',
                'tcp-request_content_lua',
                'tcp-request_content_use-service',
                'tcp-response_content_accept',
                'tcp-response_content_close',
                'tcp-response_content_reject',
                'tcp-response_content_lua',
            ],
            help_text: 'The action to take when an ACL match is found.',
        );
        $this->acl = new StringField(
            required: true,
            help_text: 'The name of the backend ACL this action is associated with.',
        );
        $this->server = new StringField(
            required: true,
            conditions: ['action' => 'use_server'],
            help_text: 'The backend server to use when an ACL match is found.',
        );
        $this->customaction = new StringField(
            required: true,
            conditions: ['action' => 'custom'],
            help_text: 'The custom action to take when an ACL match is found.',
        );
        $this->deny_status = new StringField(
            required: true,
            conditions: ['action' => ['http-request_deny', 'http-request_tarpit']],
            help_text: 'The deny status to use when an ACL match is found.',
        );
        $this->realm = new StringField(
            required: true,
            conditions: ['action' => 'http-request_auth'],
            help_text: 'The authentication realm to use when an ACL match is found.',
        );
        $this->rule = new StringField(
            required: true,
            conditions: ['action' => 'http-request_redirect'],
            help_text: 'The redirect rule to use when an ACL match is found.',
        );
        $this->lua_function = new StringField(
            required: true,
            conditions: [
                'action' => [
                    'http-request_lua',
                    'http-request_use-service',
                    'http-response_lua',
                    'tcp-request_content_lua',
                    'tcp-request_content_use-service',
                    'tcp-response_content_lua',
                ],
            ],
            help_text: 'The Lua function to use when an ACL match is found.',
        );
        $this->name = new StringField(
            required: true,
            conditions: [
                'action' => [
                    'http-request_add-header',
                    'http-request_set-header',
                    'http-request_del-header',
                    'http-request_replace-header',
                    'http-request_replace-value',
                    'http-response_add-header',
                    'http-response_set-header',
                    'http-response_del-header',
                    'http-response_replace-header',
                    'http-response_replace-value',
                    'http-after-response_add-header',
                    'http-after-response_set-header',
                    'http-after-response_del-header',
                    'http-after-response_replace-header',
                    'http-after-response_replace-value',
                ],
            ],
            help_text: 'The name to use when an ACL match is found.',
        );
        $this->fmt = new StringField(
            required: true,
            conditions: [
                'action' => [
                    'http-request_add-header',
                    'http-request_set-header',
                    'http-request_set-method',
                    'http-request_set-path',
                    'http-request_set-query',
                    'http-request_set-uri',
                    'http-response_add-header',
                    'http-response_set-header',
                    'http-after-response_add-header',
                    'http-after-response_set-header',
                ],
            ],
            help_text: 'The fmt value to use when an ACL match is found.',
        );
        $this->find = new StringField(
            required: true,
            conditions: [
                'action' => [
                    'http-request_replace-header',
                    'http-request_replace-value',
                    'http-response_replace-header',
                    'http-request_replace-path',
                    'http-response_replace-value',
                    'http-after-response_replace-header',
                    'http-after-response_replace-value',
                ],
            ],
            help_text: 'The value to find when an ACL match is found.',
        );
        $this->replace = new StringField(
            required: true,
            conditions: [
                'action' => [
                    'http-request_replace-header',
                    'http-request_replace-value',
                    'http-request_replace-path',
                    'http-response_replace-header',
                    'http-response_replace-value',
                    'http-after-response_replace-header',
                    'http-after-response_replace-value',
                ],
            ],
            help_text: 'The value to replace with when an ACL match is found.',
        );
        $this->path = new StringField(
            required: true,
            conditions: ['action' => 'http-request_replace-path'],
            help_text: 'The path to use when an ACL match is found.',
        );
        $this->status = new StringField(
            required: true,
            conditions: ['action' => ['http-response_set-status', 'http-after-response_set-status']],
            help_text: 'The status to use when an ACL match is found.',
        );
        $this->reason = new StringField(
            required: true,
            conditions: ['action' => ['http-response_set-status', 'http-after-response_set-status']],
            help_text: 'The status reason to use when an ACL match is found.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Extends the default `to_internal()` method to automatically set the internal names of fields before calling.
     * @return array The internal representation of this model.
     */
    public function to_internal(): array {
        $this->set_field_internal_names();
        return parent::to_internal();
    }

    /**
     * Extends the default `from_internal_object()` method to automatically set the internal names of fields before calling.
     * @param array $internal_object The internal representation of this model to load from.
     */
    public function from_internal_object(mixed $internal_object): void {
        # Do an initial load of the object to ensure the `action` is obtained first
        $this->action->value = $internal_object['action'];

        # Set the internal names of all fields before doing another load to include dynamic field names
        $this->set_field_internal_names();
        parent::from_internal_object($internal_object);
    }

    /**
     * Assigns action metrics/value fields' internal name based on the current `action` selected. This is necessary
     * to avoid a bunch of unnecessary fields with confusing names for example http-request_denydenystatus.
     */
    private function set_field_internal_names(): void {
        # Variables
        $fields = $this->get_fields();
        $exempt_fields = ['action', 'acl'];

        # Loop through each field and change it's internal name to be prefixed with the current `action` assigned
        foreach ($fields as $field) {
            # Don't change exempt field's internal names
            if (!in_array($field, $exempt_fields) and $this->action->value and $this->$field) {
                $corrected_field = str_replace('lua_function', 'lua-function', $field);
                $this->$field->internal_name = $this->action->value . $corrected_field;
            }
        }
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
