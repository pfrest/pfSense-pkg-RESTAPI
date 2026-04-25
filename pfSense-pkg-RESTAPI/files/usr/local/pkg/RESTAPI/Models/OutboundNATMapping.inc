<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FilterAddressField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\SpecialNetworkField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\HexValidator;

/**
 * Defines a Model that represents outbound NAT mappings.
 */
class OutboundNATMapping extends Model {
    const SOURCE_HASH_KEY_BYTES = 16;
    public InterfaceField $interface;
    public StringField $protocol;
    public BooleanField $disabled;
    public BooleanField $nonat;
    public BooleanField $nosync;
    public FilterAddressField $source;
    public PortField $source_port;
    public FilterAddressField $destination;
    public PortField $destination_port;
    public SpecialNetworkField $target;
    public IntegerField $target_subnet;
    public PortField $nat_port;
    public BooleanField $static_nat_port;
    public StringField $poolopts;
    public StringField $source_hash_key;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'nat/outbound/rule';
        $this->many = true;
        $this->subsystem = 'natconf';

        # Set model properties
        $this->interface = new InterfaceField(
            required: true,
            allow_interface_groups: true,
            verbose_name: 'Interface',
            help_text: 'The interface on which traffic is matched as it exits the firewall. In most cases this is a ' .
                'WAN-type or another externally-connected interface.',
        );
        $this->protocol = new StringField(
            default: null,
            choices: ['tcp', 'udp', 'tcp/udp', 'icmp', 'esp', 'ah', 'gre', 'ipv6', 'igmp', 'pim', 'ospf'],
            allow_null: true,
            verbose_name: 'Protocol',
            help_text: 'The protocol this rule should match. Use `null` for any protocol.',
        );
        $this->disabled = new BooleanField(
            default: false,
            verbose_name: 'Disabled',
            help_text: 'Disable this outbound NAT rule.',
        );
        $this->nonat = new BooleanField(
            default: false,
            verbose_name: 'No NAT',
            help_text: 'Do not NAT traffic matching this rule.',
        );
        $this->nosync = new BooleanField(
            default: false,
            verbose_name: 'No Sync',
            help_text: 'Do not sync this rule to HA peers.',
        );
        $this->source = new FilterAddressField(
            required: true,
            allow_ipaddr: false,
            allow_subnet: true,
            allow_alias: true,
            allow_interface: true,
            allow_interface_ip: false,
            allow_interface_groups: true,
            allow_any: true,
            allow_invert: false,
            allow_self: true,
            allow_l2tp: false,
            internal_value_key: 'network',
            verbose_name: 'Source',
            help_text: 'The source network this rule should match.',
        );
        $this->source_port = new PortField(
            default: null,
            allow_alias: true,
            allow_range: true,
            allow_null: true,
            internal_name: 'sourceport',
            verbose_name: 'Source Port',
            help_text: 'The source port this rule should match.',
        );
        $this->destination = new FilterAddressField(
            required: true,
            allow_ipaddr: false,
            allow_subnet: true,
            allow_alias: true,
            allow_interface: true,
            allow_interface_ip: false,
            allow_interface_groups: true,
            allow_any: true,
            allow_invert: true,
            allow_self: false,
            allow_l2tp: false,
            internal_value_key: 'network',
            verbose_name: 'Destination',
            help_text: 'The destination network this rule should match.',
        );
        $this->destination_port = new PortField(
            default: null,
            allow_alias: true,
            allow_range: true,
            allow_null: true,
            internal_name: 'dstport',
            verbose_name: 'Destination Port',
            help_text: 'The destination port this rule should match.',
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
            conditions: ['nonat' => false],
            verbose_name: 'Target',
            help_text: 'The target network traffic matching this rule should be translated to.',
        );
        $this->target_subnet = new IntegerField(
            default: 128,
            minimum: 1,
            maximum: 128,
            conditions: ['nonat' => false],
            verbose_name: 'Target Subnet',
            help_text: 'The subnet bits for the assigned `target`. This field is only applicable if `target` is set ' .
                'to an IP address. This has no affect for alias or interface `targets`.',
        );
        $this->static_nat_port = new BooleanField(
            default: false,
            internal_name: 'staticnatport',
            conditions: ['nonat' => false],
            verbose_name: 'Static NAT Port',
            help_text: 'Do not rewrite source port for traffic matching this rule.',
        );
        $this->nat_port = new PortField(
            default: '',
            allow_alias: false,
            allow_range: true,
            internal_range_char: ':',
            allow_empty: true,
            internal_name: 'natport',
            conditions: ['static_nat_port' => false, 'nonat' => false],
            verbose_name: 'NAT Port',
            help_text: 'The external source port or port range used for rewriting the original source port on ' .
                'connections matching the rule.',
        );
        $this->poolopts = new StringField(
            default: null,
            choices: [
                'round-robin',
                'round-robin sticky-address',
                'random',
                'random sticky-address',
                'source-hash',
                'bitmask',
            ],
            allow_null: true,
            conditions: ['nonat' => false],
            verbose_name: 'Pool Options',
            help_text: 'The pool option used to load balance external IP mapping when `target` is set to a subnet ' .
                'or alias of many addresses. Set to `null` to revert to the system default.',
        );
        $this->source_hash_key = new StringField(
            default: '0x' . bin2hex(openssl_random_pseudo_bytes(length: self::SOURCE_HASH_KEY_BYTES)),
            conditions: ['poolopts' => 'source-hash', 'nonat' => false],
            validators: [new HexValidator(bytes: self::SOURCE_HASH_KEY_BYTES, allow_hex_prefix: true)],
            verbose_name: 'Source Hash Key',
            help_text: 'The key that is fed to the hashing algorithm in hex format. This must be a 16 byte (32 ' .
                'character) hex string prefixed with `0x`. If a value is not provided, one will automatically be ' .
                'generated',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for the outbound NAT mapping.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `poolopts` field.
     * @param $poolopts string The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When `poolopts` is set but `target` is not an IP address or alias.
     */
    public function validate_poolopts(string $poolopts): string {
        # Do not allow poolopts to be set if `target` is not an alias or IP address
        if (!$this->target->has_label('is_ipaddr') and !$this->target->has_label('is_alias')) {
            throw new ValidationError(
                message: 'Field `poolopts` cannot be set if `target` is not an IP address or alias.',
                response_id: 'OUTBOUND_NAT_MAPPING_POOLOPTS_SET_WITHOUT_MULTI_HOST_TARGET',
            );
        }

        return $poolopts;
    }

    /**
     * Adds extra validation to the `target_subnet` field.
     * @param int $target_subnet The incoming value to be validated.
     * @returns int|null The validated value to be set. Returns null if `target` is not an IP address or alias.
     * @throws ValidationError When `target` is an IPv4 address but $target_subnet is greater than 32
     */
    public function validate_target_subnet(int $target_subnet): int|null {
        # Throw an error if `target` is an IPv4 address, but `target_subnet` is greater than 32.
        if ($this->target->has_label('is_ipaddrv4') and $target_subnet > 32) {
            throw new ValidationError(
                message: 'Field `target_subnet` must be 32 or less when `target` is an IPv4 address.',
                response_id: 'OUTBOUND_NAT_MAPPING_TARGET_SUBNET_TOO_LARGE_FOR_IPV4',
            );
        }

        # Unset this value if `target` is not an IP address or alias
        if (!$this->target->has_label('is_alias') and !$this->target->has_label('is_ipaddr')) {
            return null;
        }

        return $target_subnet;
    }

    /**
     * Applies any changes to outbound NAT mappings.
     */
    public function apply() {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
