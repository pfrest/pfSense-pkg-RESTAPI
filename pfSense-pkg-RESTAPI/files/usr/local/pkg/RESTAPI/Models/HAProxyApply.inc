<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\HAProxyApplyDispatcher;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model for applying the HAProxy configuration.
 */
class HAProxyApply extends Model {
    public BooleanField $applied;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_applied_status';
        $this->always_apply = true;
        $this->many = false;
        $this->packages = ['pfSense-pkg-haproxy'];
        $this->package_includes = ['haproxy/haproxy.inc', 'haproxy/haproxy_utils.inc'];
        $this->verbose_name = 'HAProxy Apply';

        # Set model fields
        $this->applied = new BooleanField(
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            help_text: 'Indicates whether all HAProxy configuration changes have been applied.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal values for this Model's field objects.
     * @return array An array containing the internal values for this Model's fields.
     */
    protected function get_applied_status(): array {
        return ['applied' => !is_file('/var/run/haproxy.conf.dirty')];
    }

    /**
     * Overrides the default _create method to do nothing. We only want to apply.
     */
    public function _create() {}

    /**
     * Applies changes to the HAProxy configuration.
     */
    public function apply(): void {
        (new HAProxyApplyDispatcher(async: $this->async))->spawn_process();
    }
}
