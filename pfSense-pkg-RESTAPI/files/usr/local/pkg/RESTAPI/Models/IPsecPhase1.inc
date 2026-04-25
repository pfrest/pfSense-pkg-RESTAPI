<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\IPsecApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with IPsec Phase 1 entries.
 */
class IPsecPhase1 extends Model {
    const INADDR_ANY = ['0.0.0.0', '::'];

    public IntegerField $ikeid;
    public StringField $descr;
    public BooleanField $disabled;
    public StringField $iketype;
    public StringField $mode;
    public StringField $protocol;
    public InterfaceField $interface;
    public StringField $remote_gateway;
    public StringField $authentication_method;
    public StringField $myid_type;
    public StringField $myid_data;
    public StringField $peerid_type;
    public StringField $peerid_data;
    public StringField $pre_shared_key;
    public ForeignModelField $certref;
    public ForeignModelField $caref;
    public IntegerField $rekey_time;
    public IntegerField $reauth_time;
    public IntegerField $rand_time;
    public IntegerField $lifetime;
    public StringField $startaction;
    public StringField $closeaction;
    public StringField $nat_traversal;
    public BooleanField $gw_duplicates;
    public BooleanField $mobike;
    public BooleanField $splitconn;
    public BooleanField $prfselect_enable;
    public PortField $ikeport;
    public PortField $nattport;
    public IntegerField $dpd_delay;
    public IntegerField $dpd_maxfail;
    public NestedModelField $encryption;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->config_path = 'ipsec/phase1';
        $this->many = true;
        $this->subsystem = 'ipsec';
        $this->verbose_name = 'IPsec Phase 1';

