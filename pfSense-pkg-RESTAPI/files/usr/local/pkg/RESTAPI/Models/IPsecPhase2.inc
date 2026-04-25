<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\IPsecApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\SpecialNetworkField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UIDField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model that represents IPsec Phase 2 settings.
 */
class IPsecPhase2 extends Model {
    public UIDField $uniqid;
    public IntegerField $reqid;
    public ForeignModelField $ikeid;
    public StringField $descr;
    public BooleanField $disabled;
    public StringField $mode;
    public SpecialNetworkField $localid_type;
    public StringField $localid_address;
    public IntegerField $localid_netbits;
    public SpecialNetworkField $natlocalid_type;
    public StringField $natlocalid_address;
    public IntegerField $natlocalid_netbits;
    public StringField $remoteid_type;
    public StringField $remoteid_address;
    public IntegerField $remoteid_netbits;
    public StringField $protocol;
    public NestedModelField $encryption_algorithm_option;
    public StringField $hash_algorithm_option;
    public IntegerField $pfsgroup;
    public IntegerField $rekey_time;
    public IntegerField $rand_time;
    public IntegerField $lifetime;
    public StringField $pinghost;
    public BooleanField $keepalive;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Obtain global IPsec phase 2 variables
        global $p2_modes;
        global $p2_halgos;
        global $p2_pfskeygroups;

        # Set model attributes
        $this->config_path = 'ipsec/phase2';
        $this->subsystem = 'ipsec';
        $this->many = true;
        $this->verbose_name = 'IPsec Phase 2';

