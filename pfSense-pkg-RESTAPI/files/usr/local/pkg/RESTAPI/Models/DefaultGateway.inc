<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RoutingApplyDispatcher;
use RESTAPI\Fields\ForeignModelField;

/**
 * Defines a Model for interacting with the system's default gateways.
 */
class DefaultGateway extends Model {
    public ForeignModelField $defaultgw4;
    public ForeignModelField $defaultgw6;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'gateways';
        $this->update_strategy = 'merge';
        $this->subsystem = 'staticroutes';
        $this->many = false;

        # Set model fields
        $this->defaultgw4 = new ForeignModelField(
            model_name: ['RoutingGateway', 'RoutingGatewayGroup'],
            model_field: 'name',
            model_query: ['ipprotocol' => 'inet'],
            allowed_keywords: ['', '-'],
            default: '',
            allow_empty: true,
            verbose_name: 'Default IPv4 Gateway',
            help_text: 'The gateway to assigns as the default IPv4 gateway for this system. Leave blank to ' .
                'automatically determine the default gateway, or set to `-` to assign no gateway.',
        );
        $this->defaultgw6 = new ForeignModelField(
            model_name: ['RoutingGateway', 'RoutingGatewayGroup'],
            model_field: 'name',
            model_query: ['ipprotocol' => 'inet6'],
            allowed_keywords: ['', '-'],
            default: '',
            allow_empty: true,
            verbose_name: 'Default IPv6 Gateway',
            help_text: 'The gateway to assigns as the default IPv6 gateway for this system. Leave blank to ' .
                'automatically determine the default gateway, or set to `-` to assign no gateway.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Apply changes to the default gateway.
     */
    public function apply(): void {
        (new RoutingApplyDispatcher(async: $this->async))->spawn_process();
    }
}
