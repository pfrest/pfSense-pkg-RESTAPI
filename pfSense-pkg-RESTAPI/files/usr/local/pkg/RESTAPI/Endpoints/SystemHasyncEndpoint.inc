<?php

namespace RESTAPI\Endpoints;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Endpoint;

/**
 * Defines an Endpoint for interacting with the SystemHasync Model object at /api/v2/system/hasync.
 */
class SystemHasyncEndpoint extends Endpoint {
    public function __construct() {
        # Set Endpoint attributes
        $this->url = '/api/v2/system/hasync';
        $this->model_name = 'SystemHasync';
        $this->request_method_options = ['GET', 'PATCH'];
        $this->get_help_text = 'Reads the current HA sync settings.';
        $this->patch_help_text = 'Updates the HA sync settings.';

        # Construct the parent Endpoint object
        parent::__construct();
    }
}
