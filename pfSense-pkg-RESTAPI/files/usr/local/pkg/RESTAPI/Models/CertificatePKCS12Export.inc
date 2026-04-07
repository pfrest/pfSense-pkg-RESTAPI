<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Model for handling the export of an existing Certificate as a PKCS12 file.
 */
class CertificatePKCS12Export extends Model {
    public ForeignModelField $certref;
    public StringField $encryption;
    public StringField $passphrase;
    public StringField $filename;
    public StringField $binary_data;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set Model attributes
        $this->internal_callable = 'get_internal';
        $this->verbose_name = 'Certificate PKCS#12 Export';

        # Define Model Fields
        $this->certref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: true,
            verbose_name: 'Certificate Reference',
            help_text: 'The Certificate to export as a PKCS12 file.',
        );
        $this->encryption = new StringField(
            default: 'high',
            choices: ['high', 'low', 'legacy'],
            verbose_name: 'Encryption',
            help_text: 'The level of encryption to use when exporting the PKCS#12 archive.',
        );
        $this->passphrase = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Passphrase',
            help_text: 'The passphrase to use when exporting the PKCS#12 archive. Leave empty for no passphrase.',
        );
        $this->filename = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Filename',
            help_text: 'The filename used when exporting the PKCS#12 archive. This value cannot be changed and will ' .
                'always be certificate refid with the .p12 extension.',
        );
        $this->binary_data = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Binary Data',
            help_text: 'The PKCS#12 archive binary data. This value cannot be changed.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Psuedo-method to return the internal data of the Model. This Model has no internal data but must use an internal
     * callable.
     * @return array An empty array.
     */
    public function get_internal(): array {
        return [];
    }

    /**
     * Creates the PKCS#12 archive file based on the Model data.
     */
    public function _create(): void {
        # Set the filename. This will be used by the BinaryContentHandler to set the filename of the download.
        $this->filename->value = $this->certref->value . '.p12';

        # Create the PKCS#12 archive
        $this->binary_data->value = cert_pkcs12_export(
            cert: $this->certref->get_related_model()->to_internal(),
            encryption: $this->encryption->value,
            passphrase: $this->passphrase->value,
            delivery: 'data',
        );

        # Throw an error if the PKCS#12 archive could not be created
        if (!$this->binary_data->value) {
            throw new ServerError(
                message: 'The PKCS#12 archive could not be created for unknown reasons.',
                response_id: 'CERTIFICATE_PKCS12_EXPORT_CREATION_FAILED',
            );
        }
    }
}
