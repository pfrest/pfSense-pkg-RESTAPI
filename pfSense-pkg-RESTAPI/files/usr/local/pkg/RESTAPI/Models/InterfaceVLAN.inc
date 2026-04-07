<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model for interacting with VLAN interfaces.
 */
class InterfaceVLAN extends Model {
    public StringField $if;
    public IntegerField $tag;
    public StringField $vlanif;
    public IntegerField $pcp;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set the configuration path for these objects and allow many to be stored.
        $this->config_path = 'vlans/vlan';
        $this->many = true;
        $this->always_apply = true;
        $this->unique_together_fields = ['if', 'tag'];

        # Model Fields
        $this->if = new StringField(
            required: true,
            editable: false,
            maximum_length: 32,
            verbose_name: 'Interface Name',
            help_text: 'The real parent interface this VLAN will be applied to.',
        );
        $this->tag = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 4094,
            verbose_name: 'Tag',
            help_text: 'The VLAN ID tag to use. This must be unique from all other VLANs on the parent interface.',
        );
        $this->vlanif = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            read_only: true,
            verbose_name: 'VLAN Interface',
            help_text: 'Displays the full interface VLAN. This value is automatically populated and cannot be set.',
        );
        $this->pcp = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 0,
            maximum: 7,
            verbose_name: 'PCP',
            help_text: 'The 802.1p VLAN priority code point (PCP) to assign to this VLAN.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description to help document the purpose of this VLAN.',
        );

        # Needs to be constructed after Fields are defined
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation to the `if` field.
     * @param string $if The parent interface value to validate.
     * @returns string The validated $if to set.
     * @throws ValidationError When the `if` value is invalid.
     */
    public function validate_if(string $if): string {
        # Throw a validation error if the requested parent interface does not exist
        if (!does_interface_exist($if)) {
            throw new ValidationError(
                message: "Parent interface `$if` does not exist.",
                response_id: 'VLAN_INTERFACE_PARENT_NOT_FOUND',
            );
        }

        return $if;
    }

    /**
     * Adds custom validation to the `tag` field.
     * @param int|null $tag The VLAN tag to validate.
     * @returns int|null The validated $name to set.
     * @throws ValidationError When the `tag` value is invalid.
     */
    public function validate_tag(int|null $tag): int {
        $if = $this->if->value;

        # Throw a validation error if this VLAN is already present as a QinQ
        if ($this->is_qinq(if: $if, tag: $tag)) {
            throw new ValidationError(
                message: "VLAN interface `$if.$tag` is already in use by a Q-in-Q.",
                response_id: 'VLAN_INTERFACE_IN_USE_BY_QINQ',
            );
        }

        return $tag;
    }

    /**
     * Automatically populates the `vlanif` field value using the `if` and `tag` values.
     * @param string $vlanif The incoming `vlanif` value. This value is immediately overwritten.
     * @returns string The full `vlanif` value.
     */
    public function validate_vlanif(string $vlanif) {
        return $this->if->value . '.' . $this->tag->value;
    }

    /**
     * Checks if a given interface VLAN is already present as a QinQ object
     * @param string $if The parent interface to check
     * @param int|null $tag The VLAN tag to check
     * @return bool `true` if this VLAN interface is already present as a QinQ, `false` if it is not.
     */
    public function is_qinq(string $if, int|null $tag): bool {
        # TODO: Change this to use a ModelSet query whenever a QinQ model is added
        foreach ($this->get_config('qinqs/qinqentry', []) as $qinq) {
            if ($qinq['if'] == $if and $qinq['tag'] == $tag) {
                return true;
            }
        }

        return false;
    }

    /**
     * Set the apply method to configure VLAN interfaces after successful creation.
     */
    public function apply_create() {
        interface_vlan_configure($this->to_internal());
    }

    /**
     * Set the apply method to configure VLAN interfaces after successful updates.
     */
    public function apply_update() {
        pfSense_interface_destroy($this->initial_object->if->value . '.' . $this->initial_object->tag->value);
        interface_vlan_configure($this->to_internal());
    }

    /**
     * Set the apply method to remove VLAN interfaces after successful deletion.
     */
    public function apply_delete() {
        pfSense_interface_destroy($this->initial_object->if->value . '.' . $this->initial_object->tag->value);
    }
}
