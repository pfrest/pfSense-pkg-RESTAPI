<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\NotAcceptableError;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Model for renewing an existing Certificate.
 */
class CertificateRenew extends Model {
    public ForeignModelField $certref;
    public BooleanField $reusekey;
    public BooleanField $reuseserial;
    public BooleanField $strictsecurity;
    public StringField $oldserial;
    public StringField $newserial;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_internal';
        $this->many = false;

        # Set model fields
        $this->certref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: true,
            verbose_name: 'Certificate Reference',
            help_text: 'The `refid` of the Certificate to renew.',
        );
        $this->reusekey = new BooleanField(
            default: true,
            indicates_true: true,
            verbose_name: 'Reuse Key',
            help_text: 'Reuses the existing private key when renewing the certificate.',
        );
        $this->reuseserial = new BooleanField(
            default: true,
            indicates_true: true,
            verbose_name: 'Reuse Serial',
            help_text: 'Reuses the existing serial number when renewing the certificate.',
        );
        $this->strictsecurity = new BooleanField(
            default: false,
            indicates_true: true,
            verbose_name: 'Strict Security',
            help_text: 'Enforces strict security measures when renewing the certificate.',
        );
        $this->oldserial = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Old Serial',
            help_text: 'The old serial number of the Certificate before the renewal.',
        );
        $this->newserial = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'New Serial',
            help_text: 'The new serial number of the Certificate after the renewal.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Returns the internal data for this model.
     * @return array The internal data for this model.
     */
    public function get_internal(): array {
        # Ensure 'reusekey' and 'reuseserial' are set to true by default
        return ['reusekey' => true, 'reuseserial' => true];
    }

    /**
     * Adds extra validation to he 'certref' field.
     * @param string $certref The incoming value to validate.
     * @return string The validated value to assign.
     */
    public function validate_certref(string $certref): string {
        # Ensure the Certificate is capable of being renewed.
        $cert = lookup_cert($certref);
        if (!is_cert_locally_renewable($cert['item'])) {
            throw new NotAcceptableError(
                message: "Certificate with refid `$certref` is not capable of being renewed.",
                response_id: 'CERTIFICATE_RENEW_UNAVAILABLE',
            );
        }

        return $certref;
    }

    /**
     * Renews the specified Certificate.
     */
    public function _create(): void {
        # Extract details from the Certificate to renew
        $cert_config = &lookup_cert($this->certref->value);
        $this->oldserial->value = cert_get_serial($cert_config['item']['crt']);

        # The pfSense cert_renew() function expects a 'path' key with the config path
        $cert_config['path'] = "cert/{$cert_config['idx']}";

        # Renew the cert using pfSense's built in cert_renew function
        $renewed = cert_renew(
            $cert_config,
            reusekey: $this->reusekey->value,
            strictsecurity: $this->strictsecurity->value,
            reuseserial: $this->reuseserial->value,
        );

        # Throw an error if the renewal failed
        if (!$renewed) {
            throw new ServerError(
                message: 'Failed to renew the Certificate for unknown reason.',
                response_id: 'CERTIFICATE_RENEW_FAILED',
            );
        }

        # Otherwise, continue with the renewal
        $this->newserial->value = cert_get_serial($cert_config['item']['crt']);
        $msg = "Renewed certificate {$cert_config['item']['descr']} ({$cert_config['item']['refid']}) - Serial {$this->oldserial->value} -> {$this->newserial->value}";
        $this->log(level: LOG_ERR, message: $msg, logfile: 'system');
        $this->write_config($msg);
    }

    /**
     * Apply changes to this certificate to backend services
     */
    public function apply(): void {
        # Reconfigure the OS truststore and restart services that use the certificate
        ca_setup_trust_store();
        cert_restart_services(cert_get_all_services($this->certref->value));
    }
}
