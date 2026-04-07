<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RoutingApplyDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model for interacting with a Gateway Group priority item.
 */
class RoutingGatewayGroupPriority extends Model {
    public ForeignModelField $gateway;
    public IntegerField $tier;
    public ForeignModelField $virtual_ip;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->parent_model_class = 'RoutingGatewayGroup';
        $this->config_path = 'item';
        $this->subsystem = 'staticroutes';
        $this->update_strategy = 'replace';
        $this->many = true;
        $this->many_minimum = 1;

        # Set model fields
        $this->gateway = new ForeignModelField(
            model_name: 'RoutingGateway',
            model_field: 'name',
            required: true,
            unique: true,
            verbose_name: 'Gateway',
            help_text: 'The name of the gateway to prioritize in this gateway group.',
        );
        $this->tier = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 5,
            verbose_name: 'Tier',
            help_text: 'The priority of this gateway in the group. Lower numbered tiers are higher priority.',
        );
        $this->virtual_ip = new ForeignModelField(
            model_name: 'VirtualIP',
            model_field: 'uniqid',
            allowed_keywords: ['address'],
            default: 'address',
            verbose_name: 'Virtual IP',
            help_text: "The virtual IP to use for this gateway group. Use `address` to use the interface's current IP.",
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Provide extra validation for the entire Model.
     */
    public function validate_extra(): void {
        # Check the IP protocol of the gateway being added vs the IP protocol of the parent gateway group
        $gw_ipprotocol = $this->gateway->get_related_model()->ipprotocol->value;
        $parent_ipprotocol = $this->parent_model->ipprotocol->value;

        # If we know the IP protocol of the parent gateway group, ensure the gateway being added matches it
        if ($parent_ipprotocol !== 'unknown' and $parent_ipprotocol !== $gw_ipprotocol) {
            throw new ValidationError(
                message: "Field `gateway` must be a gateway of the same IP protocol as all other gateways in the ' .
                    'parent gateway group. Expected $parent_ipprotocol, got $gw_ipprotocol.",
                response_id: 'ROUTING_GATEWAY_GROUP_PRIORITY_IPPROTOCOL_MISMATCH',
            );
        }
    }

    /**
     * Apply changes to cron.
     */
    public function apply(): void {
        (new RoutingApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Converts the object to its internal form.
     * @note This method has overridden the default Model method because these internal objects aren't actual objects,
     * they are a pipe delimited string which is inconsistent with other Models.
     * @return string The pipe delimited string internal representation of the object.
     */
    public function to_internal(): string {
        # Ensure the _vip prefix is added to the virtual IP if not 'address'
        if ($this->virtual_ip->value !== 'address') {
            $this->virtual_ip->value = "_vip{$this->virtual_ip->value}";
        }

        return "{$this->gateway->value}|{$this->tier->value}|{$this->virtual_ip->value}";
    }

    /**
     * Converts the object from its internal form.
     * @note This method has overridden the default Model method because these internal objects aren't actual objects,
     * they are a pipe delimited string which is inconsistent with other Models.
     * @param mixed $internal_object The internal object to convert from. This should be a pipe delimited string where
     * the first field is the gateway name, the second field is the tier, and the third field is the virtual IP value.
     */
    public function from_internal_object(mixed $internal_object): void {
        # Ensure the internal object is a pipe delimited string
        if (!is_string($internal_object) or count(explode('|', $internal_object)) !== 3) {
            throw new ServerError(
                message: "{$this->get_class_shortname()} object with ID $this->id has an invalid internal object.",
                response_id: 'ROUTING_GATEWAY_GROUP_PRIORITY_INTERNAL_OBJECT_INVALID',
            );
        }

        # Otherwise, set the fields based on the expanded internal object (pipe delimited string)
        [$gateway, $tier, $virtual_ip] = explode('|', $internal_object);
        $this->gateway->value = $gateway;
        $this->tier->value = (int) $tier;
        $this->virtual_ip->value = str_replace('_vip', '', $virtual_ip);
    }
}
