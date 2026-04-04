<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\ACMECertificateIssueDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;

class ACMECertificateIssue extends Model {
    public ForeignModelField $certificate;
    public StringField $status;
    public UnixTimeField $last_updated;
    public StringField $result_log;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->internal_callable = 'get_issuances';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];
        $this->verbose_name = 'ACME Certificate Issuance';

        # Define model Fields
        $this->certificate = new ForeignModelField(
            model_name: 'ACMECertificate',
            model_field: 'name',
            required: true,
            help_text: 'The name of the ACME certificate to be issued.',
        );
        $this->result_log = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            help_text: 'The output result of the acme.sh issue command.',
        );
        $this->last_updated = new UnixTimeField(
            default: null,
            allow_null: true,
            read_only: true,
            help_text: 'The unix timestamp of when the ACME certificate issue status last changed.',
        );
        $this->status = new StringField(
            default: 'pending',
            read_only: true,
            help_text: "The status of the ACME certificate issue process. This will show 'pending' if the issue " .
                "process is still running or 'completed' if the issue process has finished. This status only " .
                'indicates whether the issue process has completed, not whether it was successful. You will need' .
                'to refer to the result log for that information.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the issuance status for all current ACME certificates. pfSense does not store or save these results
     * in a sufficient manner to be able to reference them in an asynchronous manner. Instead, the REST API
     * keeps track of these results separately in a temporary file.
     */
    public function get_issuances(): array {
        # Variables
        $issuances = [];

        # Loop through each ACME certificate and check it's last issuance details
        foreach (ACMECertificate::read_all()->model_objects as $cert) {
            # Loop vars
            $issuance_json = [];

            # Decode the results from the last issuance JSON file
            if (file_exists("/tmp/.RESTAPI.acme.{$cert->name->value}.issuance_result.json")) {
                $issuance_json = json_decode(
                    file_get_contents("/tmp/.RESTAPI.acme.{$cert->name->value}.issuance_result.json"),
                    associative: true,
                );
            }

            $issuances[$cert->id] = [];
            $issuances[$cert->id]['certificate'] = $cert->name->value;
            $issuances[$cert->id]['result_log'] = $issuance_json['result_log'] ?? null;
            $issuances[$cert->id]['last_updated'] = $issuance_json['last_updated'] ?? null;
            $issuances[$cert->id]['status'] = $issuance_json['status'] ?? null;
        }

        return $issuances;
    }

    /**
     * Overwrites the default _create() method but only assigns an ID. This model does not create any writable data.
     */
    public function _create(): void {
        $this->id = $this->certificate->get_related_model()->id;
    }

    /**
     * Initiates the ACME verification/issue process for the certificate.
     */
    public function apply(): void {
        # Remove the previous issuance result file
        unlink("/tmp/.RESTAPI.acme.{$this->certificate->value}.issuance_result.json");

        # Start the ACME certificate issue process
        $dispatcher = new ACMECertificateIssueDispatcher(async: $this->async);
        $dispatcher->spawn_process(certificate: $this->certificate->value);

        # Update the model data with the new issuance details
        $this->from_internal();
    }
}
