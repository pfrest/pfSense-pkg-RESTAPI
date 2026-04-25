<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\WireGuardApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with the WireGuard peers on this system.
 */
class WireGuardPeer extends Model {
    public BooleanField $enabled;
    public ForeignModelField $tun;
    public StringField $endpoint;
    public PortField $port;
    public StringField $descr;
    public IntegerField $persistentkeepalive;
    public StringField $publickey;
    public StringField $presharedkey;
    public NestedModelField $allowedips;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = true;
        $this->config_path = 'installedpackages/wireguard/peers/item';
        $this->verbose_name = 'WireGuard Peer';
        $this->subsystem = 'wireguard';
        $this->packages = ['pfSense-pkg-WireGuard'];
        $this->package_includes = ['wireguard/includes/wg_service.inc', 'wireguard/includes/wg.inc'];

        # Set model fields
        $this->enabled = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Enabled',
            help_text: 'Enables or disables this WireGuard peer.',
        );
        $this->tun = new ForeignModelField(
            model_name: 'WireGuardTunnel',
            model_field: 'name',
            allowed_keywords: ['unassigned'],
            default: 'unassigned',
            verbose_name: 'Tun',
            help_text: 'The WireGuard tunnel for this peer.',
        );
        $this->endpoint = new StringField(
            default: null,
            allow_null: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Endpoint',
            help_text: 'The IP address or hostname of the remote peer. Set to `null` to make this a dynamic endpoint.',
        );
        $this->port = new PortField(
            default: '51820',
            conditions: ['!endpoint' => null],
            verbose_name: 'Port',
            help_text: 'The port used by the remote peer.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The description for this peer.',
        );
        $this->persistentkeepalive = new IntegerField(
            default: null,
            allow_null: true,
            maximum: 65535,
            verbose_name: 'Persistent Keepalive',
            help_text: 'The interval (in seconds) for Keep Alive packets sent to this peer. Set to `null` to disable.',
        );
        $this->publickey = new StringField(
            required: true,
            unique: true,
            verbose_name: 'Public Key',
            help_text: 'The public key for this peer.',
        );
        $this->presharedkey = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            write_only: true,
            verbose_name: 'Pre-Shared Key',
            help_text: 'The pre-shared key for this tunnel.',
        );
        $this->allowedips = new NestedModelField(
            model_class: 'WireGuardPeerAllowedIP',
            default: [],
            allow_empty: true,
            verbose_name: 'Allowed IPs',
            help_text: 'The allowed IP/subnets for this WireGuard peer.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `presharedkey` field.
     * @param string $presharedkey The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $presharedkey is not a valid WireGuard pre-shared key
     */
    public function validate_presharedkey(string $presharedkey): string {
        # Throw an error if this value is not a valid WireGuard key
        if ($presharedkey and !wg_is_valid_key($presharedkey)) {
            throw new ValidationError(
                message: 'Field `presharedkey` must be a valid WireGuard pre-shared key.',
                response_id: 'WIREGUARD_PEER_PRESHAREDKEY_INVALID',
            );
        }

        return $presharedkey;
    }

    /**
     * Serialize changes before applying.
     */
    public function pre_apply(): void {
        wg_apply_list_add('tunnels', [$this->tun->value]);
    }

    /**
     * Applies changes to WireGuard tunnels/peers.
     */
    public function apply(): void {
        (new WireGuardApplyDispatcher(async: $this->async))->spawn_process();
    }
}
