<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\DateTimeField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\UIDField;
use RESTAPI\Responses\ForbiddenError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\RegexValidator;
use RESTAPI\Validators\X509Validator;

/**
 * Defines a Model for interacting with the system's certificates.
 */
class Certificate extends Model {
    public StringField $descr;
    public UIDField $refid;
    public StringField $caref;
    public StringField $type;
    public Base64Field $csr;
    public Base64Field $crt;
    public Base64Field $prv;
    public DateTimeField $valid_from;
    public DateTimeField $valid_until;
    public IntegerField $valid_days_left;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'cert';
        $this->many = true;

        # Set model fields
        $this->descr = new StringField(
            required: true,
            validators: [new RegexValidator(pattern: "/[\?\>\<\&\/\\\"\']/", invert: true)],
            verbose_name: 'Description',
            help_text: 'The descriptive name for this certificate.',
        );
        $this->refid = new UIDField(
            verbose_name: 'Reference ID',
            help_text: 'The unique ID assigned to this certificate for internal system use. This value is generated ' .
                'by this system and cannot be changed.',
        );
        $this->caref = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'CA Reference',
            help_text: 'The unique ID of the existing pfSense Certificate Authority that signed this certificate.' .
                'This value is assigned by this system and cannot be changed.',
        );
        $this->type = new StringField(
            default: 'server',
            choices: ['server', 'user'],
            verbose_name: 'Type',
            help_text: 'The certificate type. Use `server` when this certificate is to be used by one or more ' .
                'services on this system. Use `user` when this certificate is intended to be assigned to a user for ' .
                'authentication purposes.',
        );
        $this->csr = new Base64Field(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'CSR Data',
            help_text: 'The X509 certificate signing request string if this certificate is pending an external signature.',
        );
        $this->crt = new Base64Field(
            required: true,
            validators: [new X509Validator(allow_crt: true)],
            verbose_name: 'Certificate Data',
            help_text: 'The X509 certificate string.',
        );
        $this->prv = new Base64Field(
            required: true,
            sensitive: true,
            validators: [new X509Validator(allow_prv: true, allow_rsa: true, allow_ecprv: true)],
            verbose_name: 'Private Key',
            help_text: 'The X509 private key string.',
        );
        $this->valid_from = new DateTimeField(
            allow_null: true,
            read_only: true,
            representation_only: true,
            verbose_name: 'Valid From',
            help_text: 'The start date from which this certificate is valid.',
        );
        $this->valid_until = new DateTimeField(
            allow_null: true,
            read_only: true,
            representation_only: true,
            verbose_name: 'Valid Until',
            help_text: 'The date until which this certificate is valid.',
        );
        $this->valid_days_left = new IntegerField(
            allow_null: true,
            read_only: true,
            representation_only: true,
            minimum: 1,
            maximum: 12000,
            verbose_name: 'Valid Days Left',
            help_text: 'The number of days remaining until this certificate expires.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Gets the start date of this certificate.
     * @returns string|null The start date in 'Y-m-d H:i:s' format.
     */
    protected function from_internal_valid_from(): string|null {
        $from_date = cert_get_dates($this->crt->value, false, false)[0];
        if (!$from_date) {
            return null;
        }

        return $from_date->format('Y-m-d H:i:s');
    }

    /**
     * Gets the expiration date of this certificate.
     * @returns string|null The expiration date in 'Y-m-d H:i:s' format.
     */
    protected function from_internal_valid_until(): string|null {
        $until_date = cert_get_dates($this->crt->value, false, false)[1];
        if (!$until_date) {
            return null;
        }

        return $until_date->format('Y-m-d H:i:s');
    }

    /**
     * Calculates the number of days left until this certificate expires.
     * @returns int The number of entire days left until expiration. -1 if already expired.
     */
    protected function from_internal_valid_days_left(): int|null {
        $enddate = cert_get_dates($this->crt->value, false, false)[1];
        if (!$enddate) {
            return null;
        }
        $interval = (new \DateTime())->diff($enddate);
        return $interval->invert ? 0 - $interval->days : $interval->days;
    }

    /**
     * Adds extra validation to the `prv` field.
     * @param string $prv The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When $prv and the `crt` field's current value are not a matching key pair
     */
    public function validate_prv(string $prv): string {
        # Throw a validation error if $prv and the `crt` field's current value are not a matching key pair
        if (!X509Validator::is_matching_keypair(public_key: $this->crt->value, private_key: $prv)) {
            throw new ValidationError(
                message: 'Field `prv` and field `crt` values must be a matching key pair.',
                response_id: 'CERTIFICATE_CRT_AND_PRV_NOT_KEYPAIR',
            );
        }

        return $prv;
    }

    /**
     * Extends the default _create() method to ensure the certificate is fully imported before creating it.
     */
    public function _create(): void {
        # Import the cert first using pfSense's cert_import function and relink CAs (if necessary)
        $config_data = $this->to_internal();
        cert_import($config_data, $this->crt->value, $this->prv->value);
        $this->caref->value = $config_data['caref'] ?? null;

        # Create the Certificate object
        parent::_create();
    }

    /**
     * Extends the default _update() method to ensure any `csr` value is removed before updating a Certificate.
     */
    public function _update(): void {
        # Remove the `csr` field value before updating the Certificate.
        $this->csr->value = null;
        parent::_update();
    }

    /**
     * Deletes this Certificate object from configuration.
     * @throws ForbiddenError When the Certificate cannot be deleted because it is in use.
     */
    public function _delete(): void {
        # Do not allow this Certificate to be deleted if it is in use
        if (cert_in_use($this->refid->value)) {
            throw new ForbiddenError(
                message: 'Certificate cannot be deleted because it is in use by one or more services.',
                response_id: 'CERTIFICATE_CANNOT_BE_DELETED_WHILE_IN_USE',
            );
        }

        parent::_delete();
    }
}
