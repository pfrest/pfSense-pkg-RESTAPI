<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\WebGUIRestartDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interacting with the system's web GUI settings.
 */
class WebGUISettings extends Model {
    public StringField $protocol;
    public PortField $port;
    public ForeignModelField $sslcertref;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'system/webgui';
        $this->update_strategy = 'merge';
        $this->many = false;
        $this->always_apply = true;

        # Set model fields
        $this->protocol = new StringField(
            default: 'https',
            choices: ['http', 'https'],
            verbose_name: 'Protocol',
            help_text: 'The protocol to use for the web GUI.',
        );
        $this->port = new PortField(
            default: '443',
            allow_alias: false,
            allow_range: false,
            verbose_name: 'Port',
            help_text: 'The port on which the web GUI listens.',
        );
        $this->sslcertref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            required: true,
            internal_name: 'ssl-certref',
            verbose_name: 'SSL Certificate',
            help_text: 'The SSL/TLS certificate to use for the web GUI.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the web GUI settings.
     * @note This method restarts the web server services which may cause a temporary loss of connectivity. It is
     * recommended to call this method with the $async property set to true to avoid blocking the API response.
     */
    public function apply(): void {
        (new WebGUIRestartDispatcher(async: $this->async))->spawn_process();
    }
}
