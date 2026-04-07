<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for packages that are currently installed on this system.
 */
class Package extends Model {
    public ForeignModelField $name;
    public StringField $shortname;
    public StringField $descr;
    public StringField $installed_version;
    public StringField $latest_version;
    public BooleanField $update_available;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_installed_pkgs';
        $this->many = true;

        # Set model fields
        $this->name = new ForeignModelField(
            model_name: 'AvailablePackage',
            model_field: 'name',
            required: true,
            verbose_name: 'Name',
            help_text: 'The name of the pfSense package.',
        );
        $this->shortname = new StringField(default: '', read_only: true, verbose_name: 'Short Name', help_text: "The package's shortname.");
        $this->descr = new StringField(default: '', read_only: true, verbose_name: 'Description', help_text: "The package's description.");
        $this->installed_version = new StringField(
            default: '',
            read_only: true,
            verbose_name: 'Installed Version',
            help_text: 'The version of the package currently installed.',
        );
        $this->latest_version = new StringField(
            default: '',
            read_only: true,
            verbose_name: 'Latest Version',
            help_text: 'The latest version available for this package.',
        );
        $this->update_available = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Update Available',
            help_text: 'Indicates whether the installed package has an update available.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains all installed packages and their associated attributes. This method is the internal callable that is
     * used to populate this Model's Field object internal values.
     * @return array An array containing the internal values of all available packages
     */
    protected function get_installed_pkgs(): array {
        # Variables
        $pkgs = [];
        $pkgs_info = get_pkg_info(pkgs: 'all', remote_repo_usage_disabled: true, installed_pkgs_only: true);

        # Loop through each of the output lines and format it
        foreach ($pkgs_info as $pkg) {
            # Obtain the latest version available for this package
            $latest_version = AvailablePackage::query(name: $pkg['name'])->first()->version->value;

            # Add our formatted package to our array
            $pkgs[] = [
                'name' => $pkg['name'],
                'shortname' => $pkg['shortname'],
                'descr' => $pkg['desc'],
                'installed_version' => $pkg['installed_version'],
                'latest_version' => $latest_version,
                'update_available' => pkg_version_compare($pkg['installed_version'], $latest_version) == '<',
            ];
        }

        return $pkgs;
    }

    /**
     * Installs the package assigned to the `name` field.
     */
    public function _create(): void {
        # Install the package
        pkg_install($this->name->value);

        # Clear the object cache to ensure we get fresh data
        $this->clear_model_cache();

        # Locate this package's current ID and load info about the package installed
        $this->id = $this->query(name: $this->name->value)->first()->id;
        $this->from_internal();
    }

    /**
     * Deletes the package assigned to the `name` field.
     */
    public function _delete(): void {
        pkg_delete($this->name->value);
    }
}
