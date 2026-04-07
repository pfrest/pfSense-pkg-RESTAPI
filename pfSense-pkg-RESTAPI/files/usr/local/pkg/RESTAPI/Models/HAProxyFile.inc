<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for HAProxy Files.
 */
class HAProxyFile extends Model {
    public StringField $name;
    public StringField $type;
    public Base64Field $content;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/haproxy/files/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy File';

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [new RegexValidator(pattern: "/^[a-zA-Z][a-zA-Z0-9\.\-_]*$/")],
            verbose_name: 'Name',
            help_text: 'The unique name for this file.',
        );
        $this->type = new StringField(
            default: null,
            choices: ['luascript', 'writetodisk'],
            allow_null: true,
            verbose_name: 'Type',
            help_text: 'The type of file. Use `null` to assume an Errorfile.',
        );
        $this->content = new Base64Field(required: true, verbose_name: 'Content', help_text: 'The content of this file.');

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
