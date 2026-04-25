<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Auth;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FilterAddressField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model that interacts with firewall rules.
 */
class FirewallRule extends Model {
    public StringField $type;
    public InterfaceField $interface;
    public StringField $ipprotocol;
    public StringField $protocol;
    public StringField $icmptype;
    public FilterAddressField $source;
    public PortField $source_port;
    public FilterAddressField $destination;
    public PortField $destination_port;
    public StringField $descr;
    public BooleanField $disabled;
    public BooleanField $log;
    public StringField $tag;
    public StringField $statetype;
    public BooleanField $tcp_flags_any;
    public StringField $tcp_flags_out_of;
    public StringField $tcp_flags_set;
    public ForeignModelField $gateway;
    public ForeignModelField $sched;
    public StringField $dnpipe;
    public StringField $pdnpipe;
    public ForeignModelField $defaultqueue;
    public ForeignModelField $ackqueue;
    public BooleanField $floating;
    public BooleanField $quick;
    public StringField $direction;
    public UnixTimeField $tracker;
    public StringField $associated_rule_id;
    public UnixTimeField $created_time;
    public StringField $created_by;
    public UnixTimeField $updated_time;
    public StringField $updated_by;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'filter/rule';
        $this->many = true;
        $this->many_minimum = 1;
        $this->client = $options['client'] ?: new Auth();

