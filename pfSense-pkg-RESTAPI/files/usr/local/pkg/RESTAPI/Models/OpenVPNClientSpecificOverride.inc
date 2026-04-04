<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\HostnameValidator;
use RESTAPI\Validators\IPAddressValidator;
use RESTAPI\Validators\SubnetValidator;

/**
 * Defines a Model that represents OpenVPN Client Specific Overrides.
 */
class OpenVPNClientSpecificOverride extends Model {
    public StringField $common_name;
    public BooleanField $disable;
    public BooleanField $block;
    public StringField $description;
    public ForeignModelField $server_list;
    public StringField $tunnel_network;
    public StringField $tunnel_networkv6;
    public StringField $local_network;
    public StringField $local_networkv6;
    public StringField $remote_network;
    public StringField $remote_networkv6;
    public BooleanField $gwredir;
    public BooleanField $push_reset;
    public StringField $remove_options;
    public StringField $dns_domain;
    public StringField $dns_server1;
    public StringField $dns_server2;
    public StringField $dns_server3;
    public StringField $dns_server4;
    public StringField $ntp_server1;
    public StringField $ntp_server2;
    public BooleanField $netbios_enable;
    public IntegerField $netbios_ntype;
    public StringField $netbios_scope;
    public StringField $wins_server1;
    public StringField $wins_server2;
    public StringField $custom_options;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->many = true;
        $this->config_path = 'openvpn/openvpn-csc';
        $this->always_apply = true;
        $this->verbose_name = 'OpenVPN Client Specific Override';

        # Set model fields
        $this->common_name = new StringField(
            required: true,
            help_text: 'The X.509 common name for the client certificate, or the username for VPNs utilizing ' .
                'password authentication.',
        );
        $this->disable = new BooleanField(default: false, help_text: 'Disables this client specific override.');
        $this->block = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables the client from connecting to this server. Do not use this option ' .
                'to permanently disable a client due to a compromised key or password. Use a CRL instead.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'The description for this client specific override.',
        );
        $this->server_list = new ForeignModelField(
            model_name: 'OpenVPNServer',
            model_field: 'vpnif',
            model_field_internal: 'vpnid',
            default: [],
            allow_empty: true,
            many: true,
            help_text: 'The OpenVPN servers that will utilize this override. When no servers are specified, the ' .
                'override will apply to all servers.',
        );
        $this->tunnel_network = new StringField(
            default: '',
            allow_empty: true,
            validators: [new SubnetValidator(allow_ipv4: true, allow_ipv6: false, allow_alias: false)],
            help_text: 'The IPv4 virtual network used for private communications between the server and client hosts.',
        );
        $this->tunnel_networkv6 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new SubnetValidator(allow_ipv4: false, allow_ipv6: true, allow_alias: false)],
            help_text: 'The IPv6 virtual network used for private communications between the server and client hosts.',
        );
        $this->local_network = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            validators: [new SubnetValidator(allow_ipv4: true, allow_ipv6: false, allow_alias: true)],
            help_text: 'The IPv4 server-side networks that will be accessible from this particular client.',
        );
        $this->local_networkv6 = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            validators: [new SubnetValidator(allow_ipv4: false, allow_ipv6: true, allow_alias: true)],
            help_text: 'the IPv6 server-side networks that will be accessible from this particular client.',
        );
        $this->remote_network = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            validators: [new SubnetValidator(allow_ipv4: true, allow_ipv6: false, allow_alias: true)],
            help_text: 'The IPv4 client-side networks that will be routed to this client specifically using iroute, ' .
                'so that a site-to-site VPN can be established.',
        );
        $this->remote_networkv6 = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            validators: [new SubnetValidator(allow_ipv4: false, allow_ipv6: true, allow_alias: true)],
            help_text: 'The IPv6 client-side networks that will be routed to this client specifically using iroute, ' .
                'so that a site-to-site VPN can be established.',
        );
        $this->gwredir = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enable forcing all client-generated traffic through the tunnel.',
        );
        $this->push_reset = new BooleanField(
            default: false,
            help_text: 'Enables or disables preventing this client from receiving any server-defined client settings.',
        );
        $this->remove_options = new StringField(
            default: [],
            choices: [
                'remove_route',
                'remove_iroute',
                'remove_redirect_gateway',
                'remove_inactive',
                'remove_ping',
                'remove_ping_action',
                'remove_dnsdomain',
                'remove_dnsservers',
                'remove_blockoutsidedns',
                'remove_ntpservers',
                'remove_netbios_ntype',
                'remove_netbios_scope',
                'remove_wins',
            ],
            many: true,
            conditions: ['push_reset' => false],
            help_text: 'Specifies the push-remove options to apply to the client',
        );
        $this->dns_domain = new StringField(
            default: '',
            allow_empty: true,
            validators: [new HostnameValidator(allow_hostname: true, allow_domain: true, allow_fqdn: true)],
            help_text: 'The default domain to provide to the client.',
        );
        $this->dns_server1 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The primary DNS server to provide to the client.',
        );
        $this->dns_server2 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The secondary DNS server to provide to the client.',
        );
        $this->dns_server3 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The tertiary DNS server to provide to the client.',
        );
        $this->dns_server4 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The quaternary DNS server to provide to the client.',
        );
        $this->ntp_server1 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The primary NTP server to provide to the client.',
        );
        $this->ntp_server2 = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The secondary NTP server to provide to the client.',
        );
        $this->netbios_enable = new BooleanField(
            default: false,
            indicates_true: 'yes',
            indicates_false: '',
            help_text: 'Enables or disables NetBIOS over TCP/IP.',
        );
        $this->netbios_ntype = new IntegerField(
            default: 0,
            choices: [0, 1, 2, 4, 8],
            conditions: ['netbios_enable' => true],
            help_text: 'The NetBIOS node type.',
        );
        $this->netbios_scope = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['netbios_enable' => true],
            help_text: 'The NetBIOS Scope ID. This provides an extended naming service for NetBIOS over TCP/IP. The ' .
                'NetBIOS scope ID isolates NetBIOS traffic on a single network to only those nodes with the same ' .
                'NetBIOS scope ID.',
        );
        $this->wins_server1 = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['netbios_enable' => true],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The primary WINS server to provide to the client.',
        );
        $this->wins_server2 = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['netbios_enable' => true],
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true)],
            help_text: 'The secondary WINS server to provide to the client.',
        );
        $this->custom_options = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: ';',
            help_text: 'Additional OpenVPN options to add for this client.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to this client specific override.
     */
    public function apply(): void {
        # Delete the previous CSO if set
        if ($this->initial_object) {
            openvpn_delete_csc($this->initial_object->to_internal());
        }

        # Setup this CSO
        openvpn_resync_csc($this->to_internal());
        services_unbound_configure(false);
    }

    /**
     * Applies the deletion of this client specific override.
     */
    public function apply_delete(): void {
        openvpn_delete_csc($this->to_internal());
        services_unbound_configure(false);
    }
}
