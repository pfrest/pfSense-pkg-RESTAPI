<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\RegexValidator;
use RESTAPI\Validators\X509Validator;

/**
 * Defines a Model for interacting with ACME certificate configurations.
 */
class ACMECertificate extends Model {
    public StringField $name;
    public StringField $descr;
    public StringField $status;
    public ForeignModelField $acmeaccount;
    public StringField $keylength;
    public StringField $keypaste;
    public StringField $preferredchain;
    public BooleanField $oscpstaple;
    public IntegerField $dnssleep;
    public IntegerField $renewafter;
    public NestedModelField $a_domainlist;
    public NestedModelField $a_actionlist;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->config_path = 'installedpackages/acme/certificates/item';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];
        $this->verbose_name = 'ACME Certificate';

        # Define model Fields
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [new RegexValidator(pattern: '/^[a-zA-Z0-9\.\-_]+$/')],
            help_text: 'The name of the ACME certificate.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            help_text: 'A description of the ACME certificate.',
        );
        $this->status = new StringField(
            default: 'active',
            choices: ['active', 'disabled'],
            help_text: 'The activation status of the ACME certificate.',
        );
        $this->acmeaccount = new ForeignModelField(
            model_name: 'ACMEAccountKey',
            model_field: 'name',
            required: true,
            help_text: 'The ACME account key to use for the ACME certificate.',
        );
        $this->keylength = new StringField(
            default: '2048',
            choices: ['2048', '3072', '4096', 'ec-256', 'ec-384', 'custom'],
            help_text: 'The length of the private key to use for the ACME certificate.',
        );
        $this->keypaste = new StringField(
            required: true,
            conditions: ['keylength' => 'custom'],
            validators: [new X509Validator(allow_crt: false, allow_prv: true, allow_rsa: true, allow_ecprv: true)],
            help_text: 'The custom private key to use for the ACME certificate.',
        );
        $this->preferredchain = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            help_text: 'The preferred certificate chain to use for the ACME certificate.',
        );
        $this->oscpstaple = new BooleanField(
            default: false,
            help_text: 'Whether to enable OCSP Stapling for the ACME certificate.',
        );
        $this->dnssleep = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            maximum: 3600,
            help_text: 'The number of seconds to wait for DNS propagation before requesting verification.',
        );
        $this->renewafter = new IntegerField(
            default: 60,
            minimum: 1,
            maximum: 365,
            help_text: 'The number of days before expiration to renew the ACME certificate.',
        );
        $this->a_domainlist = new NestedModelField(
            model_class: 'ACMECertificateDomain',
            required: true,
            help_text: 'The list of domain verifications  to include in the ACME certificate.',
        );
        $this->a_actionlist = new NestedModelField(
            model_class: 'ACMECertificateAction',
            default: [],
            help_text: 'The list of actions to perform on the ACME certificate after being issued/renewed.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