        # Set model Fields
        $this->type = new StringField(
            required: true,
            choices: ['pass', 'block', 'reject'],
            verbose_name: 'Type',
            help_text: 'The action to take against traffic that matches this rule.',
        );
        $this->interface = new InterfaceField(
            required: true,
            allow_interface_groups: true,
            allow_ipsec_interface: true,
            allow_openvpn_interface: true,
            allow_custom: ['any'],
            many: true,
            verbose_name: 'Interface',
            help_text: 'The interface where packets must originate to match this rule.',
        );
        $this->ipprotocol = new StringField(
            required: true,
            choices: ['inet', 'inet6', 'inet46'],
            verbose_name: 'IP Protocol',
            help_text: 'The IP version(s) this rule applies to.',
        );
        $this->protocol = new StringField(
            default: null,
            choices: [
                'tcp',
                'udp',
                'tcp/udp',
                'icmp',
                'esp',
                'ah',
                'gre',
                'ipv6',
                'igmp',
                'pim',
                'ospf',
                'carp',
                'pfsync',
            ],
            allow_null: true,
            verbose_name: 'Protocol',
            help_text: 'The IP/transport protocol this rule should match.',
        );
        $this->icmptype = new StringField(
            default: ['any'],
            choices: [
                'any',
                'althost',
                'dataconv',
                'echorep',
                'echoreq',
                'inforep',
                'inforeq',
                'ipv6-here',
                'ipv6-where',
                'maskrep',
                'maskreq',
                'mobredir',
                'mobregrep',
                'mobregreq',
                'paramprob',
                'photuris',
                'redir',
                'routeradv',
                'routersol',
                'skip',
                'squench',
                'timerep',
                'timereq',
                'timex',
                'trace',
                'unreach',
            ],
            many: true,
            conditions: ['protocol' => 'icmp'],
            verbose_name: 'ICMP Types',
            help_text: 'Th ICMP subtypes this rule applies to. This field is only applicable when ' .
                '`ipprotocol` is `inet` and `protocol` is `icmp`.',
        );
        $this->source = new FilterAddressField(
            required: true,
            verbose_name: 'Source',
            help_text: 'The source address this rule applies to.',
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
            help_text: 'The source port this rule applies to. Set to `null` to allow any source port.',
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
            help_text: 'The destination port this rule applies to. Set to `null` to allow any destination port.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description detailing the purpose or justification of this firewall rule.',
        );
        $this->disabled = new BooleanField(
            default: false,
            verbose_name: 'Disabled',
            help_text: 'Enable or disable this firewall rule.',
        );
        $this->log = new BooleanField(
            default: false,
            verbose_name: 'Log',
            help_text: 'Enable or disable logging of traffic that matches this rule.',
        );
        $this->tag = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Tag',
            help_text: 'A packet matching this rule can be marked and this mark used to match on other NAT/filter rules. It is called ',
        );
        $this->statetype = new StringField(
            default: 'keep state',
            choices: ['keep state', 'sloppy state', 'synproxy state', 'none'],
            verbose_name: 'State Type',
            help_text: 'The state mechanism to use for this firewall rule.',
        );
        $this->tcp_flags_any = new BooleanField(
            default: false,
            internal_name: 'tcpflags_any',
            verbose_name: 'TCP Flags Any',
            help_text: 'Allow any TCP flags.',
        );
        $this->tcp_flags_out_of = new StringField(
            default: null,
            choices: ['fin', 'syn', 'rst', 'psh', 'ack', 'urg', 'ece', 'cwr'],
            allow_null: true,
            many: true,
            internal_name: 'tcpflags2',
            conditions: ['tcp_flags_any' => false],
            verbose_name: 'TCP Flags Out Of',
            help_text: 'The TCP flags that can be set for this rule to match.',
        );
        $this->tcp_flags_set = new StringField(
            default: null,
            choices: ['fin', 'syn', 'rst', 'psh', 'ack', 'urg', 'ece', 'cwr'],
            allow_null: true,
            many: true,
            internal_name: 'tcpflags1',
            conditions: ['tcp_flags_any' => false],
            verbose_name: 'TCP Flags Set',
            help_text: 'The TCP flags that must be set for this rule to match.',
        );
        $this->gateway = new ForeignModelField(
            model_name: ['RoutingGateway', 'RoutingGatewayGroup', 'RoutingGatewayStatus'],
            model_field: 'name',
            default: null,
            allow_null: true,
            verbose_name: 'Gateway',
            help_text: 'The gateway traffic matching this rule will be routed to. Set to `null` to use default.',
        );
        $this->sched = new ForeignModelField(
            model_name: 'FirewallSchedule',
            model_field: 'name',
            default: null,
            allow_null: true,
            verbose_name: 'Schedule',
            help_text: 'The name of an existing firewall schedule to assign to this firewall rule.',
        );
        $this->dnpipe = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'Limiter Pipe (In)',
            help_text: 'The name of the traffic shaper limiter pipe or queue to use for incoming traffic.',
        );
        $this->pdnpipe = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'Limiter Pipe (Out)',
            help_text: 'The name of the traffic shaper limiter pipe or queue to use for outgoing traffic.',
        );
        $this->defaultqueue = new ForeignModelField(
            model_name: 'TrafficShaperQueue',
            model_field: 'name',
            default: null,
            allow_null: true,
            verbose_name: 'Default Queue',
            help_text: 'The name of the traffic shaper queue to assume as the default queue for traffic matching ' .
                'this rule.',
        );
        $this->ackqueue = new ForeignModelField(
            model_name: 'TrafficShaperQueue',
            model_field: 'name',
            default: null,
            allow_null: true,
            verbose_name: 'ACK Queue',
            help_text: 'The name of the traffic shaper queue to assume as the ACK queue for ACK traffic matching ' .
                'this rule.',
        );
        $this->floating = new BooleanField(
            default: false,
            editable: false,
            indicates_true: 'yes',
            verbose_name: 'Floating',
            help_text: 'Mark this rule as a floating firewall rule.',
        );
        $this->quick = new BooleanField(
            default: false,
            indicates_true: 'yes',
            conditions: ['floating' => true],
            verbose_name: 'Quick',
            help_text: 'Apply this action to traffic that matches this rule immediately. This field only applies ' .
                'to floating firewall rules.',
        );
        $this->direction = new StringField(
            default: 'any',
            choices: ['any', 'in', 'out'],
            conditions: ['floating' => true],
            verbose_name: 'Direction',
            help_text: 'The direction of traffic this firewall rule applies to. This field only applies to ' .
                'floating firewall rules.',
        );
        $this->tracker = new UnixTimeField(
            editable: false,
            read_only: true,
            auto_add_now: true,
            verbose_name: 'Tracker',
            help_text: 'The internal tracking ID for this firewall rule.',
        );
        $this->associated_rule_id = new StringField(
            unique: true,
            default: null,
            allow_null: true,
            editable: false,
            read_only: true,
            internal_name: 'associated-rule-id',
            verbose_name: 'Associated Rule ID',
            help_text: 'The internal rule ID for the NAT rule associated with this rule.',
        );
        $this->created_time = new UnixTimeField(
            editable: false,
            read_only: true,
            auto_add_now: true,
            internal_name: 'time',
            internal_namespace: 'created',
            verbose_name: 'Created Time',
            help_text: 'The unix timestamp of when this firewall rule was original created.',
        );
        $this->created_by = new StringField(
            default: "{$this->client->username}@{$this->client->ip_address} (API)",
            editable: false,
            read_only: true,
            internal_name: 'username',
            internal_namespace: 'created',
            verbose_name: 'Created By',
            help_text: 'The username and IP of the user who originally created this firewall rule.',
        );
        $this->updated_time = new UnixTimeField(
            read_only: true,
            auto_add_now: true,
            auto_update_now: true,
            internal_name: 'time',
            internal_namespace: 'updated',
            verbose_name: 'Updated Time',
            help_text: 'The unix timestamp of when this firewall rule was original created.',
        );
        $this->updated_by = new StringField(
            default: "{$this->client->username}@{$this->client->ip_address} (API)",
            read_only: true,
            internal_name: 'username',
            internal_namespace: 'updated',
            verbose_name: 'Updated By',
            help_text: 'The username and IP of the user who last updated this firewall rule.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `interface` field.
     * @param string $interface The incoming value to be validated.
     * @return string The validated value to be set.
     * @throws ValidationError When multiple `interface` values are assigned but `floating` is not enabled
     */
    public function validate_interface(string $interface): string {
        # Do not allow more than one interface to be assigned if this is not a floating rule
        if (!$this->floating->value and count($this->interface->value) > 1) {
            throw new ValidationError(
                message: 'Field `interface` cannot contain more than one value unless `floating` is enabled.',
                response_id: 'FIREWALL_RULE_MULTIPLE_INTERFACE_WITHOUT_FLOATING',
            );
        }

        return $interface;
    }

    /**
     * Adds extra validation to the `statetype` field.
     * @param string $statetype The incoming value to be validated.
     * @return string The validated value to be set.
     */
    public function validate_statetype(string $statetype): string {
        # Do not allow the `synproxy state` value to be set if `protocol` is not `tcp`
        if ($statetype === 'synproxy state' and $this->protocol->value !== 'tcp') {
            throw new ValidationError(
                message: 'Field `statetype` cannot be set to `synproxy state` unless `protocol` is set to `tcp`.',
                response_id: 'FIREWALL_RULE_SYNPROXY_STATE_TYPE_WITH_NON_TCP_PROTOCOL',
            );
        }

        # Do not allow the `synproxy state` value to be set if a `gateway` is set
        if ($statetype === 'synproxy state' and $this->gateway->value) {
            throw new ValidationError(
                message: 'Field `statetype` cannot be set to `synproxy state` when a `gateway` value is set.',
                response_id: 'FIREWALL_RULE_SYNPROXY_STATE_TYPE_WITH_GATEWAY',
            );
        }

        return $statetype;
    }

    /**
     * Adds extra validation to the `tcp_flags_set` field.
     * @param string $tcp_flags_set The incoming value to be validated.
     * @return string The validated value to be set.
     */
    public function validate_tcp_flags_set(string $tcp_flags_set): string {
        # Ensure this value is also present in `tcp_flags_out_of`
        if (!in_array($tcp_flags_set, $this->tcp_flags_out_of->value)) {
            throw new ValidationError(
                message: "Field `tcp_flags_set` value `$tcp_flags_set` is not present in `tcp_flags_out_of`.",
                response_id: 'FIREWALL_RULE_TCP_FLAGS_SET_NOT_IN_TCP_FLAGS_OUT_OF',
            );
        }

        return $tcp_flags_set;
    }

    /**
     * Adds extra validation to the `dnpipe` field.
     * @param string $dnpipe The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws NotFoundError When $dnpipe is not an existing TrafficShaperLimiter or TrafficShaperLimiterQueue name
     */
    public function validate_dnpipe(string $dnpipe): string {
        # Throw an error if $dnpipe is not an existing TrafficShaperLimiter or TrafficShaperLimiterQueue
        if (!$this->get_dnpipe_by_name($dnpipe)) {
            throw new NotFoundError(
                message: 'Field `dnpipe` must be an existing Traffic Shaper Limiter or Limiter Queue. Could not ' .
                    "locate object with `name` value `$dnpipe`",
                response_id: 'FIREWALL_RULE_DNPIPE_NOT_FOUND',
            );
        }

        return $dnpipe;
    }

    /**
     * Adds extra validation to the `pdnpipe` field.
     * @param string $pdnpipe The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $pdnpipe is defined but the `dnpipe` field was not.
     * @throws NotFoundError When $pdnpipe is not an existing TrafficShaperLimiter or TrafficShaperLimiterQueue name
     * @throws ConflictError When $pdnpipe and `dnpipe` are not of the same type (pipe or queue).
     */
    public function validate_pdnpipe(string $pdnpipe): string {
        # Throw an error if $pdnpipe has a value but `dnpipe` does not
        if ($pdnpipe and !$this->dnpipe->value) {
            throw new ValidationError(
                message: 'Field `pdnpipe` cannot be set unless `dnpipe` is also set.',
                response_id: 'FIREWALL_RULE_PDNPIPE_SET_WITHOUT_DNPIPE',
            );
        }

        # Throw an error if $pdnpipe matches the value `dnpipe` exactly
        if ($pdnpipe === $this->dnpipe->value) {
            throw new ValidationError(
                message: 'Field `pdnpipe` cannot use the same limiter as `dnpipe`.',
                response_id: 'FIREWALL_RULE_PDNPIPE_CANNOT_MATCH_DNPIPE',
            );
        }

        # Throw an error if $pdnpipe is not an existing TrafficShaperLimiter or TrafficShaperLimiterQueue
        $pdnpipe_obj = $this->get_dnpipe_by_name($pdnpipe);
        if (!$pdnpipe_obj) {
            throw new NotFoundError(
                message: 'Field `pdnpipe` must be an existing Traffic Shaper Limiter or Limiter Queue. Could not ' .
                    "locate object with `name` value `$pdnpipe`",
                response_id: 'FIREWALL_RULE_PDNPIPE_NOT_FOUND',
            );
        }

        # Throw an error if dnpipe and pdnpipe are not of the same class
        $dnpipe_obj = $this->get_dnpipe_by_name($this->dnpipe->value);
        if (get_class($dnpipe_obj) !== get_class($pdnpipe_obj)) {
            throw new ConflictError(
                message: 'Field `pdnpipe` must be of the same limiter type as `dnpipe`. A pipe and a queue cannot be ' .
                    'used concurrently.',
                response_id: 'FIREWALL_RULE_PDNPIPE_DOES_NOT_MATCH_DNPIPE_LIMITER_TYPE',
            );
        }

        return $pdnpipe;
    }

    /**
     * Adds extra validation to the `ackqueue` field.
     * @param string $ackqueue The incoming value to be validated.
     * @return string The value to be assigned.
     * @throws ValidationError When both the `ackqueue` and `defaultqueue` are the same.
     * @throws ValidationError When `ackqueue` is defined but `defaultqueue` is not.
     */
    public function validate_ackqueue(string $ackqueue): string {
        # Throw an error if ackqueue is defined, but defaultqueue is not
        if ($ackqueue and !$this->defaultqueue->value) {
            throw new ValidationError(
                message: 'Field `ackqueue` cannot be defined without `defaultqueue`.',
                response_id: 'FIREWALL_RULE_ACKQUEUE_WITHOUT_DEFAULTQUEUE',
            );
        }

        # Throw an error if the defaultqueue and ackqueue are the same
        if ($ackqueue === $this->defaultqueue->value) {
            throw new ValidationError(
                message: 'Field `ackqueue` cannot match the value of `defaultqueue`.',
                response_id: 'FIREWALL_RULE_ACKQUEUE_MATCHES_DEFAULTQUEUE',
            );
        }

        return $ackqueue;
    }

    /**
     * Adds extra validation to the `updated_by` field.
     * @param string $updated_by The incoming value to be validated.
     * @return string The validated value to be set.
     */
    public function validate_updated_by(string $updated_by): string {
        # Auto update this value to the current username and IP
        return "{$this->client->username}@{$this->client->ip_address} (API)";
    }

    /**
     * Applies firewall rule changes.
     */
    public function apply(): void {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Checks if a given name value is an existing TrafficShaperLimiter or TrafficShaperLimiterQueue name.
     * @param string $name The name of the pipe or queue to look for.
     * @returns Model|null Returns the object which matched the provided $name
     * or null if no object could be found.
     */
    private function get_dnpipe_by_name(string $name): ?Model {
        # Query for TrafficShaperLimiter objects with the provided name.
        $limiters = TrafficShaperLimiter::read_all();
        $limiter_q = $limiters->query(name: $name);

        # Allow this value if $dnpipe matches an existing TrafficShaperLimiter's name
        if ($limiter_q->exists()) {
            return $limiter_q->first();
        }

        # Check if any existing TrafficShaper limiters have child queues with a name that matches $dnpipe;
        foreach ($limiters->model_objects as $limiter) {
            $queue_q = TrafficShaperLimiterQueue::query(parent_id: $limiter->id, name: $name);
            if ($queue_q->exists()) {
                return $queue_q->first();
            }
        }

        return null;
    }
}
