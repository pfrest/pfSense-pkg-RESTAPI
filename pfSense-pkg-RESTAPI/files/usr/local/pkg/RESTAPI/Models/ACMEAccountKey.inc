<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\EmailAddressValidator;
use RESTAPI\Validators\X509Validator;

/**
 * Defines a Model for interacting with ACME account keys.
 */
class ACMEAccountKey extends Model {
    public const RSA_KEY_BITS = 2048;

    public StringField $name;
    public StringField $descr;
    public StringField $email;
    public StringField $acmeserver;
    public StringField $url;
    public Base64Field $accountkey;
    public BooleanField $registered;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->config_path = 'installedpackages/acme/accountkeys/item';
        $this->verbose_name = 'ACME Account Key';
        $this->verbose_name_plural = 'ACME Account Keys';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];

        # Define model Fields
        $this->name = new StringField(required: true, unique: true, verbose_name: 'Name', help_text: 'The name of the ACME account key.');
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description of the ACME account key.',
        );
        $this->email = new StringField(
            default: '',
            allow_empty: true,
            validators: [new EmailAddressValidator()],
            verbose_name: 'Email',
            help_text: 'The email address associated with the ACME account key.',
        );
        $this->acmeserver = new StringField(
            required: true,
            choices_callable: 'get_acmeserver_choices',
            verbose_name: 'ACME Server',
            help_text: 'The ACME server this account key will belong to.',
        );
        $this->accountkey = new Base64Field(
            default_callable: 'generate_rsa_key',
            sensitive: true,
            validators: [new X509Validator(allow_crt: false, allow_prv: true, allow_rsa: true, allow_ecprv: true)],
            verbose_name: 'Account Key',
            help_text: 'The RSA private key for the ACME account key.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the choices for the 'acmeserver' field. These choices are dynamically populated by the ACME package.
     * @return array An array of choices for the 'acmeserver' field.
     */
    public function get_acmeserver_choices(): array {
        global $a_acmeserver;
        return array_keys($a_acmeserver);
    }

    /**
     * Obtains the URL of the ACME server based on the 'acmeserver' field.
     * @return string|null The modified internal value for the 'url' field.
     */
    public function get_acme_server_url(): ?string {
        global $a_acmeserver;
        return $a_acmeserver[$this->acmeserver->value]['url'];
    }

    /**
     * Generates a new RSA private key as the 'accountkey' field's default value.
     */
    public static function generate_rsa_key(int $key_bits = self::RSA_KEY_BITS): string {
        # Define the rsa key configuration
        $config = [
            'private_key_bits' => $key_bits,
            'private_key_type' => OPENSSL_KEYTYPE_RSA,
        ];

        # Generate a new rsa key
        $private_key_resource = openssl_pkey_new($config);

        # Extract the rsa key from the object into a string
        openssl_pkey_export($private_key_resource, $private_key);

        return $private_key;
    }
}
