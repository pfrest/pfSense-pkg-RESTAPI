<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\URLValidator;

/**
 * Defines a Model that represents the FreeRADIUS EAP module configuration.
 *
 * The pfSense FreeRADIUS package stores a single EAP configuration object that controls how the
 * FreeRADIUS server negotiates and tunnels EAP authentication (default EAP type, EAP-TLS, EAP-TTLS,
 * EAP-PEAP, OCSP, certificate selection, etc.). This Model exposes that configuration as a
 * singleton at /api/v2/services/freeradius/eap.
 */
class FreeRADIUSEAP extends Model {
    # General EAP options
    public BooleanField $disable_weak_eap_types;
    public StringField $default_eap_type;
    public IntegerField $timer_expire;
    public BooleanField $ignore_unknown_eap_types;
    public BooleanField $cisco_accounting_username_bug;
    public IntegerField $max_sessions;
    public StringField $tls_min_version;

    # Certificates for TLS (refids resolved against the pfSense Cert Manager)
    public ForeignModelField $ssl_ca_cert;
    public ForeignModelField $ssl_ca_crl;
    public ForeignModelField $ssl_server_cert;

    # EAP-TLS
    public BooleanField $tls_include_length;
    public IntegerField $tls_fragment_size;
    public BooleanField $tls_check_cert_issuer;
    public StringField $tls_ca_subject;
    public BooleanField $tls_check_cert_cn;

    # EAP-TLS Cache
    public BooleanField $cache_enable;
    public IntegerField $cache_lifetime;
    public IntegerField $cache_max_entries;

    # EAP-TLS OCSP
    public BooleanField $ocsp_enable;
    public BooleanField $ocsp_override_cert_url;
    public StringField $ocsp_url;

    # EAP-TTLS
    public StringField $ttls_default_eap_type;
    public BooleanField $ttls_copy_request_to_tunnel;
    public BooleanField $ttls_use_tunneled_reply;
    public BooleanField $ttls_include_length;

    # EAP-PEAP
    public StringField $peap_default_eap_type;
    public BooleanField $peap_copy_request_to_tunnel;
    public BooleanField $peap_use_tunneled_reply;
    public StringField $peap_soh_enable;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes - the EAP module config is a singleton stored at config index 0
        $this->packages = ['pfSense-pkg-freeradius3'];
        $this->package_includes = ['freeradius.inc'];
        $this->config_path = 'installedpackages/freeradiuseapconf/config/0';
        $this->many = false;
        $this->always_apply = true;
        $this->verbose_name = 'FreeRADIUS EAP';

