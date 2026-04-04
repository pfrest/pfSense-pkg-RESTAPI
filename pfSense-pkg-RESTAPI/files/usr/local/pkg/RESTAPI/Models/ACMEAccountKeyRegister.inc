<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\ACMEAccountKeyRegisterDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model used to register a given ACME account key with the ACME package.
 */
class ACMEAccountKeyRegister extends Model {
    public ForeignModelField $name;
    public StringField $status;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->internal_callable = 'get_registrations';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];
        $this->verbose_name = 'ACME Account Key Registration';

        # Define model Fields
        $this->name = new ForeignModelField(
            model_name: 'ACMEAccountKey',
            model_field: 'name',
            required: true,
            help_text: 'The name of the ACME account key to register.',
        );
        $this->status = new StringField(
            default: 'unknown',
            read_only: true,
            help_text: "The registration status of the ACME account key. This will show 'pending' if the registration " .
                "process is still running, 'registered' if the registration was successful, 'failed' if the registration " .
                "failed, and 'unknown' if the registration status is not known. Note: This status can only be " .
                'determined for registrations initiated through the REST API.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the registration status and log of each existing ACME account key.
     */
    protected function get_registrations(): array {
        # Variables
        $registrations = [];

        # Loop through all ACME account keys
        foreach (ACMEAccountKey::read_all()->model_objects as $account_key) {
            $reg_dir = "/tmp/acme/{$account_key->name->value}";
            $reg_file = "$reg_dir/.registered";
            $reg_log = "$reg_dir/acme_issuecert.log";
            $registrations[$account_key->id] = [];
            $registrations[$account_key->id]['name'] = $account_key->name->value;
            $registrations[$account_key->id]['status'] = file_exists($reg_file)
                ? file_get_contents($reg_file)
                : 'unknown';
            $registrations[$account_key->id]['log'] = file_exists($reg_log) ? file_get_contents($reg_log) : '';
        }

        return $registrations;
    }

    /**
     * Overrides the default _create method. This simply assigns the object the correct ID.
     */
    protected function _create(): void {
        $this->id = $this->name->get_related_model()->id;
    }

    /**
     * Registers the ACME account key with the upstream ACME server
     */
    public function apply(): void {
        # Mark the account key status as pending
        $this->status->value = 'pending';

        # Spawn a new process to register the ACME account key
        (new ACMEAccountKeyRegisterDispatcher(async: $this->async))->spawn_process(id: $this->id);

        # Reload the model data in case the registration status has changed and is not unknown
        $post_apply_status = $this->get_registrations()[$this->id]['status'];
        if ($post_apply_status !== 'unknown') {
            $this->status->value = $post_apply_status;
        }
    }
}
