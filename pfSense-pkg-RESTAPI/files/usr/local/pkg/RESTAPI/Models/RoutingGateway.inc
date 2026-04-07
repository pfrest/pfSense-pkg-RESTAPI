<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RoutingApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\FilterNameValidator;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with routing gateways in pfSense.
 */
class RoutingGateway extends Model {
    public StringField $name;
    public StringField $descr;
    public BooleanField $disabled;
    public StringField $ipprotocol;
    public InterfaceField $interface;
    public StringField $gateway;
    public BooleanField $monitor_disable;
    public StringField $monitor;
    public BooleanField $action_disable;
    public BooleanField $force_down;
    public BooleanField $dpinger_dont_add_static_route;
    public StringField $gw_down_kill_states;
    public BooleanField $nonlocalgateway;
    public IntegerField $weight;
    public IntegerField $data_payload;
    public IntegerField $latencylow;
    public IntegerField $latencyhigh;
    public IntegerField $losslow;
    public IntegerField $losshigh;
    public IntegerField $interval;
    public IntegerField $loss_interval;
    public IntegerField $time_period;
    public IntegerField $alert_interval;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define Model attributes
        $this->config_path = 'gateways/gateway_item';
        $this->subsystem = 'staticroutes';
        $this->many = true;
        $this->unique_together_fields = ['interface', 'ipprotocol', 'gateway'];

