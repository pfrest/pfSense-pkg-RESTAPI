<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\SystemHasyncApplyDispatcher;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that represents the HA Sync (High Availability synchronization) configuration on this system.
 */
class SystemHasync extends Model {
    public StringField $synchronizetoip;
    public StringField $pfsyncpeerip;
    public InterfaceField $pfsyncinterface;
    public StringField $username;
    public StringField $password;
    public StringField $synchronizeusers;
    public StringField $synchronizeauthservers;
    public StringField $synchronizecerts;
    public StringField $synchronizerules;
    public StringField $synchronizeschedules;
    public StringField $synchronizealiases;
    public StringField $synchronizenat;
    public StringField $synchronizeipsec;
    public StringField $synchronizeopenvpn;
    public StringField $synchronizedhcpd;
    public StringField $synchronizedhcrelay;
    public StringField $synchronizedhcrelay6;
    public StringField $synchronizewol;
    public StringField $synchronizestaticroutes;
    public StringField $synchronizevirtualip;
    public StringField $synchronizetrafficshaper;
    public StringField $synchronizetrafficshaperlimiter;
    public StringField $synchronizednsforwarder;
    public StringField $synchronizecaptiveportal;
    public StringField $pfsyncenabled;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'hasync';
        $this->subsystem = 'system';
        $this->update_strategy = 'merge';
        $this->always_apply = true;
        $this->verbose_name = 'HA Sync Settings';

        # Set model fields
        $this->synchronizetoip = new StringField(
            allow_null: true,
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Synchronize To IP',
            help_text: 'IP address or FQDN of the HA peer to synchronize configuration to.',
        );
        $this->pfsyncpeerip = new StringField(
            allow_null: true,
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'PFSync Peer IP',
            help_text: 'IP address of the PFSync peer for state synchronization.',
        );
        $this->pfsyncinterface = new InterfaceField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'PFSync Interface',
            help_text: 'Interface used for PFSync traffic between HA peers.',
        );
        $this->username = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Username',
            help_text: 'Username for authenticating to the HA peer.',
        );
        $this->password = new StringField(
            allow_null: true,
            allow_empty: true,
            write_only: true,
            sensitive: true,
            verbose_name: 'Password',
            help_text: 'Password for authenticating to the HA peer.',
        );
        $this->synchronizeusers = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Users',
            help_text: 'Synchronize user accounts between HA peers.',
        );
        $this->synchronizeauthservers = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Auth Servers',
            help_text: 'Synchronize authentication server configurations.',
        );
        $this->synchronizecerts = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Certificates',
            help_text: 'Synchronize certificates and CA bundles.',
        );
        $this->synchronizerules = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Rules',
            help_text: 'Synchronize firewall rules.',
        );
        $this->synchronizeschedules = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Schedules',
            help_text: 'Synchronize firewall schedule entries.',
        );
        $this->synchronizealiases = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Aliases',
            help_text: 'Synchronize firewall aliases.',
        );
        $this->synchronizenat = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize NAT',
            help_text: 'Synchronize NAT rules.',
        );
        $this->synchronizeipsec = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize IPsec',
            help_text: 'Synchronize IPsec VPN configurations.',
        );
        $this->synchronizeopenvpn = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize OpenVPN',
            help_text: 'Synchronize OpenVPN configurations.',
        );
        $this->synchronizedhcpd = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize DHCP Server',
            help_text: 'Synchronize DHCP server configurations.',
        );
        $this->synchronizedhcrelay = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize DHCP Relay (IPv4)',
            help_text: 'Synchronize DHCP relay configurations for IPv4.',
        );
        $this->synchronizedhcrelay6 = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize DHCP Relay (IPv6)',
            help_text: 'Synchronize DHCP relay configurations for IPv6.',
        );
        $this->synchronizewol = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize WoL',
            help_text: 'Synchronize Wake-on-LAN settings.',
        );
        $this->synchronizestaticroutes = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Static Routes',
            help_text: 'Synchronize static route entries.',
        );
        $this->synchronizevirtualip = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Virtual IPs',
            help_text: 'Synchronize virtual IP addresses.',
        );
        $this->synchronizetrafficshaper = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Traffic Shaper',
            help_text: 'Synchronize traffic shaper rules.',
        );
        $this->synchronizetrafficshaperlimiter = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Traffic Shaper Limiters',
            help_text: 'Synchronize traffic shaper limiter configurations.',
        );
        $this->synchronizednsforwarder = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize DNS Forwarder',
            help_text: 'Synchronize DNS forwarder configurations.',
        );
        $this->synchronizecaptiveportal = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'Synchronize Captive Portal',
            help_text: 'Synchronize captive portal configurations.',
        );
        $this->pfsyncenabled = new StringField(
            allow_null: true,
            allow_empty: true,
            verbose_name: 'PFSync Enabled',
            help_text: 'Enable PFSync state synchronization.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Apply HA Sync configuration changes by reloading filter and sync settings.
     */
    public function apply(): void {
        (new SystemHasyncApplyDispatcher(async: $this->async))->spawn_process();
    }
}
