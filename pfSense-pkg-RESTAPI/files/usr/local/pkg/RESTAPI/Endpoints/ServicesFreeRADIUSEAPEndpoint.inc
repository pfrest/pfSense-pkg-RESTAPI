<?php

namespace RESTAPI\Endpoints;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Endpoint;

/**
 * Defines an Endpoint for interacting with the singleton FreeRADIUSEAP Model object at
 * /api/v2/services/freeradius/eap
 */
class ServicesFreeRADIUSEAPEndpoint extends Endpoint {
    public function __construct() {
        # Set Endpoint attributes
        $this->url = '/api/v2/services/freeradius/eap';
        $this->model_name = 'FreeRADIUSEAP';
        $this->request_method_options = ['GET', 'PATCH'];

        # Construct the parent Endpoint object
        parent::__construct();
    }
}
