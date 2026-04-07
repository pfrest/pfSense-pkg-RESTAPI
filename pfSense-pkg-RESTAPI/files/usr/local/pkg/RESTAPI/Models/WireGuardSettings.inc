<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\WireGuardApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;

/**
 * Defines a Model for interacting with the WireGuard settings on this system.
 */
class WireGuardSettings extends Model {
    public BooleanField $enable;
    public BooleanField $keep_conf;
    public BooleanField $resolve_interval_track;
    public IntegerField $resolve_interval;
    public StringField $interface_group;
    public BooleanField $hide_secrets;
    public BooleanField $hide_peers;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/wireguard/config/0'; // pfSense parses path as indexed array?
        $this->subsystem = 'wireguard';
        $this->verbose_name = 'WireGuard Settings';
        $this->packages = ['pfSense-pkg-WireGuard'];
        $this->package_includes = ['wireguard/includes/wg_service.inc', 'wireguard/includes/wg.inc'];

        # Set model fields
        $this->enable = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: 'off',
            verbose_name: 'Enable',
            help_text: 'Enables or disables WireGuard on this system. WireGuard cannot be disabled when one or more ' .
                'tunnels is assigned to a pfSense interface.',
        );
        $this->keep_conf = new BooleanField(
            default: true,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Keep Conf',
            help_text: 'Enables or disables keeping the WireGuard configuration when the package is ' .
                'uninstalled/reinstalled.',
        );
        $this->resolve_interval_track = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Resolve Interval Track',
            help_text: "Enables or disables tracking the 'Aliases Hostnames Resolve Interval' value as the " .
                '`resolve_internal` value instead of specifying a value directly.',
        );
        $this->resolve_interval = new IntegerField(
            default: 300,
            conditions: ['resolve_interval_track' => false],
            verbose_name: 'Resolve Interval',
            help_text: 'The interval (in seconds) for re-resolving endpoint host/domain names.',
        );
        $this->interface_group = new StringField(
            default: 'all',
            choices: ['all', 'unassigned', 'none'],
            verbose_name: 'Interface Group',
            help_text: 'Configures which WireGuard tunnels are members of the WireGuard interface group.',
        );
        $this->hide_secrets = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Hide Secrets',
            help_text: 'Enables or disables hiding all secrets (private and pre-shared keys) in the user interface.',
        );
        $this->hide_peers = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Hide Peers',
            help_text: 'Enables or disables initially hiding all peers in the user interface.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `enable` field.
     * @param bool $enable The incoming value to be validated.
     * @return bool The validated value to be assigned.
     * @throws ConflictError When `enable` is set to `false` but there are WireGuard tunnels in use by one or more
     * pfSense interfaces.
     */
    public function validate_enable(bool $enable): bool {
        # Throw an error if $enable is false but there are one or more pfSense interfaces using a WireGuard tunnel
        if (!$enable and NetworkInterface::query(if__startswith: 'tun_wg')->exists()) {
            throw new ConflictError(
                message: 'Field `enable` cannot be set to `false` when WireGuard tunnels are in use by an existing ' .
                    'pfSense interface.',
                response_id: 'WIREGUARD_SETTINGS_CANNOT_DISABLE_WITH_ACTIVE_TUNNELS',
            );
        }

        return $enable;
    }

    /**
     * Applies pending changes to WireGuard settings.
     */
    public function apply(): void {
        (new WireGuardApplyDispatcher(async: $this->async))->spawn_process();
    }
}
