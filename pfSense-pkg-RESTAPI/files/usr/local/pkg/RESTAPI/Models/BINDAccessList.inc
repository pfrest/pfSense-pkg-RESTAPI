<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a model that interacts with BIND access lists.
 */
class BINDAccessList extends Model {
    public StringField $name;
    public StringField $description;
    public NestedModelField $entries;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/bindacls/config';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'BIND Access List';

        # Set model fields
        $this->name = new StringField(required: true, unique: true, help_text: 'The name of the access list.');
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'A description for the access list.',
        );
        $this->entries = new NestedModelField(
            model_class: 'BINDAccessListEntry',
            required: true,
            help_text: 'The network entries for this access list.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the BIND access lists
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
