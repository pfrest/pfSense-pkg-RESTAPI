<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\WireGuardApplyDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with the WireGuard tunnel addresses on this system.
 */
class WireGuardTunnelAddress extends Model {
    public StringField $address;
    public IntegerField $mask;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = true;
        $this->parent_model_class = 'WireGuardTunnel';
        $this->config_path = 'addresses/row';
        $this->verbose_name = 'WireGuard Tunnel Address';
        $this->packages = ['pfSense-pkg-WireGuard'];
        $this->package_includes = ['wireguard/includes/wg_service.inc', 'wireguard/includes/wg.inc'];
        $this->unique_together_fields = ['address', 'mask'];

        # Set model fields
        $this->address = new StringField(
            required: true,
            validators: [new IPAddressValidator()],
            verbose_name: 'Address',
            help_text: 'The IPv4 or IPv6 address for this WireGuard tunnel.',
        );
        $this->mask = new IntegerField(
            required: true,
            minimum: 1,
            maximum: 128,
            verbose_name: 'Mask',
            help_text: 'The subnet mask for this WireGuard tunnel.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this WireGuard tunnel address entry.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to this entire model object.
     * @throws ConflictError When this Model's address and mask are currently in use elsewhere on the system.
     */
    public function validate_extra(): void {
        # Check for conflicts with this address and mask
        $conflicts = where_is_ipaddr_configured(
            ipaddr: $this->address->value,
            check_localip: true,
            check_subnets: true,
            cidrprefix: $this->mask->value,
        );

        # Throw an error if there are conflicts
        if ($conflicts) {
            throw new ConflictError(
                message: "This tunnel's `address` and `mask` is already in use on this system.",
                response_id: 'WIREGUARD_TUNNEL_ADDRESS_AND_MASK_ALREADY_IN_USE',
            );
        }
    }

    /**
     * Serializes the parent WireGuardTunnel before applying changes.
     */
    public function pre_apply(): void {
        $this->parent_model->pre_apply();
    }

    /**
     * Applies pending changes to WireGuard tunnels.
     */
    public function apply(): void {
        (new WireGuardApplyDispatcher(async: $this->async))->spawn_process();
    }
}
