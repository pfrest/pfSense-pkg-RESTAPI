<?php

namespace RESTAPI\Caches;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Cache;
use function RESTAPI\Core\Tools\get_classes_from_namespace;

/**
 * Defines a Cache that is responsible for obtaining the privileges for REST API Endpoints and Forms and caching them
 * to a centralized location where it can be read by /etc/inc/priv/restapi.priv.inc.
 */
class PrivilegesCache extends Cache {
    /**
     * Loop through each Endpoint and Cache objects and generate privileges for them.
     */
    public function get_data_to_cache(): array {
        # Use a variable to keep track of the privileges associated with this endpoint
        $privs = [
            "page-system-restapi-documentation" => [
                "name" => "WebCfg - System:  RESTAPI Documentation",
                "descr" => "Allow access to the /api/v2/documentation page.",
                "match" => ["api/v2/documentation*"],
            ]
        ];

        # Obtain privileges for each Endpoint class
        foreach (get_classes_from_namespace('\\RESTAPI\\Endpoints\\') as $endpoint_class) {
            $endpoint_object = new $endpoint_class();
            $privs += $endpoint_object->generate_pfsense_privs();
        }

        # Obtain privileges for each Form class
        foreach (get_classes_from_namespace('\\RESTAPI\\Forms\\') as $form_class) {
            $form_object = new $form_class();
            $privs += $form_object->generate_pfsense_privs();
        }

        return $privs;
    }
}
