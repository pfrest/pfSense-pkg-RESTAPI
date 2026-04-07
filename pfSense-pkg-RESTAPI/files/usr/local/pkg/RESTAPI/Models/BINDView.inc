<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a model that interacts with BIND views.
 */
class BINDView extends Model {
    public StringField $name;
    public StringField $descr;
    public BooleanField $recursion;
    public ForeignModelField $match_clients;
    public ForeignModelField $allow_recursion;
    public Base64Field $bind_custom_options;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/bindviews/config';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'BIND View';

        # Set model fields
        $this->name = new StringField(required: true, unique: true, verbose_name: 'Name', help_text: 'The name of the view.');
        $this->descr = new StringField(default: '', allow_empty: true, verbose_name: 'Description', help_text: 'A description for the view.');
        $this->recursion = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Recursion',
            help_text: 'Enables or disables recursion for the view.',
        );
        $this->match_clients = new ForeignModelField(
            model_name: 'BINDAccessList',
            model_field: 'name',
            default: [],
            allow_empty: true,
            many: true,
            internal_name: 'match-clients',
            verbose_name: 'Match Clients',
            help_text: 'The access lists to match clients against.',
        );
        $this->allow_recursion = new ForeignModelField(
            model_name: 'BINDAccessList',
            model_field: 'name',
            default: [],
            allow_empty: true,
            many: true,
            internal_name: 'allow-recursion',
            verbose_name: 'Allow Recursion',
            help_text: 'The access lists to allow recursion for.',
        );
        $this->bind_custom_options = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'BIND Custom Options',
            help_text: 'Custom BIND options for the view.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the BIND views.
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
