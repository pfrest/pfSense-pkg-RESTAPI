<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;

class ACMECertificateAction extends Model {
    public StringField $status;
    public StringField $command;
    public StringField $method;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->parent_model_class = 'ACMECertificate';
        $this->config_path = 'a_actionlist/item';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];
        $this->verbose_name = 'ACME Certificate Action';

        # Define model Fields
        $this->status = new StringField(
            default: 'active',
            choices: ['active', 'disabled'],
            help_text: 'The activation status of the ACME certificate.',
        );
        $this->command = new StringField(required: true, help_text: 'The command to execute on the ACME certificate.');
        $this->method = new StringField(
            required: true,
            choices: ['shellcommand', 'php_command', 'servicerestart', 'xmlrpcservicerestart'],
            help_text: 'The action method that should be used to run the command.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
