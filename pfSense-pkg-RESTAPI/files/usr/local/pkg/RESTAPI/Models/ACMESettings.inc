<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;

/**
 * Defines a Model for interacting with the ACME general settings
 */
class ACMESettings extends Model {
    public BooleanField $enable;
    public BooleanField $writecerts;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->config_path = 'installedpackages/acme';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];
        $this->verbose_name = 'ACME Settings';

        # Define model Fields
        $this->enable = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Enable',
            help_text: 'Enables or disables the ACME renewal job.',
        );
        $this->writecerts = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'Write Certificates',
            help_text: 'Enables or disables the writing of certificates to /conf/acme/ in various formats for use by ' .
                'other scripts or daemons which do not integrate with the pfSense certificate manager.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the ACME general settings.
     */
    public function apply(): void {
        # Run the pfSense-pkg-acme functions to setup the cronjob and write certificates if necessary
        \pfsense_pkg\acme\set_cronjob();
        acme_write_all_certificates();
    }
}