        # Define model fields
        $this->ikeid = new IntegerField(
            unique: true,
            default: ipsec_ikeid_next(),
            read_only: true,
            verbose_name: 'IKE ID',
            help_text: 'The unique IKE ID for this phase 1 entry. This value is dynamically set and cannot be set or ' .
                'changed by users.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this IPsec phase 1 entry.',
        );
        $this->disabled = new BooleanField(
            default: false,
            verbose_name: 'Disabled',
            help_text: 'Disables this IPsec phase 1 entry.',
        );
        $this->iketype = new StringField(
            required: true,
            choices: ['ikev1', 'ikev2', 'auto'],
            verbose_name: 'IKE Type',
            help_text: 'The IKE protocol version this phase 1 entry will use.',
        );
        $this->mode = new StringField(
            required: true,
            choices: ['main', 'aggressive'],
            conditions: ['iketype' => ['ikev1', 'auto']],
            verbose_name: 'Mode',
            help_text: 'The IKEv1 negotiation mode this phase 1 entry will use.',
        );
        $this->protocol = new StringField(
            required: true,
            choices: ['inet', 'inet6', 'both'],
            verbose_name: 'Protocol',
            help_text: 'The IP version this phase 1 entry will use.',
        );
        $this->interface = new InterfaceField(
            required: true,
            allow_carp_interface: true,
            verbose_name: 'Interface',
            help_text: 'The interface for the local endpoint of this phase 1 entry. This should be an interface ' .
                'that is reachable by the remote peer.',
        );
        $this->remote_gateway = new StringField(
            required: true,
            internal_name: 'remote-gateway',
            validators: [new IPAddressValidator(allow_ipv4: true, allow_ipv6: true, allow_fqdn: true)],
            verbose_name: 'Remote Gateway',
            help_text: 'The IP address or hostname of the remote gateway.',
        );
        $this->authentication_method = new StringField(
            required: true,
            choices: ['pre_shared_key', 'cert'],
            verbose_name: 'Authentication Method',
            help_text: 'The IPsec authentication method this tunnel will use.',
        );
        $this->myid_type = new StringField(
            required: true,
            choices: ['myaddress', 'address', 'fqdn', 'user_fqdn', 'asn1dn', 'keyid tag', 'dyn_dns', 'auto'],
            verbose_name: 'My ID Type',
            help_text: 'The identifier type used by the local end of the tunnel.',
        );
        $this->myid_data = new StringField(
            required: true,
            conditions: ['!myid_type' => 'myaddress'],
            verbose_name: 'My ID Data',
            help_text: 'The identifier value used by the local end of the tunnel. This must be a value that ' .
                'corresponds with the current `myid_type` value.',
        );
        $this->peerid_type = new StringField(
            required: true,
            choices: ['any', 'peeraddress', 'address', 'fqdn', 'user_fqdn', 'asn1dn', 'keyid tag', 'dyn_dns', 'auto'],
            verbose_name: 'Peer ID Type',
            help_text: 'The identifier type used by the remote end of the tunnel.',
        );
        $this->peerid_data = new StringField(
            required: true,
            conditions: ['!peerid_type' => ['any', 'peeraddress']],
            verbose_name: 'Peer ID Data',
            help_text: 'The identifier value used by the remote end of the tunnel. This must be a value that ' .
                'corresponds with the current `peerid_type` value.',
        );
        $this->pre_shared_key = new StringField(
            required: true,
            sensitive: true,
            internal_name: 'pre-shared-key',
            conditions: ['authentication_method' => 'pre_shared_key'],
            verbose_name: 'Pre Shared Key',
            help_text: 'The Pre-Shared Key (PSK) value. This key must match on both peers and should be long and ' .
                'random to protect the tunnel and its contents. A weak Pre-Shared Key can lead to a tunnel compromise.',
        );
        $this->certref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: true,
            conditions: ['authentication_method' => 'cert'],
            verbose_name: 'Certificate Reference',
            help_text: 'The certificate which identifies this system. The certificate must have at least one ' .
                'non-wildcard SAN.',
        );
        $this->caref = new ForeignModelField(
            model_name: 'CertificateAuthority',
            model_field: 'refid',
            required: true,
            conditions: ['authentication_method' => 'cert'],
            verbose_name: 'CA Reference',
            help_text: 'The certificate authority to use when validating the peer certificate.',
        );
        $this->lifetime = new IntegerField(
            default: 28800,
            verbose_name: 'Lifetime',
            help_text: 'The hard child SA lifetime (in seconds) after which the child SA will be expired.',
        );
        $this->rekey_time = new IntegerField(
            default: 25920,
            verbose_name: 'Rekey Time',
            help_text: 'The amount of time (in seconds) before an child SA establishes new keys.',
        );
        $this->reauth_time = new IntegerField(
            default: 0,
            verbose_name: 'Re-Auth Time',
            help_text: 'The amount of time (in seconds) before an child SA is torn down and recreated from scratch, ' .
                'including authentication.',
        );
        $this->rand_time = new IntegerField(
            default: 2880,
            verbose_name: 'Random Time',
            help_text: 'A random value up to this amount will be subtracted from the `rekey_time` ' .
                'to avoid simultaneous renegotiation.',
        );
        $this->startaction = new StringField(
            default: '',
            choices: ['', 'none', 'start', 'trap'],
            allow_empty: true,
            verbose_name: 'Start Action',
            help_text: 'The option used to force specific initiation/responder behavior for child SA (P2) entries.',
        );
        $this->closeaction = new StringField(
            default: '',
            choices: ['', 'none', 'start', 'trap'],
            allow_empty: true,
            verbose_name: 'Close Action',
            help_text: 'The option used to control the behavior when the remote peer unexpectedly closes a child SA (P2)',
        );
        $this->nat_traversal = new StringField(
            default: 'on',
            choices: ['on', 'force'],
            verbose_name: 'NAT Traversal',
            help_text: 'The option used to enable the use of NAT-T (i.e. the encapsulation of ESP in UDP packets) ' .
                'if needed, which can help with clients that are behind restrictive firewalls.',
        );
        $this->mobike = new BooleanField(
            default: false,
            indicates_true: 'on',
            indicates_false: 'off',
            verbose_name: 'MOBIKE',
            help_text: 'Enables or disables the use of MOBIKE for this tunnel.',
        );
        $this->gw_duplicates = new BooleanField(
            default: false,
            verbose_name: 'Gateway Duplicates',
            help_text: 'Enables or disables the allowance of multiple phase 1 configurations with the same remote ' .
                'gateway endpoint.',
        );
        $this->splitconn = new BooleanField(
            default: false,
            verbose_name: 'Split Connection',
            help_text: 'Enables or disables the use split connection entries with multiple phase 2 configurations. ' .
                'Required for remote endpoints that support only a single traffic selector per child SA.',
        );
        $this->prfselect_enable = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'PRF Select Enable',
            help_text: 'Enables or disables manual Pseudo-Random Function (PRF) selection.',
        );
        $this->ikeport = new PortField(
            default: '500',
            verbose_name: 'IKE Port',
            help_text: 'The UDP port for IKE on the remote gateway.',
        );
        $this->nattport = new PortField(
            default: '4500',
            verbose_name: 'NAT-T Port',
            help_text: 'The UDP port for NAT-T on the remote gateway.',
        );
        $this->dpd_delay = new IntegerField(
            default: 10,
            allow_null: true,
            verbose_name: 'DPD Delay',
            help_text: 'The delay (in seconds) between sending peer acknowledgement messages.',
        );
        $this->dpd_maxfail = new IntegerField(
            default: 5,
            allow_null: true,
            verbose_name: 'DPD Max Failures',
            help_text: 'The number of consecutive failures allowed before disconnecting.',
        );
        $this->encryption = new NestedModelField(
            model_class: 'IPsecPhase1Encryption',
            required: true,
            verbose_name: 'Encryption',
            help_text: 'The encryption algorithms supported by this P1 encryption.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation to the `remote_gateway` field.
     * @param string $remote_gateway The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When the `protocol` field is set to `inet`, but $remote_gateway is an IPv6 address.
     * @throws ValidationError When the `protocol` field is set to `inet6`, but $remote_gateway is an IPv4 address.
     */
    public function validate_remote_gateway(string $remote_gateway): string {
        # Throw a validation error if `protocol` is `inet` but $remote_gateway is an IPv6
        if ($this->protocol->value === 'inet' and $this->remote_gateway->has_label('is_ipaddrv6')) {
            throw new ValidationError(
                message: 'Field `remote_gateway` cannot be an IPv6 address while `protocol` is set to `inet`.',
                response_id: 'IPSEC_PHASE_1_IPV6_REMOTE_GATEWAY_NOT_ALLOWED',
            );
        }

        # Throw a validation error if `protocol` is `inet6` but $remote_gateway is an IPv4
        if ($this->protocol->value === 'inet6' and $this->remote_gateway->has_label('is_ipaddrv4')) {
            throw new ValidationError(
                message: 'Field `remote_gateway` cannot be an IPv4 address while `protocol` is set to `inet6`.',
                response_id: 'IPSEC_PHASE_1_IPV4_REMOTE_GATEWAY_NOT_ALLOWED',
            );
        }

        # Query other non-disabled phase 1 entries with the same remote gateway
        $dup_gws = $this->query(id__except: $this->id, remote_gateway: $remote_gateway, disabled: false);

        # Filter out INADDR_ANY addresses
        $dup_gws = $dup_gws->query(remote_gateway__except: '0.0.0.0');
        $dup_gws = $dup_gws->query(remote_gateway__except: '::');

        # Loop through each identified duplicate gateway and check if the duplicate is allowed
        foreach ($dup_gws->model_objects as $dup_gw) {
            # Throw a validation error if either phase 1 object does not allow duplicate gateways
            if (!$this->gw_duplicates->value or !$dup_gw->gw_duplicates->value) {
                throw new ValidationError(
                    message: "Field `remote_gateway` must be unique. Duplicate value found in $this->verbose_name with " .
                        "ID `$dup_gw->id`.",
                    response_id: 'IPSEC_PHASE_1_ILLEGAL_DUPLICATE_REMOTE_GATEWAY',
                );
            }
        }

        return $remote_gateway;
    }

    /**
     * Adds custom validation to the `myid_data` field.
     * @param string $myid_data The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When the `myid_type` field is set to `address`, but $myid_data is not an IP address.
     * @throws ValidationError When the `myid_type` field is set to `fqdn`, but $myid_data is not an FQDN.
     * @throws ValidationError When the `myid_type` field is set to `user_fqdn`, but $myid_data is not an email address.
     * @throws ValidationError When the `myid_type` field is set to `dyn_dns`, but $myid_data is not an FQDN.
     */
    public function validate_myid_data(string $myid_data): string {
        # Throw a validation error if `myid_type` is set to `address` but $myid_data is not an IP address
        if ($this->myid_type->value === 'address' and !is_ipaddr($myid_data)) {
            throw new ValidationError(
                message: 'Field `myid_data` must be an IP address when `myid_type` is set to `address`.',
                response_id: 'IPSEC_PHASE_1_MYID_DATA_MUST_BE_IP_ADDRESS',
            );
        }

        # Throw a validation error if `myid_type` is set to `fqdn` but $myid_data is not an FQDN
        if ($this->myid_type->value === 'fqdn' and !is_fqdn($myid_data)) {
            throw new ValidationError(
                message: 'Field `myid_data` must be an FQDN when `myid_type` is set to `fqdn`.',
                response_id: 'IPSEC_PHASE_1_MYID_DATA_MUST_BE_FQDN',
            );
        }

        # Throw a validation error if `myid_type` is set to `user_fqdn` but $myid_data is not an email address.
        if ($this->myid_type->value === 'user_fqdn' and !filter_var($myid_data, FILTER_VALIDATE_EMAIL)) {
            throw new ValidationError(
                message: 'Field `myid_data` must be an email address when `myid_type` is set to `user_fqdn`.',
                response_id: 'IPSEC_PHASE_1_MYID_DATA_MUST_BE_EMAIL_ADDRESS',
            );
        }

        # Throw a validation error if `myid_type` is set to `dyn_dns` but $myid_data is not an FQDN
        if ($this->myid_type->value === 'dyn_dns' and !is_fqdn($myid_data)) {
            throw new ValidationError(
                message: 'Field `myid_data` must be an FQDN when `myid_type` is set to `dyn_dns`.',
                response_id: 'IPSEC_PHASE_1_MYID_DATA_MUST_BE_FQDN',
            );
        }

        return $myid_data;
    }

    /**
     * Adds custom validation to the `peerid_type` field.
     * @param string $peerid_type The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When $peerid_type is `peeraddress` but the `remote_gateway` field is an any address.
     */
    public function validate_peerid_type(string $peerid_type): string {
        # Throw a validation error if $peerid_type is `peeraddress` but the `remote_gateway` field is an any address.
        if ($peerid_type === 'peeraddress' and in_array($this->remote_gateway->value, self::INADDR_ANY)) {
            throw new ValidationError(
                message: 'Field `peerid_type` cannot be `peeraddress` when `remote_gateway` is `0.0.0.0` or `::`',
                response_id: 'IPSEC_PHASE_1_NO_INADDR_ANY_WITH_PEERADDRESS',
            );
        }

        return $peerid_type;
    }

    /**
     * Adds custom validation to the `peerid_data` field.
     * @param string $peerid_data The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When the `peerid_type` field is set to `address`, but $peerid_data is not an IP address.
     * @throws ValidationError When the `peerid_type` field is set to `fqdn`, but $peerid_data is not an FQDN.
     * @throws ValidationError When the `peerid_type` field is set to `user_fqdn`, but $peerid_data is not an email address.
     * @throws ValidationError When the `peerid_type` field is set to `dyn_dns`, but $peerid_data is not an FQDN.
     */
    public function validate_peerid_data(string $peerid_data): string {
        # Throw a validation error if `peerid_type` is set to `address` but $peerid_data is not an IP address
        if ($this->peerid_type->value === 'address' and !is_ipaddr($peerid_data)) {
            throw new ValidationError(
                message: 'Field `peerid_data` must be an IP address when `peerid_type` is set to `address`.',
                response_id: 'IPSEC_PHASE_1_PEERID_DATA_MUST_BE_IP_ADDRESS',
            );
        }

        # Throw a validation error if `peerid_type` is set to `fqdn` but $peerid_data is not an FQDN
        if ($this->peerid_type->value === 'fqdn' and !is_fqdn($peerid_data)) {
            throw new ValidationError(
                message: 'Field `peerid_data` must be an FQDN when `peerid_type` is set to `fqdn`.',
                response_id: 'IPSEC_PHASE_1_PEERID_DATA_MUST_BE_FQDN',
            );
        }

        # Throw a validation error if `peerid_type` is set to `user_fqdn` but $peerid_data is not an email address.
        if ($this->peerid_type->value === 'user_fqdn' and !filter_var($peerid_data, FILTER_VALIDATE_EMAIL)) {
            throw new ValidationError(
                message: 'Field `peerid_data` must be an email address when `peerid_type` is set to `user_fqdn`.',
                response_id: 'IPSEC_PHASE_1_PEERID_DATA_MUST_BE_EMAIL_ADDRESS',
            );
        }

        # Throw a validation error if `peerid_type` is set to `dyn_dns` but $peerid_data is not an FQDN
        if ($this->peerid_type->value === 'dyn_dns' and !is_fqdn($peerid_data)) {
            throw new ValidationError(
                message: 'Field `peerid_data` must be an FQDN when `peerid_type` is set to `dyn_dns`.',
                response_id: 'IPSEC_PHASE_1_PEERID_DATA_MUST_BE_FQDN',
            );
        }

        return $peerid_data;
    }

    /**
     * Adds custom validation to the `rekey_time` field.
     * @param int $rekey_time The incoming value to be validated.
     * @returns int The validated value to be set.
     * @throws ValidationError When $rekey_time is greater than or equal to the `lifetime` Field's value.
     */
    public function validate_rekey_time(int $rekey_time): int {
        # Throw a validation error if $rekey_time is greater than or equal to the `lifetime` field's value
        if ($rekey_time >= $this->lifetime->value) {
            throw new ValidationError(
                message: "Field `rekey_time` must be less than the `lifetime` field's current value.",
                response_id: 'IPSEC_PHASE_1_REKEY_TIME_GREATER_THAN_LIFETIME',
            );
        }

        return $rekey_time;
    }

    /**
     * Adds custom validation to the `reauth_time` field.
     * @param int $reauth_time The incoming value to be validated.
     * @returns int The validated value to be set.
     * @throws ValidationError When $reauth_time is greater than or equal to the `lifetime` Field's value.
     */
    public function validate_reauth_time(int $reauth_time): int {
        # Throw a validation error if $reauth_time is greater than or equal to the `lifetime` field's value
        if ($reauth_time >= $this->lifetime->value) {
            throw new ValidationError(
                message: "Field `reauth_time` must be less than the `lifetime` field's current value.",
                response_id: 'IPSEC_PHASE_1_REAUTH_TIME_GREATER_THAN_LIFETIME',
            );
        }

        return $reauth_time;
    }

    /**
     * Adds custom validation to the `startaction` field.
     * @param string $startaction The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When $startaction is not `none` but the `remote_gateway` field is 0.0.0.0 or ::.
     */
    public function validate_startaction(string $startaction): string {
        # Throw a validation error if $startaction is not 'none' but the `remote_gateway` field is set to 0.0.0.0 or ::
        if ($startaction !== 'none' and in_array($this->remote_gateway->value, self::INADDR_ANY)) {
            throw new ValidationError(
                message: 'Field `startaction` must be `none` when `remote_gateway` is `0.0.0.0` or `::`.',
                response_id: 'IPSEC_PHASE_1_NO_STARTACTION_WITH_INADDR_ANY_REMOTE_GATEWAY',
            );
        }

        return $startaction;
    }

    /**
     * Adds custom validation to the `nattport` field.
     * @param string $nattport The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When $nattport is the same as the `ikeport` field's value.
     */
    public function validate_nattport(string $nattport): string {
        # Throw a valdiation error if $nattport is the same as the `ikeport` field's value.
        if ($nattport === $this->ikeport->value) {
            throw new ValidationError(
                message: 'Field `nattport` must be different than the `ikeport` value.',
                response_id: 'IPSEC_PHASE_1_NATTPORT_CANNOT_MATCH_IKEPORT',
            );
        }

        return $nattport;
    }

    /**
     * Applies pending changes to IPsec.
     */
    public function apply(): void {
        (new IPsecApplyDispatcher(async: $this->async))->spawn_process();
    }
}
