<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;

/**
 * Defines a Model that interacts with HAProxy frontend error files.
 */
class HAProxyFrontendErrorFile extends Model {
    public IntegerField $errorcode;
    public ForeignModelField $errorfile;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'HAProxyFrontend';
        $this->config_path = 'a_errorfiles/item';
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->many = true;
        $this->verbose_name = 'HAProxy Frontend Error File';

        # Set model fields
        $this->errorcode = new IntegerField(
            required: true,
            unique: true,
            minimum: 100,
            maximum: 511,
            verbose_name: 'Error Code',
            help_text: 'The HTTP status code that will trigger this error file to be used.',
        );
        $this->errorfile = new ForeignModelField(
            model_name: 'HAProxyFile',
            model_field: 'name',
            required: true,
            verbose_name: 'Error File',
            help_text: 'The HAProxy error file object that should be used for the assigned HTTP status code.',
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
