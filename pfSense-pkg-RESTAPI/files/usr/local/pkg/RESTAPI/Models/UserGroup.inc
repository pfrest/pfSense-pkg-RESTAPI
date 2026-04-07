<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ForbiddenError;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\UnprocessableContentError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for interacting with the user groups on this system.
 */
class UserGroup extends Model {
    public StringField $name;
    public IntegerField $gid;
    public StringField $scope;
    public StringField $description;
    public ForeignModelField $member;
    public StringField $priv;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'system/group';
        $this->many = true;
        $this->always_apply = true;
        $this->sort_order = SORT_ASC;
        $this->sort_by = ['name'];
        $this->protected_model_query = ['scope' => 'system'];

        # Set model Fields
        $this->name = new StringField(
            required: true,
            unique: true,
            maximum_length: 64,
            validators: [new RegexValidator(pattern: '/[^a-zA-Z0-9\.\-_]/', invert: true)],
            verbose_name: 'Name',
            help_text: 'The name for this user group.',
        );
        $this->gid = new IntegerField(
            default: (int) $this->get_config('system/nextgid'),
            editable: false,
            read_only: true,
            verbose_name: 'GID',
            help_text: 'The GID of this user group. This value is automatically assigned and cannot be changed.',
        );
        $this->scope = new StringField(
            default: 'local',
            choices: ['local', 'remote', 'system'],
            verbose_name: 'Scope',
            help_text: 'The scope of this user group. Use `local` for user groups that only apply to this system. use ' .
                '`remote` for groups that also apply to remote authentication servers. Please note the `system` scope ' .
                'is reserved for built-in, system-defined user groups and cannot be assigned manually.',
        );
        $this->member = new ForeignModelField(
            model_name: 'User',
            model_field: 'name',
            model_field_internal: 'uid',
            default: [],
            allow_empty: true,
            many: true,
            many_maximum: 2048,
            delimiter: null,
            verbose_name: 'Member',
            help_text: 'The local user names to assign to this user group.',
        );
        $this->priv = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: null,
            verbose_name: 'Privileges',
            help_text: 'The privileges to assign to this user group.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The description to assign to this user group.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds additional validation to the `name` field.
     * @param string $name The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When `scope` is `local` and the value contains more than 16 characters.
     */
    public function validate_name(string $name): string {
        # Require the `name` to be 16 characters or less if `scope` is set to `local`
        if ($this->scope->value === 'local' and strlen($name) > 16) {
            throw new ValidationError(
                message: 'Field `name` must be 16 characters or less if `scope` is set to `local`.',
                response_id: 'USER_GROUP_NAME_EXCEEDS_MAXIMUM_LENGTH_FOR_LOCAL_SCOPE',
            );
        }

        return $name;
    }

    /**
     * Adds additional validation to the `scope` field.
     * @param string $scope The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When `scope` is set to `system` and this is a newly created object or the existing
     * `scope` is not already `system`.
     */
    public function validate_scope(string $scope): string {
        $existing_scope = $this->initial_object->scope->value ?? null;

        if ($scope === 'system' and $scope !== $existing_scope) {
            throw new UnprocessableContentError(
                message: 'Field `scope` cannot be set to `system` as it is reserved for system-defined user groups only.',
                response_id: 'USER_GROUP_SCOPE_CANNOT_BE_SET_TO_SYSTEM',
            );
        }

        return $scope;
    }

    /**
     * Adds additional validation to the `priv` field.
     * @param string $priv The incoming value to be validated
     * @return string The validated value to be assigned.
     * @throws ValidationError When an unknown privilege name is requested.
     */
    public function validate_priv(string $priv): string {
        global $priv_list;

        # Ensure requested privileges are valid privilege names
        if (!array_key_exists($priv, $priv_list)) {
            throw new NotFoundError(
                message: "Field `priv` contains unknown privilege `$priv`",
                response_id: 'USER_GROUP_PRIV_NOT_FOUND',
            );
        }

        return $priv;
    }

    /**
     * Extends the _create() method so it bumps the next GID before creating the object
     */
    public function _create() {
        # Bump the nextuid in config to one greater than this UID
        $this->set_config('system/nextgid', $this->gid->value + 1);
        parent::_create();
    }

    /**
     * Extends the _delete() method to prevent the deletion of `system` scoped user groups
     * @throws ForbiddenError When the object being deleted has its `scope` set to `system`.
     */
    public function _delete() {
        # Do not allow the deletion of `system` scope UserGroups
        if ($this->scope->value === 'system') {
            throw new ForbiddenError(
                message: 'User groups with `scope` set to `system` cannot be deleted.',
                response_id: 'USER_GROUP_CANNOT_DELETE_SYSTEM_SCOPED_GROUPS',
            );
        }

        parent::_delete();
    }

    /**
     * Applies changes to this UserGroup on the backend.
     */
    public function apply() {
        # Configure this group on the underlying OS
        local_group_set($this->to_internal());

        # Reconfigure assigned user members to assign any new privileges
        foreach ($this->member->get_related_models()->model_objects as $member_user) {
            local_user_set($member_user->to_internal());
        }
    }

    /**
     * Applies the deletion of this UserGroup on the backend.
     */
    public function apply_delete() {
        local_group_del($this->to_internal());
    }

    /**
     * Applies the replacement of all existing UserGroups with a newly specified set of UserGroups.
     */
    public function apply_replace_all(ModelSet $initial_objects, ModelSet $new_objects) {
        # Remove any existing UserGroups from the backend OS
        foreach ($initial_objects->model_objects as $initial_object) {
            local_group_del($initial_object->to_internal());
        }

        # Add the new UserGroups on the backend and reconfigure assigned to users to assign new privileges.
        foreach ($new_objects->model_objects as $new_object) {
            # Configure this group on the underlying OS
            local_group_set($new_object->to_internal());

            # Reconfigure assigned user members to assign any new privileges
            foreach ($new_object->member->get_related_models()->model_objects as $member_user) {
                local_user_set($member_user->to_internal());
            }
        }
    }
}
