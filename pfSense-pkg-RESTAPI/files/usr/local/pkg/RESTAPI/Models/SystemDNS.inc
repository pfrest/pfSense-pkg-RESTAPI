<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\SystemHostnameApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that represents the DNS configuration on this system.
 */
class SystemDNS extends Model {
    public BooleanField $dnsallowoverride;
    public StringField $dnslocalhost;
    public StringField $dnsserver;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'system';
        $this->update_strategy = 'merge';
        $this->always_apply = true;

        # Set model Fields
        $this->dnsallowoverride = new BooleanField(
            default: false,
            verbose_name: 'DNS Allow Override',
            help_text: 'Allow DNS servers to be overwritten by DHCP on WAN interfaces.',
        );
        $this->dnslocalhost = new StringField(
            default: null,
            choices: ['local', 'remote'],
            allow_null: true,
            verbose_name: 'DNS Localhost',
            help_text: 'Use local DNS server (DNS Resover or DNS Forwarder) as the primary DNS, or use only remote ' .
                'DNS servers specified in `dnsserver`. Set to `null` to use local DNS server as the primary and ' .
                'remote DNS servers as backup.',
        );
        $this->dnsserver = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: null,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            verbose_name: 'DNS Servers',
            help_text: 'The remote DNS server IPv4 or IPv6 addresses.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Apply these DNS server changes to the system and restart associated services.
     */
    public function apply() {
        (new SystemHostnameApplyDispatcher(async: $this->async))->spawn_process();
    }
}
