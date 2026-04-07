<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RoutingApplyDispatcher;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\FilterNameValidator;

/**
 * Defines a Model for interacting with a Gateway Group priority item.
 */
class RoutingGatewayGroup extends Model {
    public StringField $name;
    public StringField $trigger;
    public StringField $descr;
    public StringField $ipprotocol;
    public NestedModelField $priorities;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'gateways/gateway_group';
        $this->subsystem = 'staticroutes';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            editable: false,
            maximum_length: 31,
            referenced_by: ['StaticRoute' => 'gateway', 'FirewallRule' => 'gateway'],
            validators: [new FilterNameValidator()],
            verbose_name: 'Name',
            help_text: 'The name of the gateway group.',
        );
        $this->trigger = new StringField(
            default: 'down',
            choices: ['down', 'downloss', 'downlatency', 'downlosslatency'],
            verbose_name: 'Trigger',
            help_text: 'The trigger that will cause a gateway to be excluded from the group.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 255,
            verbose_name: 'Description',
            help_text: 'A description of the gateway group.',
        );
        $this->ipprotocol = new StringField(
            default: 'unknown',
            read_only: true,
            representation_only: true,
            verbose_name: 'IP Protocol',
            help_text: 'The assumed IP protocol of the gateways in this group.',
        );
        $this->priorities = new NestedModelField(
            model_class: 'RoutingGatewayGroupPriority',
            required: true,
            verbose_name: 'Priorities',
            help_text: 'The priorities of the gateways in this group.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the IP protocol of the gateways in this group. This method is automatically called to populate the
     * `ipprotocol` field's value.
     * @return string The IP protocol of the gateways in this group or 'unknown' if the IP protocol cannot be determined.
     */
    protected function from_internal_ipprotocol(): string {
        # Check the IP protocol of the gateways in this group
        foreach ($this->priorities->value as $priority) {
            # Query for the gateway related to this priority
            $gw_q = RoutingGateway::query(name: $priority['gateway']);

            # Skip this priority if the gateway does not exist. This can happen if the gateway is not in config.
            if (!$gw_q->exists()) {
                continue;
            }

            # Check the IP protocol of the gateway
            $gw_obj = $gw_q->first();
            if ($gw_obj->ipprotocol->value === 'inet') {
                return 'inet';
            } elseif ($gw_obj->ipprotocol->value === 'inet6') {
                return 'inet6';
            }
        }
        return 'unknown';
    }

    /**
     * Defines the pre-apply method to ensure the 'ipprotocol' field is set after creation.
     */
    public function pre_apply(): void {
        # Ensure the IP protocol is always up to date after changes are made
        $this->ipprotocol->value = $this->from_internal_ipprotocol();
    }

    /**
     * Apply changes to gateway groups.
     */
    public function apply(): void {
        (new RoutingApplyDispatcher(async: $this->async))->spawn_process();
    }
}