        # Define Model Fields
        $this->name = new StringField(
            required: true,
            unique: true,
            editable: false,
            maximum_length: 31,
            referenced_by: ['StaticRoute' => 'gateway', 'FirewallRule' => 'gateway'],
            validators: [new FilterNameValidator()],
            verbose_name: 'Name',
            help_text: 'Sets a name for the gateway.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'Sets a descriptions for the gateway.',
        );
        $this->disabled = new BooleanField(default: false, verbose_name: 'Disabled', help_text: 'Disable this gateway.');
        $this->ipprotocol = new StringField(
            required: true,
            choices: ['inet', 'inet6'],
            verbose_name: 'IP Protocol',
            help_text: 'Sets the Internet Protocol version this gateway uses.',
        );
        $this->interface = new InterfaceField(
            required: true,
            verbose_name: 'Interface',
            help_text: 'Sets the interface this gateway will apply to.',
        );
        $this->gateway = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_keywords: ['dynamic'])],
            verbose_name: 'Gateway',
            help_text: 'Sets the IP address of the remote gateway.',
        );
        $this->monitor_disable = new BooleanField(
            default: false,
            verbose_name: 'Monitor Disable',
            help_text: 'Disable gateway monitoring for this gateway.',
        );
        $this->monitor = new StringField(
            required: false,
            allow_null: true,
            conditions: ['monitor_disable' => false],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'Monitor',
            help_text: "Sets a different IP address to use when monitoring this gateway. This is typically only
                necessary if the gateway IP does not accept ICMP probes.",
        );
        $this->action_disable = new BooleanField(
            default: false,
            verbose_name: 'Action Disable',
            help_text: "Disable actions from taking place when gateway events occur. The gateway will always be
                considered up.",
        );
        $this->force_down = new BooleanField(default: false, verbose_name: 'Force Down', help_text: 'Always consider this gateway to be up.');
        $this->dpinger_dont_add_static_route = new BooleanField(
            default: false,
            verbose_name: 'Dpinger No Static Route',
            help_text: "Prevents gateway monitoring from adding a static route for this gateway's monitor IP.",
        );
        $this->gw_down_kill_states = new StringField(
            default: '',
            choices: ['', 'none', 'down'],
            allow_empty: true,
            verbose_name: 'GW Down Kill States',
            help_text: 'Controls the state killing behavior when this specific gateway goes down. Killing states for ' .
                'specific down gateways only affects states created by policy routing rules and reply-to. Has no ' .
                'effect if gateway monitoring or its action are disabled or if the gateway is forced down. May not ' .
                'have any effect on dynamic gateways during a link loss event.',
        );
        $this->nonlocalgateway = new BooleanField(
            default: true,
            verbose_name: 'Non-Local Gateway',
            help_text: "Allows or disallows gateway IPs that are not a part of the parent interface's subnet(s).",
        );
        $this->weight = new IntegerField(
            default: 1,
            minimum: 1,
            maximum: 30,
            verbose_name: 'Weight',
            help_text: 'Sets the weight for this gateway when used in a Gateway Group.',
        );
        $this->data_payload = new IntegerField(
            default: 1,
            minimum: 1,
            verbose_name: 'Data Payload',
            help_text: 'Sets the data payload to send in ICMP packets to gateway monitor IP.',
        );
        $this->latencylow = new IntegerField(
            default: 200,
            minimum: 1,
            verbose_name: 'Low Latency Threshold',
            help_text: 'Sets the threshold to consider latency as low.',
        );
        $this->latencyhigh = new IntegerField(
            default: 500,
            minimum: 2,
            verbose_name: 'High Latency Threshold',
            help_text: 'Sets the threshold to consider latency as high. This value must be greater than `latencylow`.',
        );
        $this->losslow = new IntegerField(
            default: 10,
            minimum: 1,
            maximum: 99,
            verbose_name: 'Low Loss Threshold',
            help_text: 'Sets the threshold to consider packet loss as low.',
        );
        $this->losshigh = new IntegerField(
            default: 20,
            minimum: 2,
            maximum: 100,
            verbose_name: 'High Loss Threshold',
            help_text: 'Sets the threshold to consider packet loss as high. This value must be greater than `losslow`.',
        );
        $this->interval = new IntegerField(
            default: 500,
            minimum: 1,
            maximum: 3600000,
            verbose_name: 'Interval',
            help_text: 'Sets how often ICMP probes will be sent in milliseconds.',
        );
        $this->loss_interval = new IntegerField(
            default: 2000,
            minimum: 1,
            verbose_name: 'Loss Interval',
            help_text: 'Sets the time interval in milliseconds before packets are treated as lost.',
        );
        $this->time_period = new IntegerField(
            default: 60000,
            minimum: 1,
            verbose_name: 'Time Period',
            help_text: 'Sets the time period in milliseconds over which results are averaged.',
        );
        $this->alert_interval = new IntegerField(
            default: 1000,
            minimum: 1,
            verbose_name: 'Alert Interval',
            help_text: 'Sets the time interval in milliseconds between checking for an alert conditions.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation to the `disabled` field.
     * @param bool $disabled The current $disabled value to validate.
     * @return bool The validated disabled value to set.
     */
    public function validate_disabled(bool $disabled): bool {
        # When disabling this gateway is requested, ensure it is not in use first
        if ($disabled) {
            # Ensure the gateway cannot be disabled while it is in use by a gateway group
            # TODO: replace this with a `referenced_by` Field value when the GatewayGroups Model is developed
            foreach ($this->get_config(path: 'gateways/gateway_group', default: []) as $gw_group_id => $gw_group) {
                foreach ($gw_group['item'] as $item) {
                    $items = explode('|', $item);
                    if ($items[0] == $this->name->value) {
                        throw new ValidationError(
                            message: 'Gateway cannot be disabled because it is in use by gateway ' .
                                "group with ID `$gw_group_id`",
                            response_id: 'ROUTING_GATEWAY_DISABLED_NOT_ALLOWED_WITH_EXISTING_GW_GROUP',
                        );
                    }
                }
            }

            # Ensure the gateway cannot be disabled while it is in use by a non-disabled static route
            $routes_in_use = StaticRoute::query(['gateway' => $this->name->value, 'disabled' => false]);
            if ($routes_in_use->exists()) {
                throw new ValidationError(
                    message: 'Gateway cannot be disabled because it is in use by gateway ' .
                        "route with ID `{$routes_in_use->first()->id}`",
                    response_id: 'ROUTING_GATEWAY_DISABLED_NOT_ALLOWED_WITH_EXISTING_ROUTE',
                );
            }

            # Ensure the gateway cannot be disabled while it is in use by a system DNS server
            # TODO: replace with a Model query when the SystemDNSServer Model is developed
            $dnsgw_counter = 1;
            while ($this->get_config("system/dns{$dnsgw_counter}gw")) {
                if ($this->name->value == $this->get_config("system/dns{$dnsgw_counter}gw")) {
                    throw new ValidationError(
                        message: 'Gateway cannot be disabled because it is in use by a system DNS ' .
                            "server with ID `$dnsgw_counter`",
                        response_id: 'ROUTING_GATEWAY_DISABLED_NOT_ALLOWED_WITH_EXISTING_ROUTE',
                    );
                }
                $dnsgw_counter++;
            }
        }

        return $disabled;
    }

    /**
     * Adds custom validation to the `interface` field.
     * @param string $interface The $interface value to validate.
     * @return string The validated interface value to set.
     */
    public function validate_interface(string $interface): string {
        # Obtain the NetworkInterface Model object associated with our current interface value
        $if_obj = $this->interface->get_related_model();

        # Ensure the configured `interface` has an IPv4 address if `protocol` is set to `inet`
        if (
            $this->ipprotocol->value === 'inet' and
            (!$if_obj->get_current_ipv4() or !$if_obj->get_current_subnetv4())
        ) {
            throw new ValidationError(
                message: 'Field `interface` must be an interface with an existing IPv4 address when `ipprotocol` ' .
                    'is set to `inet`',
                response_id: 'ROUTING_GATEWAY_INTERFACE_HAS_NO_IPV4',
            );
        }
        # Ensure the configured `interface` has an IPv6 address if `protocol` is set to `inet6`
        if (
            $this->ipprotocol->value === 'inet6' and
            (!$if_obj->get_current_ipv6() or !$if_obj->get_current_subnetv6())
        ) {
            throw new ValidationError(
                message: 'Field `interface` must be an interface with an existing IPv6 address when `ipprotocol` ' .
                    'is set to `inet6`',
                response_id: 'ROUTING_GATEWAY_INTERFACE_HAS_NO_IPV6',
            );
        }

        return $interface;
    }

    /**
     * Adds custom validation to the `gateway` field
     * @param string $gateway The gateway value to validate.
     * @return string The validated gateway value to set.
     */
    public function validate_gateway(string $gateway): string {
        # Only allow IPv4 addresses if `ipprotocol` is `inet`
        if (
            $this->ipprotocol->value === 'inet' and
            !$this->gateway->has_label('is_ipaddrv4') and
            $gateway !== 'dynamic'
        ) {
            throw new ValidationError(
                message: 'Field `gateway` must be an IPv4 address when `ipprotocol` is set to `inet`',
                response_id: 'ROUTING_GATEWAY_NO_IPV6_WITH_INET',
            );
        }

        # Only allow IPv4 addresses if `ipprotocol` is `inet6`
        if (
            $this->ipprotocol->value === 'inet6' and
            !$this->gateway->has_label('is_ipaddrv6') and
            $gateway !== 'dynamic'
        ) {
            throw new ValidationError(
                message: 'Field `gateway` must be an IPv6 address when `ipprotocol` is set to `inet6`',
                response_id: 'ROUTING_GATEWAY_NO_IPV4_WITH_INET6',
            );
        }

        return $gateway;
    }

    /**
     * Adds custom validation for the `latencyhigh` field.
     * @param int $latencyhigh The incoming `latencyhigh` value to validate.
     * @return int The validated `latencyhigh`value to set.
     */
    public function validate_latencyhigh(int $latencyhigh): int {
        # Do not allow the `latencylow` value to be greater than `latencyhigh`
        if ($this->latencylow->value > $latencyhigh) {
            throw new ValidationError(
                message: 'Routing gateway `latencyhigh` must be greater than `latencylow`.',
                response_id: 'ROUTING_GATEWAY_WITH_LATENCY_LOW_GREATER_THAN_HIGH',
            );
        }

        return $latencyhigh;
    }

    /**
     * Adds custom validation for the `losshigh` field.
     * @param int $losshigh The incoming `losshigh` value to validate.
     * @return int The validated `losshigh`value to set.
     */
    public function validate_losshigh(int $losshigh): int {
        # Do not allow the `losslow` value to be greater than `losshigh`
        if ($this->losslow->value > $losshigh) {
            throw new ValidationError(
                message: 'Routing gateway `losshigh` must be greater than `losslow`.',
                response_id: 'ROUTING_GATEWAY_WITH_LOSS_LOW_GREATER_THAN_HIGH',
            );
        }

        return $losshigh;
    }

    /**
     * Adds custom validation for the `loss_interval` field.
     * @param int $loss_interval The incoming `loss_interval` value to validate.
     * @return int The validated `loss_interval`value to set.
     */
    public function validate_loss_interval(int $loss_interval): int {
        # Do not allow the `loss_interval` value to be less than `latencyhigh`
        if ($this->latencyhigh->value > $loss_interval) {
            throw new ValidationError(
                message: 'Routing gateway `loss_interval` must be greater than or equal to `latencyhigh`.',
                response_id: 'ROUTING_GATEWAY_WITH_LOSS_INTERVAL_GREATER_THAN_LATENCY_HIGH',
            );
        }

        return $loss_interval;
    }

    /**
     * Adds custom validation for the `time_period` field.
     * @param int $time_period The incoming `time_period` value to validate.
     * @return int The validated `time_period`value to set.
     */
    public function validate_time_period(int $time_period): int {
        # Require `time_period` to be greater than twice the `interval` plus the `loss_interval`
        if ($time_period <= 2 * $this->interval->value + $this->loss_interval->value) {
            throw new ValidationError(
                message: 'Routing gateway `time_period` must be greater than twice the `interval` value plus the ' .
                    '`loss_interval` value.',
                response_id: 'ROUTING_GATEWAY_TIME_PERIOD_UNDER_MINIMUM',
            );
        }

        return $time_period;
    }

    /**
     * Adds custom validation for the `alert_interval` field.
     * @param int $alert_interval The incoming `alert_interval` value to validate.
     * @return int The validated `alert_interval`value to set.
     */
    public function validate_alert_interval(int $alert_interval): int {
        # Require `alert_interval` to be greater than or equal to the `interval`
        if ($alert_interval <= $this->interval->value) {
            throw new ValidationError(
                message: 'Routing gateway `alert_interval` must be greater than or equal to the `interval`.',
                response_id: 'ROUTING_GATEWAY_ALERT_INTERVAL_UNDER_MINIMUM',
            );
        }

        return $alert_interval;
    }

    /**
     * Defines how gateways should be applied after creation and updates.
     */
    public function apply(): void {
        (new RoutingApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Define steps to create this RoutingGateway object internally.
     */
    public function _create() {
        # Variables
        $internal_values = $this->to_internal();

        # There's an inconsistency where `save_gateway()` expects bool values as `yes` but stores them as empty
        # strings. To accommodate this, change true bool values to `yes` beforehand.
        foreach ($internal_values as $internal_name => $internal_value) {
            # Check if this is a BooleanField
            if (
                $this->$internal_name->type === 'boolean' and
                $internal_value === $this->$internal_name->indicates_true
            ) {
                $internal_values[$internal_name] = 'yes';
            }
        }

        # Call the pfSense `save_gateway()` function with the internal data of the gateway to create.
        save_gateway($internal_values);
    }

    /**
     * Define steps to update this RoutingGateway object internally.
     */
    public function _update() {
        # Variables
        $internal_values = $this->to_internal();

        # There's an inconsistency where `save_gateway()` expects bool values as `yes` but stores them as empty
        # strings. To accommodate this, change true bool values to `yes` beforehand.
        foreach ($internal_values as $internal_name => $internal_value) {
            # Check if this is a BooleanField
            if (
                $this->$internal_name->type === 'boolean' and
                $internal_value === $this->$internal_name->indicates_true
            ) {
                $internal_values[$internal_name] = 'yes';
            }
        }

        # Call the pfSense `save_gateway()` function with the internal data of the gateway to update and its ID.
        save_gateway($internal_values, $this->id);
    }

    /**
     * Overrides the `apply()` method for RoutingGateway deletions.
     */
    public function _delete() {
        # Remove the static routes for this gateway and it's monitor IP
        route_del($this->gateway->value, $this->ipprotocol->value);
        route_del($this->monitor->value, $this->ipprotocol->value);

        # Remove the default route if this is a default gateway
        if ($this->is_default_gateway()) {
            route_del('default', $this->ipprotocol->value);
        }

        # Remove the gateway from config
        $this->del_config("$this->config_path/$this->id");
        $this->write_config("Deleted $this->verbose_name via API");
    }

    /**
     * Checks if this RoutingGateway is the current default route.
     * @return bool Returns `true` if this RoutingGateway is a default route, `false` if not.
     */
    public function is_default_gateway(): bool {
        # Loop through the gateway days until we find the gateway to delete
        foreach (return_gateways_array(true, false, true, true) as $gw) {
            # Check if this gateways 'attribute' matches the ID of this RoutingGateway
            if ($this->id === $gw['attribute']) {
                if ($gw['isdefaultgw']) {
                    return true;
                }
            }
        }

        return false;
    }
}
