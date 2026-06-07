<?php

namespace RESTAPI\Caches;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Cache;
use RESTAPI\Models\RESTAPIVersion;

/**
 * Defines a Cache that can be used to update the available pfSense-pkg-RESTAPI releases cache. This Cache
 * runs on a schedule to periodically update the releases cache in the background.
 */
class RESTAPIVersionReleasesCache extends Cache {
    const RELEASES_URL = 'https://api.github.com/repos/pfrest/pfSense-pkg-RESTAPI/releases';

    public int $timeout = 30;
    public string $schedule = '0 * * * *';

    /**
     * Retrieves available release information from external repos and updates the releases cache files.
     */
    public function get_data_to_cache(): array {
        # Retrieve the current API version for the User-Agent header
        $api_version = RESTAPIVersion::get_api_version();

        # Retrieve the available releases from the GitHub API
        $releases_json = \RESTAPI\Core\Tools\http_request(
            url: self::RELEASES_URL,
            method: 'GET',
            headers: ["User-Agent: pfSense-pkg-RESTAPI/$api_version"],
        );
        $releases = json_decode($releases_json, true);

        # If we don't have an 'assets' key, the response is invalid. Log the response, keep the existing cache
        if (!is_array($releases) or !array_key_exists(0, $releases) or !array_key_exists('assets', $releases[0])) {
            Cache::log(
                level: LOG_ERR,
                message: "Releases data is malformed, received $releases_json. Keeping existing cache.",
            );
            return $this->read();
        }

        # Return the fetched releases
        return $releases;
    }
}
