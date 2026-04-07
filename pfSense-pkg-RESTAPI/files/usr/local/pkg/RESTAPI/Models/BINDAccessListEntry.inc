<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\SubnetValidator;

/**
 * Defines a model that interacts with BIND access list entries.
 */
class BINDAccessListEntry extends Model {
    public StringField $value;
    public StringField $description;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'BINDAccessList';
        $this->config_path = 'row';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->always_apply = true;
        $this->many = true;
        $this->many_minimum = 1;
        $this->verbose_name = 'BIND Access List Entry';

        # Set model fields
        $this->value = new StringField(
            required: true,
            validators: [new SubnetValidator()],
            verbose_name: 'Value',
            help_text: 'The network CIDR to allow.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description of the access list entry.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the BIND access list entries
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