        # Set model fields
        $this->uniqid = new UIDField(
            verbose_name: 'Unique ID',
            help_text: 'A unique ID used to identify this IPsec phase2 entry internally. This value is automatically ' .
                'set by the system and cannot be changed.',
        );
        $this->reqid = new IntegerField(
            unique: true,
            default: ipsec_new_reqid(),
            editable: false,
            read_only: true,
            verbose_name: 'Reqid',
            help_text: 'A unique ID used to identify this IPsec phase2 entry internally. This value is automatically ' .
                'set by the system and cannot be changed.',
        );
        $this->ikeid = new ForeignModelField(
            model_name: 'IPsecPhase1',
            model_field: 'ikeid',
            required: true,
            verbose_name: 'IKE ID',
            help_text: 'The `ikeid` of the parent IPsec phase 1 entry this IPsec phase 2 entry belongs to.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this IPsec phase 2 entry.',
        );
        $this->disabled = new BooleanField(
            default: false,
            verbose_name: 'Disabled',
            help_text: 'Disables this IPsec phase 2 entry.',
        );
        $this->mode = new StringField(
            required: true,
            choices: array_keys($p2_modes),
            verbose_name: 'Mode',
            help_text: 'The IPsec phase 2 mode this entry will use.',
        );
        $this->localid_type = new SpecialNetworkField(
            required: true,
            allow_ipaddr: false,
            allow_subnet: false,
            allow_alias: false,
            allow_interface: true,
            allow_interface_ip: false,
            allow_interface_groups: false,
            allow_self: false,
            allow_l2tp: false,
            allow_pppoe: false,
            allowed_keywords: ['address', 'network'],
            internal_name: 'type',
            internal_namespace: 'localid',
            conditions: ['!mode' => 'transport'],
            verbose_name: 'Local ID Type',
            help_text: 'The local ID type to use for this phase 2 entry.',
        );
        $this->localid_address = new StringField(
            required: true,
            internal_name: 'address',
            internal_namespace: 'localid',
            conditions: ['localid_type' => $this->localid_type->allowed_keywords],
            validators: [new IPAddressValidator()],
            verbose_name: 'Local ID Address',
            help_text: 'The local network IP component of this IPsec security association.',
        );
        $this->localid_netbits = new IntegerField(
            required: true,
            minimum: 0,
            maximum: 128,
            internal_name: 'netbits',
            internal_namespace: 'localid',
            conditions: ['localid_type' => 'network'],
            verbose_name: 'Local ID Network Bits',
            help_text: 'The subnet bits of the `localid_address` network.',
        );
        $this->natlocalid_type = new SpecialNetworkField(
            default: null,
            allow_ipaddr: false,
            allow_subnet: false,
            allow_alias: false,
            allow_interface: true,
            allow_interface_ip: false,
            allow_interface_groups: false,
            allow_self: false,
            allow_l2tp: false,
            allow_pppoe: false,
            allowed_keywords: ['address', 'network'],
            allow_null: true,
            internal_name: 'type',
            internal_namespace: 'natlocalid',
            conditions: ['!mode' => ['transport', 'vti']],
            verbose_name: 'NAT Local ID Type',
            help_text: 'The NAT/BINAT translation type for this IPsec phase 2 entry. Leave as `null` if NAT/BINAT is ' .
                'not needed.',
        );
        $this->natlocalid_address = new StringField(
            required: true,
            internal_name: 'address',
            internal_namespace: 'natlocalid',
            conditions: ['natlocalid_type' => $this->natlocalid_type->allowed_keywords],
            validators: [new IPAddressValidator()],
            verbose_name: 'NAT Local ID Address',
            help_text: 'The NAT/BINAT local network IP component of this IPsec security association.',
        );
        $this->natlocalid_netbits = new IntegerField(
            required: true,
            minimum: 0,
            maximum: 128,
            internal_name: 'netbits',
            internal_namespace: 'natlocalid',
            conditions: ['natlocalid_type' => 'network'],
            verbose_name: 'NAT Local ID Network Bits',
            help_text: 'The subnet bits of the `natlocalid_address` network.',
        );
        $this->remoteid_type = new SpecialNetworkField(
            required: true,
            allow_ipaddr: false,
            allow_subnet: false,
            allow_alias: false,
            allow_interface: false,
            allow_interface_ip: false,
            allow_interface_groups: false,
            allow_self: false,
            allow_l2tp: false,
            allow_pppoe: false,
            allowed_keywords: ['address', 'network'],
            internal_name: 'type',
            internal_namespace: 'remoteid',
            conditions: ['!mode' => 'transport'],
            verbose_name: 'Remote ID Type',
            help_text: 'The remote ID type to use for this phase 2 entry.',
        );
        $this->remoteid_address = new StringField(
            required: true,
            internal_name: 'address',
            internal_namespace: 'remoteid',
            conditions: ['remoteid_type' => $this->remoteid_type->allowed_keywords],
            validators: [new IPAddressValidator()],
            verbose_name: 'Remote ID Address',
            help_text: 'The remote network IP component of this IPsec security association.',
        );
        $this->remoteid_netbits = new IntegerField(
            required: true,
            minimum: 0,
            maximum: 128,
            internal_name: 'netbits',
            internal_namespace: 'remoteid',
            conditions: ['remoteid_type' => 'network'],
            verbose_name: 'Remote ID Network Bits',
            help_text: 'The subnet bits of the `remoteid_address` network.',
        );
        $this->protocol = new StringField(
            default: 'esp',
            choices: ['esp', 'ah'],
            verbose_name: 'Protocol',
            help_text: 'the IPsec phase 2 proposal protocol for this entry. Encapsulating Security Payload (`esp`) ' .
                'performs encryption and authentication, Authentication Header (`ah`) is authentication only.',
        );
        $this->encryption_algorithm_option = new NestedModelField(
            model_class: 'IPsecPhase2Encryption',
            required: true,
            conditions: ['protocol' => 'esp'],
            verbose_name: 'Encryption Algorithm',
            help_text: 'The encryption algorithms to be used by this phase 2 entry.',
        );
        $this->hash_algorithm_option = new StringField(
            required: true,
            choices: array_keys($p2_halgos),
            many: true,
            many_minimum: 1,
            delimiter: null,
            internal_name: 'hash-algorithm-option',
            verbose_name: 'Hash Algorithm',
            help_text: 'The hashing algorithms used by this IPsec phase 2 entry. Note: Hash is ignored with GCM ' .
                'algorithms. SHA1 provides weak security and should be avoided.',
        );
        $this->pfsgroup = new IntegerField(
            default: 14,
            choices: array_keys($p2_pfskeygroups),
            verbose_name: 'PFS Group',
            help_text: 'The PFS key group this IPsec phase 2 entry should use. Note: Groups 1, 2, 5, 22, 23, and 24 ' .
                'provide weak security and should be avoided.',
        );
        $this->lifetime = new IntegerField(
            default: 3600,
            verbose_name: 'Lifetime',
            help_text: 'The hard IKE SA lifetime (in seconds) after which the IKE SA will be expired.',
        );
        $this->rekey_time = new IntegerField(
            default: 3240,
            verbose_name: 'Rekey Time',
            help_text: 'The amount of time (in seconds) before an IKE SA establishes new keys.',
        );
        $this->rand_time = new IntegerField(
            default: 360,
            verbose_name: 'Random Time',
            help_text: 'A random value up to this amount will be subtracted from the `rekey_time` and `reauth_time` ' .
                'to avoid simultaneous renegotiation.',
        );
        $this->pinghost = new StringField(
            default: '',
            allow_empty: true,
            validators: [new IPAddressValidator(allow_fqdn: true)],
            verbose_name: 'Ping Host',
            help_text: 'The IP address to send an ICMP echo request to inside the tunnel. Can trigger initiation ' .
                'of a tunnel mode P2, but does not trigger initiation of a VTI mode P2.',
        );
        $this->keepalive = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'Keepalive',
            help_text: 'Enables or disables checking this P2 and initiating if disconnected; does not send traffic ' .
                "inside the tunnel. This check ignores the P1 option 'Child SA Start Action' and works for both VTI " .
                'and tunnel mode P2s. For IKEv2 without split connections, this only needs to be enabled on one P2.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `localid_type` field.
     * @param string $localid_type The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When $localid_type is an existing interface but the `mode` field's value is set to `vti`
     */
    public function validate_localid_type(string $localid_type): string {
        # Do not allow this field to use interface values if `mode` is set to `vti`
        if (!in_array($localid_type, $this->localid_type->allowed_keywords) and $this->mode->value === 'vti') {
            throw new ValidationError(
                message: 'Field `localid_type` cannot be assigned an interface value while `mode` is set to `vti`.',
                response_id: 'IPSEC_PHASE_2_LOCALID_TYPE_CANNOT_BE_INTERFACE_WITH_VTI_MODE',
            );
        }

        return $localid_type;
    }

