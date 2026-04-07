<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UIDField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with remote authentication servers.
 */
class AuthServer extends Model {
    public UIDField $refid;
    public StringField $type;
    public StringField $name;
    public StringField $host;
    public PortField $ldap_port;
    public StringField $ldap_urltype;
    public IntegerField $ldap_protver;
    public IntegerField $ldap_timeout;
    public ForeignModelField $ldap_caref;
    public StringField $ldap_scope;
    public StringField $ldap_basedn;
    public StringField $ldap_authcn;
    public BooleanField $ldap_extended_enabled;
    public StringField $ldap_extended_query;
    public StringField $ldap_binddn;
    public StringField $ldap_bindpw;
    public StringField $ldap_attr_user;
    public StringField $ldap_attr_group;
    public StringField $ldap_attr_member;
    public BooleanField $ldap_rfc2307;
    public BooleanField $ldap_rfc2307_userdn;
    public StringField $ldap_attr_groupobj;
    public StringField $ldap_pam_groupdn;
    public BooleanField $ldap_utf8;
    public BooleanField $ldap_nostrip_at;
    public BooleanField $ldap_allow_unauthenticated;

    public StringField $radius_secret;
    public PortField $radius_auth_port;
    public PortField $radius_acct_port;
    public StringField $radius_protocol;
    public IntegerField $radius_timeout;
    public InterfaceField $radius_nasip_attribute;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'system/authserver';
        $this->verbose_name = 'authentication server';
        $this->many = true;

