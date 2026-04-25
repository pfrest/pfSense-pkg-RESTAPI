<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Auth;
use RESTAPI\Core\Model;
use RESTAPI\Core\Response;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FilterAddressField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\SpecialNetworkField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;
use RESTAPI\Responses\ServerError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that represents port forward rules.
 */
class PortForward extends Model {
    public InterfaceField $interface;
    public StringField $ipprotocol;
    public StringField $protocol;
    public FilterAddressField $source;
    public PortField $source_port;
    public FilterAddressField $destination;
    public PortField $destination_port;
    public SpecialNetworkField $target;
    public PortField $local_port;
    public BooleanField $disabled;
    public BooleanField $nordr;
    public BooleanField $nosync;
    public StringField $descr;
    public StringField $natreflection;
    public ForeignModelField $associated_rule_id;
    public UnixTimeField $created_time;
    public StringField $created_by;
    public UnixTimeField $updated_time;
    public StringField $updated_by;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'nat/rule';
        $this->many = true;
        $this->subsystem = 'natconf';
        $this->client = $options['client'] instanceof Auth ? $options['client'] : new Auth();

        # Set model Fields
        $this->interface = new InterfaceField(
            required: true,
            allow_interface_groups: true,
            allow_ipsec_interface: true,
            allow_openvpn_interface: true,
            allow_carp_interface: false,
            verbose_name: 'Interface',
            help_text: 'The interface this port forward rule applies to.',
        );
        $this->ipprotocol = new StringField(
            default: 'inet',
            choices: ['inet', 'inet6', 'inet46'],
            verbose_name: 'IP Protocol',
            help_text: 'The IP protocol this port forward rule should match.',
        );
        $this->protocol = new StringField(
            required: true,
            choices: ['any', 'tcp', 'udp', 'tcp/udp', 'icmp', 'esp', 'ah', 'gre', 'ipv6', 'igmp', 'pim', 'ospf'],
            verbose_name: 'Protocol',
            help_text: 'The IP/transport protocol this port forward rule should match.',
        );
        $this->source = new FilterAddressField(
            required: true,
            verbose_name: 'Source',
            help_text: 'The source address this port forward rule applies to.',
        );
        $this->source_port = new PortField(
            default: null,
            allow_alias: true,
            allow_range: true,
            allow_null: true,
            internal_name: 'port',
            internal_namespace: 'source',
            conditions: ['protocol' => ['tcp', 'udp', 'tcp/udp']],
            verbose_name: 'Source Port',
            help_text: 'The source port this port forward rule applies to. Set to `null` to allow any source port.',
        );
        $this->destination = new FilterAddressField(
            required: true,
            verbose_name: 'Destination',
            help_text: 'The destination address this rule applies to.',
        );
        $this->destination_port = new PortField(
            default: null,
            allow_alias: true,
            allow_range: true,
            allow_null: true,
            internal_name: 'port',
            internal_namespace: 'destination',
            conditions: ['protocol' => ['tcp', 'udp', 'tcp/udp']],
            verbose_name: 'Destination Port',
            help_text: 'The destination port this port forward rule applies to. Set to `null` to allow any destination port.',
        );
        $this->target = new SpecialNetworkField(
            required: true,
            allow_ipaddr: true,
            allow_subnet: false,
            allow_alias: true,
            allow_interface: false,
            allow_interface_ip: true,
            allow_interface_groups: false,
            allow_self: false,
            allow_l2tp: false,
            allow_pppoe: false,
            verbose_name: 'Target',
            help_text: 'The IP address or alias of the internal host to forward matching traffic to.',
        );
        $this->local_port = new PortField(
            required: true,
            allow_alias: true,
            allow_range: false,
            internal_name: 'local-port',
            conditions: ['protocol' => ['tcp', 'udp', 'tcp/udp']],
            verbose_name: 'Local Port',
            help_text: 'The port on the internal host to forward matching traffic to. In most cases, this must match ' .
                'the `destination_port` value. In the event that the `desintation_port` is a range, this value should ' .
                'be the first value in that range.',
        );
        $this->disabled = new BooleanField(
            default: false,
            verbose_name: 'Disabled',
            help_text: 'Disables this port forward rule.',
        );
        $this->nordr = new BooleanField(
            default: false,
            verbose_name: 'No Redirect',
            help_text: 'Disables redirection for traffic matching this rule.',
        );
        $this->nosync = new BooleanField(
            default: false,
            verbose_name: 'No Sync',
            help_text: 'Prevents this port forward rule from being synced to non-primary CARP members.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this port forward rule.',
        );
        $this->natreflection = new StringField(
            default: null,
            choices: ['enable', 'disable', 'purenat'],
            allow_null: true,
            verbose_name: 'NAT Reflection',
            help_text: 'The NAT reflection mode to use for traffic matching this port forward rule. Set to `null` to ' .
                'use the system default.',
        );
        $this->associated_rule_id = new ForeignModelField(
            model_name: 'FirewallRule',
            model_field: 'associated_rule_id',
            allowed_keywords: ['', 'new', 'pass'],
            default: '',
            allow_empty: true,
            internal_name: 'associated-rule-id',
            verbose_name: 'Associated Rule ID',
            help_text: 'The associated firewall rule mode. Use an empty string to require a separate firewall rule ' .
                'to be created to pass traffic matching this port forward rule. Use `new` to create a new associated ' .
                'firewall rule to pass traffic matching this port forward rule. Use `pass` to automatically pass ' .
                'traffic matching this port forward rule without the need for a firewall rule.   Otherwise, you can ' .
                'specify the `associated_rule_id` of an existing firewall rule to associate with this port forward rule.',
        );
        $this->created_time = new UnixTimeField(
            editable: false,
            read_only: true,
            auto_add_now: true,
            internal_name: 'time',
            internal_namespace: 'created',
            verbose_name: 'Created Time',
            help_text: 'The unix timestamp of when this port forward rule was original created.',
        );
        $this->created_by = new StringField(
            default: "{$this->client->username}@{$this->client->ip_address} (API)",
            editable: false,
            read_only: true,
            internal_name: 'username',
            internal_namespace: 'created',
            verbose_name: 'Created By',
            help_text: 'The username and IP of the user who originally created this port forward rule.',
        );
        $this->updated_time = new UnixTimeField(
            read_only: true,
            auto_add_now: true,
            auto_update_now: true,
            internal_name: 'time',
            internal_namespace: 'updated',
            verbose_name: 'Updated Time',
            help_text: 'The unix timestamp of when this port forward rule was original created.',
        );
        $this->updated_by = new StringField(
            default: "{$this->client->username}@{$this->client->ip_address} (API)",
            read_only: true,
            internal_name: 'username',
            internal_namespace: 'updated',
            verbose_name: 'Updated By',
            help_text: 'The username and IP of the user who last updated this port forward rule.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Overrides the default _create() method to ensure the associated_rule_id is generated if it's set to `new`.
     */
    public function _create(): void {
        # Before writing this model to the config, generate an associated_rule_id if it's set to 'new'
        if ($this->associated_rule_id->value === 'new') {
            $this->associated_rule_id->value = get_unique_id();
        }

        parent::_create();
    }

    /**
     * Creates an associated firewall rule for this port forward rule if requested after creation.
     */
    public function pre_apply_create(): void {
        # Only create a new associated rule if the associated_rule_id is not pass or empty
        if ($this->associated_rule_id->value and $this->associated_rule_id->value !== 'pass') {
            $firewall_rule = new FirewallRule(
                type: 'pass',
                disabled: $this->disabled->value,
                interface: [$this->interface->value],
                ipprotocol: $this->ipprotocol->value,
                protocol: $this->protocol->value,
                source: $this->source->value,
                source_port: $this->source_port->value,
                destination: $this->target->value,
                destination_port: $this->local_port->value,
                descr: "Associated rule for port forward rule {$this->associated_rule_id->value}",
                client: $this->client,
            );
            $firewall_rule->associated_rule_id->value = $this->associated_rule_id->value;

            # Format any errors that occur during the creation of the rule so its clear that its associated with the linked rule
            try {
                $firewall_rule->create();
            } catch (Response $e) {
                throw new ServerError(
                    message: "Failed to create associated firewall rule for port forward rule: $e->message",
                    response_id: 'PORT_FORWARD_ASSOCIATED_FIREWALL_RULE_CREATION_FAILED',
                );
            }
        }
    }

    /**
     * Updates the associated firewall rule for this port forward rule if assigned after updates
     */
    public function pre_apply_update(): void {
        # Skip this method if the associated_rule_id is not set or is set to pass
        if (!$this->associated_rule_id->value or $this->associated_rule_id->value === 'pass') {
            return;
        }

        # Query for the associated firewall rule
        $rule_q = FirewallRule::query(associated_rule_id: $this->associated_rule_id->value);

        # If the associated rule exists, update it
        if ($rule_q->exists()) {
            $firewall_rule = $rule_q->first();
            $firewall_rule->from_representation(
                type: 'pass',
                disabled: $this->disabled->value,
                interface: [$this->interface->value],
                ipprotocol: $this->ipprotocol->value,
                protocol: $this->protocol->value,
                source: $this->source->value,
                source_port: $this->source_port->value,
                destination: $this->target->value,
                destination_port: $this->local_port->value,
            );

            # Format any errors that occur during the update of the rule so its clear that its associated with the linked rule
            try {
                $firewall_rule->update();
            } catch (Response $e) {
                throw new ServerError(
                    message: "Failed to update associated firewall rule for port forward rule: $e->message",
                    response_id: 'PORT_FORWARD_ASSOCIATED_FIREWALL_RULE_UPDATE_FAILED',
                );
            }
        }
    }

    /**
     * Deletes the associated firewall rule for this port forward rule if assigned after deletion.
     */
    public function pre_apply_delete(): void {
        # Skip this method if the associated_rule_id is not set or is set to pass
        if (!$this->associated_rule_id->value or $this->associated_rule_id->value === 'pass') {
            return;
        }

        # Query for the associated firewall rule and delete it
        $rule_q = FirewallRule::query(associated_rule_id: $this->associated_rule_id->value);
        $rule_q->delete();
    }

    /**
     * Applies port forward changes.
     */
    public function apply(): void {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