    /**
     * Adds extra validation to the `localid_address` field.
     * @param string $localid_address The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When `mode` is set to `tunnel` but $localid_address is not an IPv4 address
     * @throws ValidationError When `mode` is set to `tunnel6` but $localid_address is not an IPv6 address
     */
    public function validate_localid_address(string $localid_address): string {
        # Require this value to be an IPv4 address if `mode` is set to `tunnel`
        if (!$this->localid_address->has_label('is_ipaddrv4') and $this->mode->value === 'tunnel') {
            throw new ValidationError(
                message: 'Field `localid_address` must be an IPv4 address while `mode` is set to `tunnel`.',
                response_id: 'IPSEC_PHASE_2_LOCALID_ADDRESS_MUST_BE_IPV4_WITH_TUNNEL_MODE',
            );
        }
        # Require this value to be an IPv6 address if `mode` is set to `tunnel6`
        if (!$this->localid_address->has_label('is_ipaddrv6') and $this->mode->value === 'tunnel6') {
            throw new ValidationError(
                message: 'Field `localid_address` must be an IPv6 address while `mode` is set to `tunnel6`.',
                response_id: 'IPSEC_PHASE_2_LOCALID_ADDRESS_MUST_BE_IPV6_WITH_TUNNEL6_MODE',
            );
        }

        return $localid_address;
    }

    /**
     * Adds extra validation to the `localid_netbits` field.
     * @param int $localid_netbits The incoming value to be validated.
     * @returns int The validated value to be assigned.
     * @throws ValidationError When `localid_address` is an IPv4 address but the $localid_netbits is greater than 32
     */
    public function validate_localid_netbits(int $localid_netbits): int {
        # Do not allow this value to exceed 32 if it is an IPv4 address
        if ($localid_netbits > 32 and $this->localid_address->has_label('is_ipaddrv4')) {
            throw new ValidationError(
                message: 'Field `localid_netbits` cannot exceed `32` while `localid_address` is an IPv4 address.',
                response_id: 'IPSEC_PHASE_2_LOCALID_NETBITS_EXCEEDS_IPV4_MAXIMUM',
            );
        }

        return $localid_netbits;
    }

    /**
     * Adds extra validation to the `natlocalid_type` field.
     * @param string $natlocalid_type The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When $natlocalid_type is an existing interface but the `mode` field's value is set to `vti`
     */
    public function validate_natlocalid_type(string $natlocalid_type): string {
        # Variables
        $localid_type = $this->localid_type->value;

        # Require this value to match the `localid_type` fields value if it is `network` or `address`
        if (
            $natlocalid_type === 'address' and $localid_type === 'network' or
            $natlocalid_type === 'network' and $localid_type === 'address'
        ) {
            throw new ValidationError(
                message: 'Field `natlocalid_type` value must match the `localid_type` value if either value is ' .
                    'not an interface.',
                response_id: 'IPSEC_PHASE_2_NATLOCALID_TYPE_MUST_MATCH_LOCALID_TYPE',
            );
        }

        return $natlocalid_type;
    }

    /**
     * Adds extra validation to the `natlocalid_address` field.
     * @param string $natlocalid_address The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When `mode` is set to `tunnel` but $natlocalid_address is not an IPv4 address
     * @throws ValidationError When `mode` is set to `tunnel6` but $natlocalid_address is not an IPv6 address
     */
    public function validate_natlocalid_address(string $natlocalid_address): string {
        # Require this value to be an IPv4 address if `mode` is set to `tunnel`
        if (!$this->natlocalid_address->has_label('is_ipaddrv4') and $this->mode->value === 'tunnel') {
            throw new ValidationError(
                message: 'Field `natlocalid_address` must be an IPv4 address while `mode` is set to `tunnel`.',
                response_id: 'IPSEC_PHASE_2_NATLOCALID_ADDRESS_MUST_BE_IPV4_WITH_TUNNEL_MODE',
            );
        }
        # Require this value to be an IPv6 address if `mode` is set to `tunnel6`
        if (!$this->natlocalid_address->has_label('is_ipaddrv6') and $this->mode->value === 'tunnel6') {
            throw new ValidationError(
                message: 'Field `natlocalid_address` must be an IPv6 address while `mode` is set to `tunnel6`.',
                response_id: 'IPSEC_PHASE_2_NATLOCALID_ADDRESS_MUST_BE_IPV6_WITH_TUNNEL6_MODE',
            );
        }

        return $natlocalid_address;
    }

