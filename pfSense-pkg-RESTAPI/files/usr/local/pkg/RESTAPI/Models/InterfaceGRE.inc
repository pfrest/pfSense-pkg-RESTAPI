<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with Interface GRE Tunnels.
 */
class InterfaceGRE extends Model {
    public InterfaceField $if;
    public StringField $greif;
    public StringField $descr;
    public BooleanField $add_static_route;
    public StringField $remote_addr;
    public StringField $tunnel_local_addr;
    public StringField $tunnel_remote_addr;
    public IntegerField $tunnel_remote_net;
    public StringField $tunnel_local_addr6;
    public StringField $tunnel_remote_addr6;
    public IntegerField $tunnel_remote_net6;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'gres/gre';
        $this->many = true;
        $this->always_apply = true;

        # Set model fields
        $this->if = new InterfaceField(
            required: true,
            verbose_name: 'Interface Name',
            help_text: 'The pfSense interface interface serving as the local address to be used for the GRE tunnel.',
        );
        $this->greif = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'GRE Interface',
            help_text: 'The real interface name for this GRE interface.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this GRE interface.',
        );
        $this->add_static_route = new BooleanField(
            default: false,
            internal_name: 'link1',
            verbose_name: 'Add Static Route',
            help_text: 'Whether to add an explicit static route for the remote inner tunnel address/subnet via the ' .
                'local tunnel address.',
        );
        $this->remote_addr = new StringField(
            required: true,
            internal_name: 'remote-addr',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'Remote Addr',
            help_text: 'The remote address to use for the GRE tunnel.',
        );
        $this->tunnel_local_addr = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'tunnel-local-addr',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Tunnel Local Address',
            help_text: 'The local IPv4 address to use for the GRE tunnel.',
        );
        $this->tunnel_remote_addr = new StringField(
            required: true,
            unique: true,
            internal_name: 'tunnel-remote-addr',
            conditions: ['!tunnel_local_addr' => null],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: false)],
            verbose_name: 'Tunnel Remote Address',
            help_text: 'The remote IPv4 address to use for the GRE tunnel.',
        );
        $this->tunnel_remote_net = new IntegerField(
            default: 32,
            minimum: 1,
            maximum: 32,
            internal_name: 'tunnel-remote-net',
            conditions: ['!tunnel_local_addr' => null],
            verbose_name: 'Tunnel Remote Network',
            help_text: 'The remote IPv4 subnet bitmask to use for the GRE tunnel.',
        );
        $this->tunnel_local_addr6 = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'tunnel-local-addr6',
            validators: [new IPAddressValidator(allow_ipv4: false, allow_ipv6: true)],
            verbose_name: 'Tunnel Local IPv6 Address',
            help_text: 'The local IPv6 address to use for the GRE tunnel.',
        );
        $this->tunnel_remote_addr6 = new StringField(
            required: true,
            unique: true,
            internal_name: 'tunnel-remote-addr6',
            conditions: ['!tunnel_local_addr6' => null],
            validators: [new IPAddressValidator(allow_ipv4: false, allow_ipv6: true)],
            verbose_name: 'Tunnel Remote IPv6 Address',
            help_text: 'The remote IPv6 address to use for the GRE tunnel.',
        );
        $this->tunnel_remote_net6 = new IntegerField(
            default: 128,
            minimum: 1,
            maximum: 128,
            internal_name: 'tunnel-remote-net6',
            conditions: ['!tunnel_local_addr6' => null],
            verbose_name: 'Tunnel Remote IPv6 Network',
            help_text: 'The remote IPv6 subnet bitmask to use for the GRE tunnel.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to this entire Model.
     * @throws ValidationError If neither a local IPv4 nor IPv6 tunnel address is present.
     */
    public function validate_extra(): void {
        # Require either a local IPv4 and/or IPv6 address to present
        if (!$this->tunnel_local_addr->value and !$this->tunnel_local_addr6->value) {
            throw new ValidationError(
                message: 'GRE tunnel must have a `tunnel_local_addr` and/or `tunnel_local_addr6`.',
                response_id: 'INTERFACE_GRE_HAS_NO_LOCAL_ADDRESS',
            );
        }
    }

    /**
     * Applies changes to this Interface GRE Tunnel.
     */
    public function apply(): void {
        # If the greif is already assigned to an interface, reconfigure the interface
        $if_q = NetworkInterface::query(if: $this->greif->value);
        if ($if_q->exists()) {
            interface_configure($if_q->first()->id);
        }
    }

    /**
     * Applies the deletion of this Interface GRE Tunnel.
     */
    public function apply_delete(): void {
        pfSense_interface_destroy($this->greif->value);
    }

    /**
     * Extend the default _create method to create the GRE interface and obtain the real interface name.
     */
    public function _create(): void {
        $this->greif->value = interface_gre_configure($this->to_internal());
        parent::_create();
    }

    /**
     * Extend the default _update method to reconfigure the GRE interface.
     */
    public function _update(): void {
        interface_gre_configure($this->to_internal());
        parent::_update();
    }

    /**
     * Extend the default _delete method to prevent deletion of the GRE interface while in use.
     * @throws ConflictError If the GRE interface is in use.
     */
    public function _delete(): void {
        # If the greif is in use, don't allow deletion
        $if_q = NetworkInterface::query(if: $this->greif->value);
        if ($if_q->exists()) {
            throw new ConflictError(
                message: 'Cannot delete GRE interface while it is in use.',
                response_id: 'INTERFACE_GRE_CANNOT_DELETE_WHILE_IN_USE',
            );
        }

        parent::_delete();
    }
}
