<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\WireGuardApplyDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with the WireGuard peer allowed IPs on this system.

 */
class WireGuardPeerAllowedIP extends Model {
    public StringField $address;
    public IntegerField $mask;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = true;
        $this->parent_model_class = 'WireGuardPeer';
        $this->config_path = 'allowedips/row';
        $this->verbose_name = 'WireGuard Peer Allowed IP';
        $this->packages = ['pfSense-pkg-WireGuard'];
        $this->package_includes = ['wireguard/includes/wg_service.inc', 'wireguard/includes/wg.inc'];
        $this->unique_together_fields = ['address', 'mask'];

        # Set model fields
        $this->address = new StringField(
            required: true,
            validators: [new IPAddressValidator()],
            verbose_name: 'Address',
            help_text: 'The IPv4 or IPv6 address for this peer IP.',
        );
        $this->mask = new IntegerField(
            required: true,
            minimum: 0,
            maximum: 128,
            verbose_name: 'Mask',
            help_text: 'The subnet mask for this peer IP.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this allowed peer IP.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
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