    /**
     * Adds extra validation to the `natlocalid_netbits` field.
     * @param int $natlocalid_netbits The incoming value to be validated.
     * @returns int The validated value to be assigned.
     * @throws ValidationError When `natlocalid_address` is an IPv4 address but the $natlocalid_netbits is greater than 32
     */
    public function validate_natlocalid_netbits(int $natlocalid_netbits): int {
        # Do not allow this value to exceed 32 if it is an IPv4 address
        if ($natlocalid_netbits > 32 and $this->natlocalid_address->has_label('is_ipaddrv4')) {
            throw new ValidationError(
                message: 'Field `natlocalid_netbits` cannot exceed `32` while `natlocalid_address` is an IPv4 address.',
                response_id: 'IPSEC_PHASE_2_NATLOCALID_NETBITS_EXCEEDS_IPV4_MAXIMUM',
            );
        }

        return $natlocalid_netbits;
    }

    /**
     * Adds extra validation to the `remoteid_address` field.
     * @param string $remoteid_address The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When `mode` is set to `tunnel` but $remoteid_address is not an IPv4 address
     * @throws ValidationError When `mode` is set to `tunnel6` but $remoteid_address is not an IPv6 address
     */
    public function validate_remoteid_address(string $remoteid_address): string {
        # Require this value to be an IPv4 address if `mode` is set to `tunnel`
        if (!$this->remoteid_address->has_label('is_ipaddrv4') and $this->mode->value === 'tunnel') {
            throw new ValidationError(
                message: 'Field `remoteid_address` must be an IPv4 address while `mode` is set to `tunnel`.',
                response_id: 'IPSEC_PHASE_2_REMOTEID_ADDRESS_MUST_BE_IPV4_WITH_TUNNEL_MODE',
            );
        }
        # Require this value to be an IPv6 address if `mode` is set to `tunnel6`
        if (!$this->remoteid_address->has_label('is_ipaddrv6') and $this->mode->value === 'tunnel6') {
            throw new ValidationError(
                message: 'Field `remoteid_address` must be an IPv6 address while `mode` is set to `tunnel6`.',
                response_id: 'IPSEC_PHASE_2_REMOTEID_ADDRESS_MUST_BE_IPV6_WITH_TUNNEL6_MODE',
            );
        }

        return $remoteid_address;
    }

    /**
     * Adds extra validation to the `remoteid_netbits` field.
     * @param int $remoteid_netbits The incoming value to be validated.
     * @returns int The validated value to be assigned.
     * @throws ValidationError When `remoteid_address` is an IPv4 address but the $remoteid_netbits is greater than 32
     */
    public function validate_remoteid_netbits(int $remoteid_netbits): int {
        # Do not allow this value to exceed 32 if it is an IPv4 address
        if ($remoteid_netbits > 32 and $this->remoteid_address->has_label('is_ipaddrv4')) {
            throw new ValidationError(
                message: 'Field `remoteid_netbits` cannot exceed `32` while `remoteid_address` is an IPv4 address.',
                response_id: 'IPSEC_PHASE_2_REMOTEID_NETBITS_EXCEEDS_IPV4_MAXIMUM',
            );
        }

        return $remoteid_netbits;
    }

    /**
     * Adds extra validation to the `rekey_time` field.
     * @param int $rekey_time The incoming value to be validated.
     * @returns int The validated value to be set.
     * @throws ValidationError When $rekey_time is greater than or equal to the `lifetime` Field's value.
     */
    public function validate_rekey_time(int $rekey_time): int {
        # Throw a validation error if $rekey_time is greater than or equal to the `lifetime` field's value
        if ($rekey_time >= $this->lifetime->value) {
            throw new ValidationError(
                message: "Field `rekey_time` must be less than the `lifetime` field's current value.",
                response_id: 'IPSEC_PHASE_2_REKEY_TIME_GREATER_THAN_LIFETIME',
            );
        }

        return $rekey_time;
    }

    /**
     * Applies pending changes to IPsec.
     */
    public function apply(): void {
        (new IPsecApplyDispatcher(async: $this->async))->spawn_process();
    }
}