        # General EAP fields
        $this->disable_weak_eap_types = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'Disable Weak EAP Types',
            internal_name: 'vareapconfdisableweakeaptypes',
            help_text: 'Disables weak EAP types (MD5 and GTC). When enabled, only stronger EAP types like ' .
                'TLS, TTLS, PEAP, and MSCHAPv2 are allowed.',
        );
        $this->default_eap_type = new StringField(
            required: false,
            default: 'md5',
            choices: ['md5', 'gtc', 'leap', 'tls', 'ttls', 'peap', 'mschapv2'],
            verbose_name: 'Default EAP Type',
            internal_name: 'vareapconfdefaulteaptype',
            help_text: 'The default EAP type invoked when an EAP-Identity response is received.',
        );
        $this->timer_expire = new IntegerField(
            required: false,
            default: 60,
            minimum: 1,
            verbose_name: 'EAP-Response/Request Expiration',
            internal_name: 'vareapconftimerexpire',
            help_text: 'The expiration time (in seconds) of the EAP-Response / EAP-Request correlation list.',
        );
        $this->ignore_unknown_eap_types = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Ignore Unknown EAP Types',
            internal_name: 'vareapconfignoreunknowneaptypes',
            help_text: 'If enabled, unknown EAP types are ignored instead of rejected. A proxy module must be ' .
                'configured to forward such requests to another RADIUS server.',
        );
        $this->cisco_accounting_username_bug = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'Cisco Accounting Username Bug Workaround',
            internal_name: 'vareapconfciscoaccountingusernamebug',
            help_text: 'Enables the workaround for the CISCO AP1230B firmware 12.2(13)JA1 accounting username bug.',
        );
        $this->max_sessions = new IntegerField(
            required: false,
            default: 4096,
            minimum: 1,
            verbose_name: 'Maximum Sessions Tracking Per Server',
            internal_name: 'vareapconfmaxsessions',
            help_text: 'Helps prevent DoS attacks by limiting the number of EAP sessions the server tracks.',
        );
        $this->tls_min_version = new StringField(
            required: false,
            default: '1.0',
            choices: ['1.0', '1.1', '1.2'],
            verbose_name: 'Minimum TLS Version',
            internal_name: 'vareapconftlsminversion',
            help_text: 'The minimum TLS version that will be accepted for EAP-TLS-derived methods.',
        );

        # Certificates for TLS
        $this->ssl_ca_cert = new ForeignModelField(
            model_name: 'CertificateAuthority',
            model_field: 'refid',
            required: false,
            default: null,
            allow_empty: true,
            allow_null: true,
            verbose_name: 'SSL CA Certificate',
            internal_name: 'ssl_ca_cert',
            help_text: 'The Certificate Authority (refid) used by the FreeRADIUS EAP module. Leave unset to ' .
                'have the FreeRADIUS package auto-generate a temporary CA on apply.',
        );
        $this->ssl_ca_crl = new ForeignModelField(
            model_name: 'CertificateRevocationList',
            model_field: 'refid',
            required: false,
            default: null,
            allow_empty: true,
            allow_null: true,
            verbose_name: 'SSL CA Revocation List',
            internal_name: 'ssl_ca_crl',
            help_text: 'The Certificate Revocation List (refid) checked when verifying EAP client certificates. ' .
                'Leave unset to disable CRL checking.',
        );
        $this->ssl_server_cert = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: false,
            default: null,
            allow_empty: true,
            allow_null: true,
            verbose_name: 'SSL Server Certificate',
            internal_name: 'ssl_server_cert',
            help_text: 'The server Certificate (refid) presented by the FreeRADIUS EAP module. Leave unset to ' .
                'have the FreeRADIUS package auto-generate a temporary server certificate on apply.',
        );

        # EAP-TLS
        $this->tls_include_length = new BooleanField(
            required: false,
            default: true,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TLS Include Length',
            internal_name: 'vareapconfincludelength',
            help_text: 'When enabled, the total length of the EAP-TLS message is included in every packet, ' .
                'instead of only the first packet of a fragment series.',
        );
        $this->tls_fragment_size = new IntegerField(
            required: false,
            default: 1024,
            minimum: 1,
            maximum: 4096,
            verbose_name: 'EAP-TLS Fragment Size',
            internal_name: 'vareapconffragmentsize',
            help_text: 'The EAP-TLS fragment size (in bytes). Cannot exceed 4096.',
        );
        $this->tls_check_cert_issuer = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'EAP-TLS Check Cert Issuer',
            internal_name: 'vareapconfenablecheckcertissuer',
            help_text: 'When enabled, the server/client certificate must match the configured CA issuer.',
        );
        $this->tls_ca_subject = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            verbose_name: 'EAP-TLS CA Subject',
            internal_name: 'vareapconfcasubject',
            help_text: 'The expected CA subject string for issuer validation. Leave blank to use the subject of ' .
                'the configured SSL CA Certificate.',
        );
        $this->tls_check_cert_cn = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'on',
            indicates_false: '',
            verbose_name: 'EAP-TLS Check Client Certificate CN',
            internal_name: 'vareapconfenablecheckcertcn',
            help_text: 'When enabled, the Common Name of the client certificate must match the User-Name attribute.',
        );

        # EAP-TLS Cache
        $this->cache_enable = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TLS Cache Enable',
            internal_name: 'vareapconfcacheenablecache',
            help_text: 'Enables the EAP-TLS session resumption / fast reauthentication cache.',
        );
        $this->cache_lifetime = new IntegerField(
            required: false,
            default: 24,
            minimum: 1,
            verbose_name: 'EAP-TLS Cache Lifetime',
            internal_name: 'vareapconfcachelifetime',
            help_text: 'Lifetime of cached entries (in hours).',
        );
        $this->cache_max_entries = new IntegerField(
            required: false,
            default: 255,
            minimum: 0,
            verbose_name: 'EAP-TLS Cache Max Entries',
            internal_name: 'vareapconfcachemaxentries',
            help_text: 'The maximum number of entries in the EAP-TLS cache. Set to 0 for infinite.',
        );

        # EAP-TLS OCSP
        $this->ocsp_enable = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TLS OCSP Enable',
            internal_name: 'vareapconfocspenable',
            help_text: 'Enables OCSP support for EAP-TLS client certificate verification.',
        );
        $this->ocsp_override_cert_url = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TLS OCSP Override Cert URL',
            internal_name: 'vareapconfocspoverridecerturl',
            help_text: 'When enabled, the configured OCSP responder URL overrides the URL extracted from the ' .
                'client certificate.',
        );
        $this->ocsp_url = new StringField(
            required: false,
            default: 'http://127.0.0.1/ocsp/',
            allow_empty: true,
            verbose_name: 'EAP-TLS OCSP Responder URL',
            internal_name: 'vareapconfocspurl',
            validators: [new URLValidator()],
            help_text: 'The OCSP responder URL used to verify EAP-TLS client certificates when OCSP is enabled.',
        );

        # EAP-TTLS
        $this->ttls_default_eap_type = new StringField(
            required: false,
            default: 'md5',
            choices: ['md5', 'gtc', 'otp', 'tls', 'mschapv2'],
            verbose_name: 'EAP-TTLS Default EAP Type',
            internal_name: 'vareapconfttlsdefaulteaptype',
            help_text: 'The default EAP type used inside the EAP-TTLS tunnel.',
        );
        $this->ttls_copy_request_to_tunnel = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TTLS Copy Request To Tunnel',
            internal_name: 'vareapconfttlscopyrequesttotunnel',
            help_text: 'When enabled, attributes available outside of the EAP-TTLS tunnel are copied into the ' .
                'tunneled authentication request.',
        );
        $this->ttls_use_tunneled_reply = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TTLS Use Tunneled Reply',
            internal_name: 'vareapconfttlsusetunneledreply',
            help_text: 'When enabled, reply attributes from the tunneled authentication are returned to the NAS.',
        );
        $this->ttls_include_length = new BooleanField(
            required: false,
            default: true,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-TTLS Include Length',
            internal_name: 'vareapconfttlsincludelength',
            help_text: 'When enabled, the total length of the EAP-TTLS message is included in every packet, ' .
                'instead of only the first packet of a fragment series.',
        );

        # EAP-PEAP
        $this->peap_default_eap_type = new StringField(
            required: false,
            default: 'mschapv2',
            choices: ['md5', 'gtc', 'otp', 'tls', 'mschapv2'],
            verbose_name: 'EAP-PEAP Default EAP Type',
            internal_name: 'vareapconfpeapdefaulteaptype',
            help_text: 'The default EAP type used inside the EAP-PEAP tunnel.',
        );
        $this->peap_copy_request_to_tunnel = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-PEAP Copy Request To Tunnel',
            internal_name: 'vareapconfpeapcopyrequesttotunnel',
            help_text: 'When enabled, attributes available outside of the EAP-PEAP tunnel are copied into the ' .
                'tunneled authentication request.',
        );
        $this->peap_use_tunneled_reply = new BooleanField(
            required: false,
            default: false,
            indicates_true: 'yes',
            indicates_false: 'no',
            verbose_name: 'EAP-PEAP Use Tunneled Reply',
            internal_name: 'vareapconfpeapusetunneledreply',
            help_text: 'When enabled, reply attributes from the tunneled authentication are returned to the NAS.',
        );
        $this->peap_soh_enable = new StringField(
            required: false,
            default: 'Disable',
            choices: ['Disable', 'Enable'],
            verbose_name: 'EAP-PEAP Microsoft SoH Support',
            internal_name: 'vareapconfpeapsohenable',
            help_text: 'Enable or disable Microsoft Statement of Health (SoH) support inside EAP-PEAP. The SoH ' .
                'site configuration must be edited at /usr/local/etc/raddb/sites-available/soh outside of this API.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Custom from-internal handler for `tls_include_length` that mirrors the upstream resync
     * behavior — if the value is missing from the pfSense config the rendered eap.conf falls back
     * to `yes`, so the Model's representation should also fall back to `true`.
     */
    public function from_internal_tls_include_length(bool $value): bool {
        return $this->coerce_yes_default($value, 'vareapconfincludelength');
    }

    /**
     * Custom from-internal handler for `ttls_include_length` — see `from_internal_tls_include_length`.
     */
    public function from_internal_ttls_include_length(bool $value): bool {
        return $this->coerce_yes_default($value, 'vareapconfttlsincludelength');
    }

    /**
     * For BooleanFields whose upstream resync defaults a missing key to `yes`, return `true` when
     * the underlying internal value is unset (rather than `false` from the standard BooleanField
     * fallback). When the key exists, honor whatever was actually stored.
     */
    private function coerce_yes_default(bool $value, string $internal_key): bool {
        $config = $this->get_config($this->config_path, []);
        if (!is_array($config) or !array_key_exists($internal_key, $config) or $config[$internal_key] === '') {
            return true;
        }
        return $value;
    }

    /**
     * Performs additional validation on the Model's fields and data by delegating to the
     * upstream pfSense FreeRADIUS validation function.
     */
    public function validate_extra(): void {
        # Run service-level validations against the upstream package's validator
        $input_errors = [];
        $eap = $this->to_internal();
        freeradius_validate_eap($eap, $input_errors);

        # If there were validation errors that were not caught by the model fields, throw a ValidationError.
        if (!empty($input_errors)) {
            throw new ValidationError(
                message: "An unexpected validation error has occurred: $input_errors[0]. Please report this issue at " .
                    'https://github.com/pfrest/pfSense-pkg-RESTAPI/issues/new',
                response_id: 'FREERADIUS_EAP_UNEXPECTED_VALIDATION_ERROR',
            );
        }
    }

    /**
     * Apply the changes made to this Model by re-rendering the FreeRADIUS EAP module configuration.
     */
    public function apply(): void {
        freeradius_eapconf_resync();
    }
}
