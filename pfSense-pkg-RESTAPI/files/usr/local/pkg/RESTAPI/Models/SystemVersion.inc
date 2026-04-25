<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the pfSense version of the current system.
 */
class SystemVersion extends Model {
    const VERSION_PATH = '/etc/version';
    const VERSION_PATCH_PATH = '/etc/version.patch';
    const VERSION_BUILDTIME_PATH = '/etc/version.buildtime';

    public StringField $version;
    public StringField $base;
    public StringField $patch;
    public StringField $buildtime;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_pfsense_version';

        # Set model Fields
        $this->version = new StringField(
            read_only: true,
            verbose_name: 'Version',
            help_text: 'The official pfSense version release name. (e.g. X.X.X-RELEASE)',
        );
        $this->base = new StringField(
            read_only: true,
            verbose_name: 'Base',
            help_text: 'The base pfSense version. For pfSense CE, this will be the major and minor version but not ' .
                'the patch. For pfSense Plus, this will be the version year and month but not the patch.',
        );
        $this->patch = new StringField(
            read_only: true,
            verbose_name: 'Patch',
            help_text: "The pfSense build's patch version.",
        );
        $this->buildtime = new StringField(
            read_only: true,
            verbose_name: 'Build Time',
            help_text: 'The datetime string of when the pfSense version was initially built.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains details about the current running version of pfSense.
     * @return array An array of the running pfSense version details
     */
    protected static function get_pfsense_version(): array {
        # VARIABLES
        $ver_data = [];

        # Ensure each file exists, then read it's contents and save it to our version data.
        if (file_exists(self::VERSION_PATH)) {
            # Read full release version
            $version = file_get_contents(self::VERSION_PATH);
            $version = str_replace(PHP_EOL, '', $version);
            $ver_data['version'] = $version;

            # Discard everything but the version tag for our base version (e.g. 2.6.0)
            $ver_data['base'] = explode('-', $version)[0];
        }
        if (file_exists(self::VERSION_PATCH_PATH)) {
            $ver_patch_file = file_get_contents(self::VERSION_PATCH_PATH);
            $ver_patch = str_replace(PHP_EOL, '', $ver_patch_file);
            $ver_data['patch'] = $ver_patch;
        }
        if (file_exists(self::VERSION_BUILDTIME_PATH)) {
            $ver_bt_file = file_get_contents(self::VERSION_BUILDTIME_PATH);
            $ver_bt = str_replace(PHP_EOL, '', $ver_bt_file);
            $ver_data['buildtime'] = $ver_bt;
        }

        return $ver_data;
    }
}
