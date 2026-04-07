<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\NotAcceptableError;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Model for renewing an existing Certificate Authority.
 */
class CertificateAuthorityRenew extends Model {
    public ForeignModelField $caref;
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
        $this->caref = new ForeignModelField(
            model_name: 'CertificateAuthority',
            model_field: 'refid',
            required: true,
            verbose_name: 'CA Reference',
            help_text: 'The Certificate Authority to renew.',
        );
        $this->reusekey = new BooleanField(
            default: true,
            indicates_true: true,
            verbose_name: 'Reuse Key',
            help_text: 'Reuses the existing private key when renewing the certificate authority.',
        );
        $this->reuseserial = new BooleanField(
            default: true,
            indicates_true: true,
            verbose_name: 'Reuse Serial',
            help_text: 'Reuses the existing serial number when renewing the certificate authority.',
        );
        $this->strictsecurity = new BooleanField(
            default: false,
            indicates_true: true,
            verbose_name: 'Strict Security',
            help_text: 'Enforces strict security measures when renewing the certificate authority.',
        );
        $this->oldserial = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Old Serial',
            help_text: 'The old serial number of the Certificate Authority before the renewal.',
        );
        $this->newserial = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'New Serial',
            help_text: 'The new serial number of the Certificate Authority after the renewal.',
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
     * Adds extra validation to he 'caref' field.
     * @param string $caref The incoming value to validate.
     * @return string The validated value to assign.
     */
    public function validate_caref(string $caref): string {
        # Ensure the Certificate Authority is capable of being renewed.
        if (!is_cert_locally_renewable($this->caref->get_related_model()->to_internal())) {
            throw new NotAcceptableError(
                message: "Certificate Authority with refid `$caref` is not capable of being renewed.",
                response_id: 'CERTIFICATE_AUTHORITY_RENEW_UNAVAILABLE',
            );
        }

        return $caref;
    }

    /**
     * Renews the specified Certificate Authority.
     */
    public function _create(): void {
        # Extract details from the Certificate Authority to renew
        $ca_config = &lookup_ca($this->caref->value);
        $this->oldserial->value = cert_get_serial($ca_config['item']['crt']);
        $this->id = $ca_config['idx'];

        # The pfSense cert_renew() function expects a 'path' key with the config path
        $ca_config['path'] = "ca/{$ca_config['idx']}";

        # Renew the cert using pfSense's built in cert_renew function
        $renewed = cert_renew(
            $ca_config,
            reusekey: $this->reusekey->value,
            strictsecurity: $this->strictsecurity->value,
            reuseserial: $this->reuseserial->value,
        );

        # Throw an error if the renewal failed
        if (!$renewed) {
            throw new ServerError(
                message: 'Failed to renew the Certificate Authority for unknown reason.',
                response_id: 'CERTIFICATE_AUTHORITY_RENEW_FAILED',
            );
        }

        # Otherwise, continue with the renewal
        $this->newserial->value = cert_get_serial($ca_config['item']['crt']);
        $msg = "Renewed CA {$ca_config['item']['descr']} ({$ca_config['item']['refid']}) - Serial {$this->oldserial->value} -> {$this->newserial->value}";
        $this->log(level: LOG_ERR, message: $msg, logfile: 'system');
        $this->write_config($msg);
    }

    /**
     * Apply changes to this CA to backend services
     */
    public function apply(): void {
        # Reconfigure the OS truststore and restart services that use the CA
        ca_setup_trust_store();
        cert_restart_services(ca_get_all_services($this->caref->value));
    }
}
