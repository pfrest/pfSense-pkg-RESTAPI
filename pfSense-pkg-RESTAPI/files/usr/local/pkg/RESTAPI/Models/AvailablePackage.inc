<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for packages that are available to install on this system.
 */
class AvailablePackage extends Model {
    public StringField $name;
    public StringField $shortname;
    public StringField $descr;
    public StringField $version;
    public BooleanField $installed;
    public StringField $deps;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_all_pkgs';
        $this->many = true;
        $this->cache_class = 'AvailablePackageCache';

        # Set model fields
        $this->name = new StringField(required: true, unique: true, verbose_name: 'Name', help_text: 'The name of the pfSense package.');
        $this->shortname = new StringField(default: '', read_only: true, verbose_name: 'Short Name', help_text: "The package's shortname.");
        $this->descr = new StringField(default: '', read_only: true, verbose_name: 'Description', help_text: "The package's description.");
        $this->version = new StringField(
            default: '',
            read_only: true,
            verbose_name: 'Version',
            help_text: 'The latest version available for this package.',
        );
        $this->installed = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Installed',
            help_text: 'Indicates whether the package is currently installed or not.',
        );
        $this->deps = new StringField(
            default: [],
            read_only: true,
            many: true,
            delimiter: null,
            verbose_name: 'Deps',
            help_text: 'Dependencies for this package that are also installed when this package is installed.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains all available packages and their associated attributes. This method is the internal callable that is
     * used to populate this Model's Field object internal values.
     * @return array An array containing the internal values of all available packages
     * @note To avoid flooding the package servers with requests, this method will automatically cache package
     * information obtained from the remote repos hourly.
     */
    protected function get_all_pkgs(): array {
        # Variables
        $pkgs = [];

        # Loop through each available package in the cache and it to our return value
        foreach ($this->cache->read() as $pkg) {
            # Add our formatted package to our array
            $pkg_data = [
                'name' => $pkg['name'],
                'shortname' => $pkg['shortname'],
                'version' => $pkg['version'],
                'descr' => $pkg['desc'],
                'installed' => is_package_installed($pkg['shortname']),
                'deps' => [],
            ];

            # If this package has dependencies, include them
            if ($pkg['deps']) {
                foreach ($pkg['deps'] as $dep => $dep_data) {
                    $pkg_data['deps'][] = $dep . '-' . $dep_data['version'];
                }
            }

            $pkgs[] = $pkg_data;
        }

        return $pkgs;
    }
}
