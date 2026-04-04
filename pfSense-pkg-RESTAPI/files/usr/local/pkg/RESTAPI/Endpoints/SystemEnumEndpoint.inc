<?php

namespace RESTAPI\Endpoints;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Endpoint;

/**
 * Defines an Endpoint for calculating enums for specific model fields.
 */
class SystemEnumEndpoint extends Endpoint {
    public function __construct() {
        # Set Endpoint attributes
        $this->url = '/api/v2/system/enum';
        $this->model_name = 'Enum';
        $this->request_method_options = ['POST'];
        $this->post_help_text = 'Enumerate all possible choices for a given model field.';

        # Construct the parent Endpoint object
        parent::__construct();
    }
}
