<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RoutingApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\SubnetValidator;

/**
 * Defines a Model that interacts with static routes on this system.
 */
class StaticRoute extends Model {
    public StringField $network;
    public ForeignModelField $gateway;
    public StringField $descr;
    public BooleanField $disabled;

    /**
     * Constructs the StaticRoute object.
     */
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->config_path = 'staticroutes/route';
        $this->many = true;
        $this->subsystem = 'staticroutes';

        # Define model Fields
        $this->network = new StringField(
            required: true,
            validators: [new SubnetValidator(allow_ipv4: true, allow_ipv6: true, allow_alias: true)],
            verbose_name: 'Network',
            help_text: 'Sets the destination network for this static route in CIDR notation.',
        );
        $this->gateway = new ForeignModelField(
            model_name: ['RoutingGateway', 'RoutingGatewayGroup'],
            model_field: 'name',
            required: true,
            verbose_name: 'Gateway',
            help_text: 'Sets which gateway this route applies to.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'Sets a description for administrative reference.',
        );
        $this->disabled = new BooleanField(default: false, verbose_name: 'Disabled', help_text: 'Disable this static route.');

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation for the `network` field.
     * @param string $network The incoming `network` value to be validated.
     * @return string The validated `network` value to set.
     */
    public function validate_network(string $network): string {
        # Skip additional validation if the `network` is an alias
        if ($this->network->has_label('is_alias')) {
            return $network;
        }

        # Extract the address and subnet from the CIDR
        $addr = explode('/', $network)[0];
        $subnet = explode('/', $network)[1];

        # Find the network address for the subnet
        if ($this->network->has_label('is_subnetv4')) {
            $addr = gen_subnet($addr, $subnet);
        } elseif ($this->network->has_label('is_subnetv6')) {
            $addr = gen_subnetv6($addr, $subnet);
        }

        # Format the network address and subnet into a CIDR
        $network = "$addr/$subnet";

        # Ensure this network is not used by an existing NetworkInterface
        foreach (NetworkInterface::read_all()->model_objects as $interface) {
            # Check if this is an IPv4 address and this interface has a static IPv4 address
            if ($this->network->has_label('is_subnetv4') and $interface->typev4->value === 'static') {
                # Ensure this route doesn't conflict with this interface's subnet
                if ($addr === gen_subnet($interface->ipaddr->value, $interface->subnet->value)) {
                    throw new ValidationError(
                        message: "Destination network `$network` conflicts with the IP address of interface with " .
                            "ID `$interface->id`",
                        response_id: 'STATIC_ROUTE_CONFLICTS_WITH_IPV4_INTERFACE',
                    );
                }
            }
            # Check if this is an IPv4 address and this interface has a static IPv6 address
            if ($this->network->has_label('is_subnetv6') and $interface->typev6->value === 'staticv6') {
                # Ensure this route doesn't conflict with this interface's subnet
                if ($addr === gen_subnetv6($interface->ipaddrv6->value, $interface->subnetv6->value)) {
                    throw new ValidationError(
                        message: "Destination network `$network` conflicts with the IP address of interface with " .
                            "ID `$interface->id`",
                        response_id: 'STATIC_ROUTE_CONFLICTS_WITH_IPV6_INTERFACE',
                    );
                }
            }
        }

        return $network;
    }

    /**
     * Adds custom validation for the `gateway` field.
     * @param string $gateway The incoming `gateway` value to be validated.
     * @return string The validated `gateway` value to set.
     */
    public function validate_gateway(string $gateway): string {
        # Skip extra validation if the `network` is an alias
        if ($this->network->has_label('is_alias')) {
            return $gateway;
        }

        # Fetch the gateway object of the value assigned
        $gateway_obj = $this->gateway->get_related_model();

        # Ensure the upstream gateways IP version matches the `network` assigned
        if ($gateway_obj->ipprotocol->value === 'inet' and !$this->network->has_label('is_subnetv4')) {
            throw new ValidationError(
                message: 'Cannot assign non-IPv4 `network` to IPv4 `gateway`.',
                response_id: 'STATIC_ROUTE_NON_IPV4_NETWORK_WITH_IPV4_GATEWAY',
            );
        }

        # Ensure the upstream gateways IP version matches the `network` assigned
        if ($gateway_obj->ipprotocol->value === 'inet6' and !$this->network->has_label('is_subnetv6')) {
            throw new ValidationError(
                message: 'Cannot assign non-IPv6 `network` to IPv6 `gateway`.',
                response_id: 'STATIC_ROUTE_NON_IPV6_NETWORK_WITH_IPV6_GATEWAY',
            );
        }

        return $gateway;
    }