        # Set model fields
        $this->refid = new UIDField(
            verbose_name: 'Reference ID',
            help_text: 'The unique reference ID for this authentication server. This value is only used internally ' .
                'and cannot be manually set or changed.',
        );
        $this->type = new StringField(
            required: true,
            choices: ['ldap', 'radius'],
            verbose_name: 'Type',
            help_text: 'The type of this authentication server.',
        );
        $this->name = new StringField(
            required: true,
            unique: true,
            editable: false,
            verbose_name: 'Name',
            help_text: 'The descriptive name for this authentication server.',
        );
        $this->host = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_fqdn: true)],
            verbose_name: 'Host',
            help_text: 'The remote IP address or hostname of the authentication server.',
        );
        $this->ldap_port = new PortField(
            required: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Port',
            help_text: 'The LDAP port to connect to on this LDAP authentication server.',
        );
        $this->ldap_urltype = new StringField(
            required: true,
            choices: ['Standard TCP', 'STARTTLS Encrypt', 'SSL/TLS Encrypted'],
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP URL Type',
            help_text: 'The encryption mode to use when connecting to this authentication server. Use `Standard TCP` for ' .
                "unencrypted LDAP connections, use `STARTTLS Encrypt` to start an encrypted connection via STARTTLS if it's " .
                'available, or `SSL/TLS Encrypted` to only use LDAPS encrypted connections.',
        );
        $this->ldap_protver = new IntegerField(
            default: 3,
            choices: [2, 3],
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Protocol Version',
            help_text: 'The LDAP protocol version to use for connections to this LDAP authentication server.',
        );
        $this->ldap_timeout = new IntegerField(
            default: 25,
            minimum: 1,
            maximum: 300,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Timeout',
            help_text: 'The timeout (in seconds) for connections to the LDAP authentication server.',
        );
        $this->ldap_caref = new ForeignModelField(
            model_name: 'CertificateAuthority',
            model_field: 'refid',
            allowed_keywords: ['global'],
            default: 'global',
            conditions: ['ldap_urltype' => ['starttls', 'encrypted']],
            verbose_name: 'LDAP CA Reference',
            help_text: 'The certificate authority used to validate the LDAP server certificate.',
        );
        $this->ldap_scope = new StringField(
            required: true,
            choices: ['one', 'subtree'],
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Scope',
            help_text: 'The LDAP search scope. Use `one` to limit the scope to a single level, or `subtree` to allow ' .
                'the entire subtree to be searched.',
        );
        $this->ldap_basedn = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Base DN',
            help_text: 'The root for LDAP searches on this authentication server.',
        );
        $this->ldap_authcn = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Auth Containers',
            help_text: 'The LDAP authentication container.',
        );
        $this->ldap_extended_enabled = new BooleanField(
            default: false,
            indicates_true: 'yes',
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Extended Query',
            help_text: 'Enable LDAP extended queries.',
        );
        $this->ldap_extended_query = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['type' => 'ldap', 'ldap_extended_enabled' => true],
            verbose_name: 'LDAP Extended Query Filter',
            help_text: 'The extended LDAP query to make during LDAP searches.',
        );
        $this->ldap_binddn = new StringField(
            default: null,
            allow_null: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Bind DN',
            help_text: 'The DN to use when binding to this authentication server. Set to `null` to bind anonymously.',
        );
        $this->ldap_bindpw = new StringField(
            required: true,
            sensitive: true,
            conditions: ['type' => 'ldap', '!ldap_binddn' => null],
            verbose_name: 'LDAP Bind Password',
            help_text: 'The password to use when binding to this authentication server.',
        );
        $this->ldap_attr_user = new StringField(
            default: 'cn',
            allow_empty: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP User Attribute',
            help_text: 'The LDAP user attribute.',
        );
        $this->ldap_attr_group = new StringField(
            default: 'cn',
            allow_empty: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Group Attribute',
            help_text: 'The LDAP group attribute.',
        );
        $this->ldap_attr_member = new StringField(
            default: 'member',
            allow_empty: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Member Attribute',
            help_text: 'The LDAP member attribute.',
        );
        $this->ldap_rfc2307 = new BooleanField(
            default: false,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP RFC 2307',
            help_text: 'Enables or disable RFC2307 LDAP options.',
        );
        $this->ldap_rfc2307_userdn = new BooleanField(
            default: false,
            conditions: ['type' => 'ldap', 'ldap_rfc2307' => true],
            verbose_name: 'LDAP RFC 2307 User DN',
            help_text: 'Enables or disable the use of DNs for username searches.',
        );
        $this->ldap_attr_groupobj = new StringField(
            default: 'posixGroup',
            allow_empty: true,
            conditions: ['type' => 'ldap', 'ldap_rfc2307' => true],
            verbose_name: 'LDAP Group Object Class',
            help_text: 'The group object class for groups in RFC2307 mode.',
        );
        $this->ldap_pam_groupdn = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP PAM Group DN',
            help_text: 'The group DN to use for shell authentication. Users must be a member of this group and ' .
                'have valid posixAccount attributes to sign in.',
        );
        $this->ldap_utf8 = new BooleanField(
            default: false,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP UTF-8',
            help_text: 'Enables or disables UTF-8 encoding LDAP parameters before sending them to this authentication server',
        );
        $this->ldap_nostrip_at = new BooleanField(
            default: false,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP No Strip @',
            help_text: 'Do not strip away parts of the username after the @ symbol.',
        );
        $this->ldap_allow_unauthenticated = new BooleanField(
            default: true,
            conditions: ['type' => 'ldap'],
            verbose_name: 'LDAP Allow Unauthenticated',
            help_text: 'Enables or disables unauthenticated binding. Unauthenticated binds are bind with an existing ' .
                'login but with an empty password. Some LDAP servers (Microsoft AD) allow this type of bind without ' .
                'any possibility to disable it.',
        );
        $this->radius_secret = new StringField(
            required: true,
            sensitive: true,
            conditions: ['type' => 'radius'],
            verbose_name: 'RADIUS Secret',
            help_text: 'The shared secret to use when authenticating to this RADIUS server.',
        );
        $this->radius_protocol = new StringField(
            default: 'MSCHAPv2',
            choices: ['MSCHAPv2', 'MSCHAPv1', 'CHAP_MD5', 'PAP'],
            conditions: ['type' => 'radius'],
            verbose_name: 'RADIUS Protocol',
            help_text: 'The RADIUS protocol to use when authenticating.',
        );
        $this->radius_auth_port = new PortField(
            default: '1812',
            allow_null: true,
            conditions: ['type' => 'radius'],
            verbose_name: 'RADIUS Auth Port',
            help_text: 'The port used by RADIUS for authentication. Set to `null` to disable use of authentication services.',
        );
        $this->radius_acct_port = new PortField(
            default: '1813',
            allow_null: true,
            conditions: ['type' => 'radius'],
            verbose_name: 'RADIUS Accounting Port',
            help_text: 'The port used by RADIUS for accounting. Set to `null` to disable use of accounting services.',
        );
        $this->radius_timeout = new IntegerField(
            default: 5,
            minimum: 1,
            maximum: 300,
            conditions: ['type' => 'radius'],
            verbose_name: 'RADIUS Timeout',
            help_text: 'The timeout (in seconds) for connections to this RADIUS authentication server.',
        );
        $this->radius_nasip_attribute = new InterfaceField(
            required: true,
            conditions: ['type' => 'radius'],
            verbose_name: 'RADIUS NAS IP Attribute',
            help_text: "The interface whose IP will be used as the 'NAS-IP-Address' attribute during RADIUS " .
                'Access-Requests. This choice will not change the interface used for contacting the RADIUS server.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
