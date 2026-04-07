<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\FilterNameValidator;

/**
 * Defines a Model for interacting with Interface Groups.
 */
class InterfaceGroup extends Model {
    public StringField $ifname;
    public InterfaceField $members;
    public StringField $descr;

    /**
     * @inheritDoc
     */
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'ifgroups/ifgroupentry';
        $this->many = true;
        $this->always_apply = true;

        # Set model Fields
        $this->ifname = new StringField(
            required: true,
            unique: true,
            editable: false,
            maximum_length: 15,
            validators: [new FilterNameValidator()],
            verbose_name: 'Interface Name',
            help_text: 'The name of this interface group.',
        );
        $this->members = new InterfaceField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: ' ',
            verbose_name: 'Members',
            help_text: 'The member interfaces to assign to this interface group.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The description for this interface group.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies the initial creation of an InterfaceGroup.
     */
    public function apply_create() {
        interface_group_setup($this->to_internal());
    }

    /**
     * Applies updates to an existing InterfaceGroup.
     */
    public function apply_update() {
        # Loop through each initial member interface and remove it from the group
        foreach ($this->initial_object->members->get_related_models()->model_objects as $member) {
            new Command("/sbin/ifconfig {$member->if->value} -group " . $this->ifname->value);
        }

        # Setup the group with the new member interfaces
        interface_group_setup($this->to_internal());
    }

    /**
     * Applies the deletion of an interface group.
     */
    public function apply_delete() {
        # Loop through each member interface and remove it from the group
        foreach ($this->members->get_related_models()->model_objects as $member) {
            new Command("/sbin/ifconfig {$member->if->value} -group " . $this->ifname->value);
        }
    }

    /**
     * Gracefully brings down the previous (existing) set InterfaceGroup objects and applies the new set.
     * @param ModelSet $initial_objects A ModelSet containing each existing object for this Model as they were when
     * the `replace_all()` method was initially called. This is useful when a function or service needs to be restarted
     * for each object before replacing the set.
     * @param ModelSet $new_objects A ModelSet containing each of the new objects for this Model. Note: at this
     * point, each Model object within this ModelSet has already been written to config. This method is responsible
     * for ensuring it is applied to any backend services.
     */
    function apply_replace_all(ModelSet $initial_objects, ModelSet $new_objects) {
        # Loop through each of the InterfaceGroup objects being replaced and remove the group from its members
        foreach ($initial_objects->model_objects as $old_if_group) {
            # Loop through each initial member interface and remove it from the group
            foreach ($old_if_group->members->get_related_models()->model_objects as $member) {
                new Command("/sbin/ifconfig {$member->if->value} -group " . $old_if_group->ifname->value);
            }
        }

        # Loop through each new InterfaceGroup object and ensure it is applied
        foreach ($new_objects->model_objects as $new_object) {
            interface_group_setup($new_object->to_internal());
        }
    }
}