    /**
     * Adds custom validation for the `disabled` field.
     * @param bool $disabled The incoming `disabled` value to be validated.
     * @return bool The validated `disabled` value to set.
     */
    public function validate_disabled(bool $disabled): bool {
        # Fetch the gateway object of the gateway assigned
        $gateway_obj = RoutingGateway::query(['name' => $this->gateway->value])->first();

        # Force route to be disabled if the parent gateway is disabled
        if ($gateway_obj->disabled->value and !$disabled) {
            throw new ValidationError(
                message: ' Static route must also be `disabled` when the parent gateway is `disabled`.',
                response_id: 'STATIC_ROUTE_GATEWAY_DISABLED',
            );
        }

        return $disabled;
    }

    /**
     * Applies pending static route changes.
     */
    public function apply() {
        (new RoutingApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Ensure routing changes are serialized and store at /tmp/.system_routes.apply before applying.
     */
    public function pre_apply_update() {
        $this->serialize_route();
    }

    /**
     * Ensures the static route deletion is performed before removing it from config
     */
    public function _delete() {
        delete_static_route($this->id, true);
        parent::_delete();
    }

    /**
     * Expands `network` values that are aliases in an array of subnets to apply static routes for.
     */
    public function get_networks() {
        # Keep track of networks that can be applied to a static route
        $networks = [];

        # Check if the current network value is an alias
        if ($this->network->has_label('is_alias')) {
            # Query for the alias assigned and loop through it's addresses
            $alias = FirewallAlias::query(['name' => $this->network->value])->first();
            foreach ($alias->address->value as $alias_network) {
                # Set CIDR if this is an IPv4 address
                if (is_ipaddrv4($alias_network)) {
                    $alias_network .= '/32';
                }
                # Set CIDR if this is an IPv6 address
                if (is_ipaddrv6($alias_network)) {
                    $alias_network .= '/128';
                }
                # Don't include this network if it is not a subnet
                if (!is_subnet($alias_network)) {
                    continue;
                }
                # Add this network to the return array
                $networks[] = $alias_network;
            }
        }
        # Otherwise, we already know it is a subnet, simply add it to the networks list
        else {
            $networks[] = $this->network->value;
        }

        return $networks;
    }

    /**
     * Serializes pending routing changes and stores them at /tmp/.system_routes.apply to apply later.
     */
    private function serialize_route(): void {
        global $g;
        $old_network = [];

        # Include pending route changes that haven't been applied yet, or simply initialize our pending change var
        if (file_exists("{$g['tmp_path']}/.system_routes.apply")) {
            $to_apply_list = unserialize(file_get_contents("{$g['tmp_path']}/.system_routes.apply"));
        } else {
            $to_apply_list = [];
        }

        # Check for the static route that is currently operating
        if (file_exists("{$g['tmp_path']}/staticroute_{$this->id}")) {
            $old_network = unserialize(file_get_contents("{$g['tmp_path']}/staticroute_{$this->id}"));
        }
        # Check for the static route gateway that is currently operating
        if (file_exists("{$g['tmp_path']}/staticroute_{$this->id}_gw")) {
            $old_gateway = unserialize(file_get_contents("{$g['tmp_path']}/staticroute_{$this->id}_gw"));
        }

        # If was set previously, and has changed, ensure we delete the old route and recreate it.
        if (!empty($old_gateway) && $this->gateway->value != $old_gateway) {
            $delete_networks = $old_network;
            $delete_gateway = lookup_gateway_ip_by_name($old_gateway);
        }
        # Otherwise, check for routes that are no longer relevant to this entry
        else {
            $delete_networks = array_diff($old_network, $this->get_networks());
            $delete_gateway = lookup_gateway_ip_by_name($this->gateway->value);
        }

        # Assign routes to be deleted when changes are applied
        if (count($delete_networks)) {
            foreach ($delete_networks as $del_net) {
                # Set the IP address family correctly when this is an IPv6 route
                if (is_ipaddrv6($del_net)) {
                    $ip_family = '-inet6';
                }
                # Fetch the route to be deleted
                $route = route_get($del_net, '', true);

                # Skip this route if we failed to fetch the route in the previous call
                if (!count($route)) {
                    continue;
                }

                # Mark this route for deletion during the next change
                $to_apply_list[] = "/sbin/route delete {$ip_family} {$del_net} {$delete_gateway}";
            }
        }

        file_put_contents("{$g['tmp_path']}/.system_routes.apply", serialize($to_apply_list));
    }
}
