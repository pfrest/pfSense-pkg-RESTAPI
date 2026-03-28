<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\NotAcceptableError;

/**
 * Defines a Model for interacting with certificates revoked by a certificate revocation list.
 */
class CertificateRevocationListRevokedCertificate extends Model {
    public ForeignModelField $certref;
    public StringField $serial;
    public IntegerField $reason;
    public UnixTimeField $revoke_time;
    public StringField $caref;
    public StringField $descr;
    public StringField $type;
    public Base64Field $crt;
    public Base64Field $prv;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'CertificateRevocationList';
        $this->config_path = 'cert';
        $this->many = true;
        $this->always_apply = true;

        # Set model fields
        $this->certref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: true,
            editable: false,
            internal_name: 'refid',
            conditions: ['serial' => null],
            help_text: 'The reference ID of the certificate to be revoked',
        );
        $this->serial = new StringField(
            default: null,
            allow_null: true,
            help_text: 'The serial number of the certificate to be revoked.',
        );
        $this->reason = new IntegerField(
            default: 0,
            choices: [-1, 0, 1, 2, 3, 4, 5, 6, 9],
            minimum: -1,
            help_text: 'The CRL reason for revocation code.',
        );
        $this->revoke_time = new UnixTimeField(
            required: true,
            auto_add_now: true,
            help_text: 'The unix timestamp of when the certificate was revoked.',
        );
        $this->descr = new StringField(
            default: null,
            allow_null: true,
            write_only: true,
            help_text: 'The unique name/description for this CRL.',
        );
        $this->caref = new StringField(
            default: null,
            allow_null: true,
            write_only: true,
            help_text: 'The unique ID of the CA that signed the revoked certificate.',
        );
        $this->type = new StringField(
            default: null,
            allow_null: true,
            write_only: true,
            help_text: 'The type of the certificate to be revoked.',
        );
        $this->crt = new Base64Field(
            default: null,
            allow_null: true,
            write_only: true,
            help_text: 'The X509 certificate string.',
        );
        $this->prv = new Base64Field(
            default: null,
            allow_null: true,
            write_only: true,
            help_text: 'The X509 private key string.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation for this entire Model
     * @throws ConflictError if both a serial number and a certificate reference are set
     */
    public function validate_extra(): void {
        # Do not allow the parent CRL to be used if it's not an internal CRL
        if ($this->parent_model->method->value !== 'internal') {
            throw new NotAcceptableError(
                message: "Parent CRL with '$this->parent_id' can't revoke certificates because it's an external CRL.",
                response_id: 'CERTIFICATE_REVOCATION_LIST_REVOKED_CERTIFICATE_PARENT_NOT_INTERNAL',
            );
        }

        # If a `certref` is set, add attributes from the referenced certificate
        if ($this->certref->value) {
            $cert = $this->certref->get_related_model();
            $this->descr->value = $cert->descr->value;
            $this->serial->value = $cert->serial->value;
            $this->caref->value = $cert->caref->value;
            $this->type->value = $cert->type->value;
            $this->crt->value = $cert->crt->value;
            $this->prv->value = $cert->prv->value;
        }
    }

    /**
     * Adds extra processing when loading internal objects for this model. This is primarily used to catch any value
     * that is not using the expected reason codes and convert them to a -1 (unspecified) reason code.
     */
    public function from_internal_object(mixed $internal_object): void {
        # If the reason code is not a number, convert it to -1 (unspecified)
        if (!is_numeric($internal_object['reason'])) {
            $internal_object['reason'] = -1;
        }

        parent::from_internal_object($internal_object);
    }

    /**
     * Applies the newly created CRL by reloading the OpenVPN and IPsec services.
     */
    public function apply(): void {
        # Update the parent CRL. This ensures the CRL's X509 data is updated to reflect the changes.
        $this->parent_model->client = $this->client;
        $this->parent_model->from_internal();
        $this->parent_model->update();

        # Reload the OpenVPN and IPsec services
        openvpn_refresh_crls();
        ipsec_configure();
    }
}
