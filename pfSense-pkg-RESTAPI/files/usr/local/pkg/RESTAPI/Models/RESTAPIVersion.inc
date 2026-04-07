<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Model that represents the REST API version installed on this system.
 */
class RESTAPIVersion extends Model {
    const REPO_URL = 'https://github.com/pfrest/pfSense-pkg-RESTAPI';

    public StringField $current_version;
    public StringField $latest_version;
    public StringField $latest_version_release_date;
    public BooleanField $update_available;
    public StringField $install_version;
    public StringField $available_versions;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_api_version_details';
        $this->cache_class = 'RESTAPIVersionReleasesCache';
        $this->verbose_name = 'REST API Version';

        # Set model Fields
        $this->current_version = new StringField(
            read_only: true,
            verbose_name: 'Current Version',
            help_text: 'The current API version installed on this system.',
        );
        $this->latest_version = new StringField(
            read_only: true,
            verbose_name: 'Latest Version',
            help_text: 'The latest API version available to this system.',
        );
        $this->latest_version_release_date = new StringField(
            read_only: true,
            verbose_name: 'Release date',
            help_text: "The latest API version's release date.",
        );
        $this->update_available = new BooleanField(
            read_only: true,
            indicates_true: true,
            verbose_name: 'Update Available',
            help_text: 'Indicates if an API update is available for this system.',
        );
        $this->install_version = new StringField(
            choices_callable: 'get_all_available_versions',
            allow_null: true,
            write_only: true,
            verbose_name: 'Install Version',
            help_text: 'Set the API version to update or rollback to.',
        );
        $this->available_versions = new StringField(
            default: [],
            read_only: true,
            many: true,
            delimiter: null,
            verbose_name: 'Available Versions',
            help_text: 'All available versions of the REST API package for this system.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains API version details. This method is the internal callable for this model.
     * @return array An array of API version details that correspond with this model's Fields.
     */
    protected function get_api_version_details(): array {
        return [
            'current_version' => $this->get_api_version(),
            'latest_version' => $this->get_latest_api_version(),
            'latest_version_release_date' => $this->get_latest_api_release_date(),
            'update_available' => $this->is_update_available(),
            'available_versions' => array_keys($this->get_all_available_versions()),
        ];
    }

    /**
     * Updates the REST API package to the requested version.
     */
    public function _update(): void {
        # Default the result to true in case nothing needs to be done
        $result = true;

        # Only attempt an update if an install version is provided
        if (!$this->install_version->value) {
            return;
        }

        # Install in the background if $async is set to true
        if ($this->async) {
            $this->install_version_async(tag: $this->install_version->value);
        }
        # Otherwise, install the version synchronously
        else {
            $result = $this->install_version(tag: $this->install_version->value);
            $this->from_internal();
        }

        # Notify the user of installation failures
        if ($result === false) {
            throw new ServerError(
                message: 'The REST API package failed to install the requested version. Check logs for more info.',
                response_id: 'RESTAPI_VERSION_INSTALLATION_FAILED',
            );
        }
    }

    /**
     * Obtains the current API version installed on the system.
     * @return string The current API version installed in vX.X.X format
     */
    public static function get_api_version(): string {
        # Pull the raw pkg info for the API package into an array for each line
        $pkg_info = explode(PHP_EOL, shell_exec('pkg-static info pfSense-pkg-RESTAPI'));

        # Loop through each line and check the version
        foreach ($pkg_info as $pkg_line) {
            if (str_starts_with($pkg_line, 'Version')) {
                # Locate the version and format it to a standard semantic version format (x.x.x)
                $version = str_replace(' ', '', $pkg_line);
                $version = explode(':', $version)[1];
                $version = strlen($version) === 3 ? $version . '.0' : $version;
                $version = str_replace('_', '.', $version);
                return "v$version";
            }
        }

        return 'unknown';
    }

    /**
     * Obtains the latest API version available to this system.
     * @return string|null The latest API version available to this system or `null` if none are available.
     */
    public function get_latest_api_version(): string|null {
        # Fetch our latest version and format it semantically (x.x.x)
        return array_key_first($this->get_all_available_versions());
    }

    /**
     * Obtains the release date of the latest API version available to this system.
     * @return string The release date of the latest API version available to this system.
     */
    public function get_latest_api_release_date(): string {
        # Loop through each release and locate the latest available releases creation date
        foreach ($this->get_github_releases() as $release) {
            # Check if this releases is latest available for our platform
            if ($release['tag_name'] === self::get_latest_api_version()) {
                return $release['created_at'];
            }
        }

        return 'unknown';
    }

    /**
     * Determines if an API update is available to this system.
     * @return bool `true` if there is an API update available to this system. `false` if it is up-to-date.
     */
    public function is_update_available(): bool {
        return version_compare($this->get_api_version(), $this->get_latest_api_version(), operator: '<');
    }

    /**
     * Obtains all releases for the pfsense-RESTAPI package via GitHub API. To avoid GitHub API rate limits, this
     * method also creates a local cache of obtained releases. All subsequent calls of this method will simply
     * return the cached releases. The cache can be renewed every 2 minutes by rerunning this method.
     * @return array An array of all releases from the pfsense-RESTAPI repository on GitHub via GitHub API.
     * @link https://docs.github.com/en/rest/releases/releases?apiVersion=2022-11-28#list-releases
     */
    public function get_github_releases(): array {
        return $this->cache->read();
    }

    /**
     * Obtains all pfSense-pkg-RESTAPI versions available to this version of pfSense.
     * @returns array Returns an array of all REST API versions available to this host. The array keys will be the
     * semantic version strings and the array value will a descriptive representation of the version.
     */
    public function get_all_available_versions(): array {
        # Variables
        $pf_version = new SystemVersion();
        $api_settings = new RESTAPISettings();
        $versions = [];
        $count = 0;
        $releases = $this->get_github_releases();

        # Loop through each version and populate our version data
        foreach ($releases as $release) {
            # Loop through the assets of each release and check if our version of pfSense is supported
            foreach ($release['assets'] as $asset) {
                # Skip pre-releases if pre-releases are not allowed
                if ($release['prerelease'] and !$api_settings->allow_pre_releases->value) {
                    continue;
                }

                # Only include releases for this version of pfSense
                if ($asset['name'] === 'pfSense-' . $pf_version->base->value . '-pkg-RESTAPI.pkg') {
                    # The first item of our list is the latest release, mark it as such.
                    if ($count === 0) {
                        $versions[$release['tag_name']] = $release['name'] . ' - Latest';
                        $count++;
                    } else {
                        $versions[$release['tag_name']] = $release['name'];
                    }
                }
            }
        }
        return $versions;
    }

    /**
     * Installs the specified version of the REST API package.
     * @param string $tag The tag of the version to install.
     * @return bool|null `true` if the version was successfully installed. `false` if the version failed to install.
     * `null` if the version is not available.
     */
    public function install_version(string $tag): ?bool {
        # Variables
        $pf_version = (new SystemVersion())->base->value;
        $install_url = escapeshellarg(self::REPO_URL . "/releases/download/$tag/pfSense-$pf_version-pkg-RESTAPI.pkg");

        # Don't perform any changes if we are already running the requested version
        if ($this->current_version->value === $tag) {
            return true;
        }

        # Ensure this version is available to this host
        if (in_array($tag, array_keys($this->get_all_available_versions()))) {
            # Remove the current REST API package and install the target version
            $delete = new Command('pkg-static delete -y pfSense-pkg-RESTAPI');
            $add = new Command("pkg-static add $install_url");

            # If an error occurred either deleting or adding the package, return false
            if ($delete->result_code !== 0 or $add->result_code !== 0) {
                $this->log(level: LOG_ERR, message: $delete->output);
                $this->log(level: LOG_ERR, message: $add->output);
                return false;
            }

            return true;
        }

        return null;
    }

    /**
     * Runs install_version() in the background by calling php -f /usr/local/pkg/RESTAPI/.resources/scripts/manage.php
     * revert in a nohup process.
     * @param string $tag The tag of the version to install.
     */
    public static function install_version_async(string $tag): void {
        $tag = escapeshellarg($tag);
        new Command(
            command: "nohup php -f /usr/local/pkg/RESTAPI/.resources/scripts/manage.php revert $tag",
            redirect: '> /dev/null 2>&1 &',
        );
    }
}
