<?php

namespace RESTAPI\Endpoints;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Endpoint;

/**
 * Defines an Endpoint for interacting with pfSense High Availability synchronization settings.
 */
class SystemHASyncEndpoint extends Endpoint {
    public function __construct() {
        # Set Endpoint attributes
        $this->url = '/api/v2/system/hasync';
        $this->model_name = 'HASync';
        $this->request_method_options = ['GET', 'PATCH'];
        $this->tag = 'System';

        $this->get_help_text = 'Reads pfSense High Availability synchronization settings.';
        $this->patch_help_text = 'Updates pfSense High Availability synchronization settings and applies the configuration.';

        parent::__construct();
    }
}