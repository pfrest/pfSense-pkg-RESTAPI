<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';
require_once 'filter.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for managing pfSense High Availability synchronization settings.
 */
class HASync extends Model {
    public StringField $synchronizetoip;
    public StringField $pfsyncpeerip;
    public InterfaceField $pfsyncinterface;
    public StringField $pfhostid;
    public StringField $username;
    public StringField $password;

    public BooleanField $pfsyncenabled;
    public BooleanField $adminsync;

    public BooleanField $synchronizeusers;
    public BooleanField $synchronizeauthservers;
    public BooleanField $synchronizecerts;
    public BooleanField $synchronizerules;
    public BooleanField $synchronizeschedules;
    public BooleanField $synchronizealiases;
    public BooleanField $synchronizenat;
    public BooleanField $synchronizeipsec;
    public BooleanField $synchronizeopenvpn;
    public BooleanField $synchronizedhcpd;
    public BooleanField $synchronizedhcpdv6;
    public BooleanField $synchronizekea6;
    public BooleanField $synchronizewol;
    public BooleanField $synchronizestaticroutes;
    public BooleanField $synchronizevirtualip;
    public BooleanField $synchronizetrafficshaper;
    public BooleanField $synchronizetrafficshaperlimiter;
    public BooleanField $synchronizednsforwarder;
    public BooleanField $synchronizecaptiveportal;
    public BooleanField $synchronizedhcrelay;
    public BooleanField $synchronizedhcrelay6;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set Model attributes
        $this->config_path = 'hasync';
        $this->many = false;
        $this->always_apply = true;
        $this->verbose_name = 'HA Sync Settings';
        $this->verbose_name_plural = 'HA Sync Settings';

        # State Synchronization Settings (pfsync)
        $this->pfsyncenabled = $this->sync_flag('Enable pfsync state synchronization.');

        $this->pfsyncinterface = new InterfaceField(
            default: '',
            allow_empty: true,
            help_text: 'The interface used by pfsync state synchronization.',
        );

        $this->pfhostid = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 8,
            help_text: 'Custom pf host identifier carried in state data.',
        );

        $this->pfsyncpeerip = new StringField(
            default: '',
            allow_empty: true,
            validators: [
                new IPAddressValidator(
                    allow_ipv4: true,
                    allow_ipv6: false,
                ),
            ],
            help_text: 'The peer IP address used by pfsync.',
        );

        # Configuration Synchronization Settings (XMLRPC Sync)
        $this->synchronizetoip = new StringField(
            default: '',
            allow_empty: true,
            validators: [
                new IPAddressValidator(
                    allow_ipv4: true,
                    allow_ipv6: false,
                ),
            ],
            help_text: 'The remote pfSense host IP address used for XMLRPC configuration synchronization.',
        );

        $this->username = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'The remote pfSense username used for XMLRPC synchronization.',
        );

        # pfSense stores the XMLRPC password in config.xml as <passwordfld>.
        # The API exposes it as "password" but writes it internally as "passwordfld".
        $this->password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            sensitive: true,
            internal_name: 'passwordfld',
            help_text: 'The remote pfSense password used for XMLRPC synchronization.',
        );

        $this->adminsync = $this->sync_flag(
            'Synchronize admin accounts and automatically update the XMLRPC sync password.',
        );

        # Select options to sync
        $this->synchronizeusers = $this->sync_flag('Synchronize users and groups.');
        $this->synchronizeauthservers = $this->sync_flag('Synchronize authentication servers.');
        $this->synchronizecerts = $this->sync_flag('Synchronize certificates.');
        $this->synchronizerules = $this->sync_flag('Synchronize firewall rules.');
        $this->synchronizeschedules = $this->sync_flag('Synchronize firewall schedules.');
        $this->synchronizealiases = $this->sync_flag('Synchronize firewall aliases.');
        $this->synchronizenat = $this->sync_flag('Synchronize NAT configuration.');
        $this->synchronizeipsec = $this->sync_flag('Synchronize IPsec configuration.');
        $this->synchronizeopenvpn = $this->sync_flag('Synchronize OpenVPN configuration.');
        $this->synchronizedhcpd = $this->sync_flag('Synchronize DHCP server configuration.');
        $this->synchronizedhcpdv6 = $this->sync_flag('Synchronize DHCPv6 server configuration.');
        $this->synchronizekea6 = $this->sync_flag('Synchronize Kea DHCPv6 server configuration.');
        $this->synchronizedhcrelay = $this->sync_flag('Synchronize DHCP relay configuration.');
        $this->synchronizedhcrelay6 = $this->sync_flag('Synchronize DHCPv6 relay configuration.');
        $this->synchronizewol = $this->sync_flag('Synchronize Wake-on-LAN configuration.');
        $this->synchronizestaticroutes = $this->sync_flag('Synchronize static routes.');
        $this->synchronizevirtualip = $this->sync_flag('Synchronize virtual IP addresses.');
        $this->synchronizetrafficshaper = $this->sync_flag('Synchronize traffic shaper queues.');
        $this->synchronizetrafficshaperlimiter = $this->sync_flag('Synchronize traffic shaper limiters.');
        $this->synchronizednsforwarder = $this->sync_flag('Synchronize DNS Forwarder and DNS Resolver configuration.');
        $this->synchronizecaptiveportal = $this->sync_flag('Synchronize captive portal configuration.');

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * pfSense HA Sync checkboxes are stored as the string "on" when enabled.
     * When disabled, the XML key should be removed.
     */
    private function sync_flag(string $help_text): BooleanField {
        return new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: null,
            help_text: $help_text,
        );
    }

    /**
     * Applies HA Sync configuration changes.
     */
    public function apply(): bool|null {
        filter_configure_sync();
        filter_configure();

        return true;
    }
}