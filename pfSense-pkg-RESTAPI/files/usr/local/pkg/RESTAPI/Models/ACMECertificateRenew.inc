<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\ACMECertificateRenewDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;

class ACMECertificateRenew extends Model {
    public ForeignModelField $certificate;
    public StringField $status;
    public UnixTimeField $last_updated;
    public StringField $result_log;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->internal_callable = 'get_renewals';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];
        $this->verbose_name = 'ACME Certificate Renewal';

        # Define model Fields
        $this->certificate = new ForeignModelField(
            model_name: 'ACMECertificate',
            model_field: 'name',
            required: true,
            verbose_name: 'Certificate',
            help_text: 'The name of the ACME certificate to be renewed.',
        );
        $this->result_log = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Result Log',
            help_text: 'The output result of the acme.sh renew command. Note: This log is only populated when the ' .
                'renew process is initiated via REST API, not when it is initiated via the pfSense webConfigurator ' .
                'or auto-renewals.',
        );
        $this->last_updated = new UnixTimeField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Last Updated',
            help_text: 'The unix timestamp of when the ACME certificate renew status last changed. Note: This ' .
                'timestamp is only updated when the renew process is initiated via REST API, not when it is ' .
                'initiated via the pfSense webConfigurator or auto-renewals.',
        );
        $this->status = new StringField(
            default: 'pending',
            read_only: true,
            verbose_name: 'Status',
            help_text: "The status of the ACME certificate renew process. This will show 'pending' if the renew " .
                "process is still running or 'completed' if the renew process has finished. This status only " .
                'indicates whether the renew process has completed, not whether it was successful. You will need' .
                'to refer to the result log for that information. Note: This log is only populated when the ' .
                'renew process is initiated via REST API, not when it is initiated via the pfSense webConfigurator ' .
                'or auto-renewals.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the renewal status for all current ACME certificates. pfSense does not store or save these results
     * in a sufficient manner to be able to reference them in an asynchronous manner. Instead, the REST API
     * keeps track of these results separately in a temporary file.
     */
    public function get_renewals(): array {
        # Variables
        $renewals = [];

        # Loop through each ACME certificate and check it's last renewal details
        foreach (ACMECertificate::read_all()->model_objects as $cert) {
            # Loop vars
            $renewal_json = [];

            # Decode the results from the last renewal JSON file
            if (file_exists("/tmp/.RESTAPI.acme.{$cert->name->value}.renewal_result.json")) {
                $renewal_json = json_decode(
                    file_get_contents("/tmp/.RESTAPI.acme.{$cert->name->value}.renewal_result.json"),
                    associative: true,
                );
            }

            $renewals[$cert->id] = [];
            $renewals[$cert->id]['certificate'] = $cert->name->value;
            $renewals[$cert->id]['result_log'] = $renewal_json['result_log'] ?? null;
            $renewals[$cert->id]['last_updated'] = $renewal_json['last_updated'] ?? null;
            $renewals[$cert->id]['status'] = $renewal_json['status'] ?? null;
        }

        return $renewals;
    }

    /**
     * Overwrites the default _create() method but only assigns an ID. This model does not create any writable data.
     */
    public function _create(): void {
        $this->id = $this->certificate->get_related_model()->id;
    }

    /**
     * Initiates the ACME verification/renew process for the certificate.
     */
    public function apply(): void {
        # Remove the previous renewal result file
        unlink("/tmp/.RESTAPI.acme.{$this->certificate->value}.renewal_result.json");

        # Start the ACME certificate renew process
        $dispatcher = new ACMECertificateRenewDispatcher(async: $this->async);
        $dispatcher->spawn_process(certificate: $this->certificate->value);

        # Update the model data with the new renewal details
        $this->from_internal();
    }
}
