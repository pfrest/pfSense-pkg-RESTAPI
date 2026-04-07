<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DNSResolverApplyDispatcher;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that interacts with access list entries for the DNS Resolver (Unbound) service.
 */
class DNSResolverAccessList extends Model {
    public StringField $name;
    public StringField $action;
    public StringField $description;
    public NestedModelField $networks;

    /**
     * @inheritDoc
     */
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'unbound/acls';
        $this->subsystem = 'unbound';
        $this->many = true;

        # Set model Fields
        $this->name = new StringField(
            required: true,
            allow_empty: true,
            internal_name: 'aclname',
            verbose_name: 'Name',
            help_text: 'The name of this access list.',
        );
        $this->action = new StringField(
            required: true,
            choices: ['allow', 'deny', 'refuse', 'allow snoop', 'deny nonlocal', 'refuse nonlocal'],
            internal_name: 'aclaction',
            verbose_name: 'Action',
            help_text: 'The action to take when an access list match is found.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this access list.',
        );
        $this->networks = new NestedModelField(
            model_class: 'DNSResolverAccessListNetwork',
            required: true,
            verbose_name: 'Networks',
            help_text: 'The DNS Resolver access list network entries to include in this access list.',
        );

        # Construct the parent model object
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies pending DNS Resolver changes.
     */
    public function apply() {
        (new DNSResolverApplyDispatcher(async: $this->async))->spawn_process();
    }
}
