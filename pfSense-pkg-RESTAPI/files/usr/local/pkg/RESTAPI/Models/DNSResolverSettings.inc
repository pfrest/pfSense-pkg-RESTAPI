<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\DNSResolverApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that interacts with the DNS Resolver settings.
 */
class DNSResolverSettings extends Model {
    public BooleanField $enable;
    public PortField $port;
    public BooleanField $enablessl;
    public ForeignModelField $sslcertref;
    public PortField $tlsport;
    public InterfaceField $active_interface;
    public InterfaceField $outgoing_interface;
    public BooleanField $strictout;
    public StringField $system_domain_local_zone_type;
    public BooleanField $dnssec;
    public BooleanField $python;
    public StringField $python_order;
    public StringField $python_script;
    public BooleanField $forwarding;
    public BooleanField $regdhcp;
    public BooleanField $regdhcpstatic;
    public BooleanField $regovpnclients;
    public Base64Field $custom_options;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'unbound';
        $this->subsystem = 'unbound';

        # Set model fields
        $this->enable = new BooleanField(default: false, verbose_name: 'Enable', help_text: 'Enables or disables the DNS Resolver service.');
        $this->port = new PortField(
            default: '53',
            allow_alias: false,
            allow_range: false,
            allow_empty: true,
            verbose_name: 'Port',
            help_text: 'The port on which the DNS Resolver service listens.',
        );
        $this->enablessl = new BooleanField(
            default: false,
            verbose_name: 'Enable SSL',
            help_text: 'Enables or disables SSL/TLS for the DNS Resolver service.',
        );
        $this->sslcertref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: true,
            conditions: ['enablessl' => true],
            verbose_name: 'SSL Certificate',
            help_text: 'The SSL/TLS certificate to use for the DNS Resolver service.',
        );
        $this->tlsport = new PortField(
            default: '853',
            allow_alias: false,
            allow_range: false,
            allow_empty: true,
            conditions: ['enablessl' => true],
            verbose_name: 'Tlsport',
            help_text: 'The port on which the DNS Resolver service listens for SSL/TLS connections.',
        );
        $this->active_interface = new InterfaceField(
            default: [],
            allow_localhost_interface: true,
            allow_custom: ['all', '_llocwan', '_lloclan'],
            allow_empty: true,
            many: true,
            verbose_name: 'Active Interface',
            help_text: 'The interface on which the DNS Resolver service listens for DNS queries. Set empty value ".
                "to listen on all interfaces.',
        );
        $this->outgoing_interface = new InterfaceField(
            default: [],
            allow_localhost_interface: true,
            allow_custom: ['all', '_llocwan', '_lloclan'],
            allow_empty: true,
            many: true,
            verbose_name: 'Outgoing Interface',
            help_text: 'The interface on which the DNS Resolver service sends outgoing DNS queries. Set empty value ".
                "to use any interface.',
        );
        $this->strictout = new BooleanField(
            default: false,
            verbose_name: 'Strict Outgoing',
            help_text: 'Enables or disables sending recursive queries if none of the selected Outgoing Network ".
                "Interfaces are available.',
        );
        $this->system_domain_local_zone_type = new StringField(
            default: 'transparent',
            choices: [
                'deny',
                'refuse',
                'static',
                'transparent',
                'typetransparent',
                'redirect',
                'inform',
                'inform_deny',
                'nodefault',
            ],
            verbose_name: 'System Domain Local Zone Type',
            help_text: 'The type of local zone used for the system domain.',
        );
        $this->dnssec = new BooleanField(default: false, verbose_name: 'DNSSEC', help_text: 'Enables or disables DNSSEC validation.');
        $this->python = new BooleanField(default: false, verbose_name: 'Python', help_text: 'Enables or disables the Python module.');
        $this->python_order = new StringField(
            default: 'pre_validator',
            choices: ['pre_validator', 'post_validator'],
            conditions: ['python' => true],
            verbose_name: 'Python Order',
            help_text: 'The order in which the Python module is loaded.',
        );
        $this->python_script = new StringField(
            default: '',
            choices_callable: 'get_python_script_options',
            allow_empty: true,
            conditions: ['python' => true],
            verbose_name: 'Python Script',
            help_text: 'The Python module to utilize.',
        );
        $this->forwarding = new BooleanField(
            default: false,
            verbose_name: 'Forwarding',
            help_text: 'Enables or disables DNS Resolver forwarding mode.',
        );
        $this->regdhcp = new BooleanField(
            default: false,
            verbose_name: 'Register DHCP Leases',
            help_text: 'Enables or disables registering DHCP leases in the DNS Resolver service.',
        );
        $this->regdhcpstatic = new BooleanField(
            default: false,
            verbose_name: 'Register DHCP Static Mappings',
            help_text: 'Enables or disables registering static DHCP mappings in the DNS Resolver service.',
        );
        $this->regovpnclients = new BooleanField(
            default: false,
            verbose_name: 'Register OpenVPN Clients',
            help_text: 'Enables or disables registering OpenVPN clients in the DNS Resolver service.',
        );
        $this->custom_options = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'Custom Options',
            help_text: 'Custom options to add to the DNS Resolver configuration.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains all available Python module options.
     * @return array The available Python module options.
     */
    public function get_python_script_options(): array {
        # Use glob to obtain all .py files in /var/unbound/
        $files = glob('/var/unbound/*.py');

        # Use pathinfo to get the Python module name
        $files = array_map(function ($file) {
            return pathinfo($file, PATHINFO_FILENAME);
        }, $files);

        return $files;
    }

    /**
     * Applies pending DNS Resolver changes.
     */
    public function apply(): void {
        (new DNSResolverApplyDispatcher(async: $this->async))->spawn_process();
    }
}
