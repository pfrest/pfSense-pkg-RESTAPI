<?php

namespace RESTAPI\Forms;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Form;
use RESTAPI\Models\RESTAPIJWT;

/**
 * Defines the Form for configuring REST API settings in the pfSense webConfigurator at /system_restapi_settings.php.
 */
class SystemRESTAPISettingsForm extends Form {
    public string $url = '/system_restapi_settings.php';
    public string $model_class = 'RESTAPISettings';
    public array $title_path = ['System', 'REST API', 'Settings'];
    public array $tabs = [
        ['Settings', true, '/system_restapi_settings.php'],
        ['Keys', false, '/system_restapi_key.php'],
        ['Access List', false, '/system_restapi_acl.php'],
        ['Updates', false, '/system_restapi_updates.php'],
        ['Documentation', false, '/api/v2/documentation'],
    ];
    public array $sections = [
        'General' => [
            'fields' => ['enabled', 'keep_backup', 'represent_interfaces_as'],
        ],
        'Security' => [
            'fields' => [
                'allowed_interfaces',
                'read_only',
                'auth_methods',
                'jwt_exp',
                'login_protection',
                'log_successful_auth',
                'log_level',
                'expose_sensitive_fields',
                'override_sensitive_fields',
            ],
        ],
        'Advanced Settings' => [
            'fields' => [
                'hateoas',
                'allow_pre_releases',
                'allow_development_packages',
                'ha_sync',
                'ha_sync_validate_certs',
                'ha_sync_hosts',
                'ha_sync_username',
                'ha_sync_password',
            ],
        ],
    ];
    public array $buttons = [
        'rotate_server_key' => [
            'title' => 'Rotate Server Key',
            'icon' => 'fa-solid fa-turn-up',
            'classes' => ['btn-success'],
            'on_click' => "return confirm(\"Rotating the server key will void any existing JWTs. Proceed?\");",
            'callable' => 'rotate_server_key',
        ],
        'report_an_issue' => [
            'title' => 'Report an Issue',
            'icon' => 'fa-solid fa-question-circle',
            'link' => 'https://github.com/pfrest/pfSense-pkg-RESTAPI/issues/new/choose',
            'classes' => ['btn-info'],
        ],
    ];

    /**
     * A button callable method that rotates the JWT server key when the `rotate_server_key` button is pressed.
     * @throws \Exception When the JWT server key fails to be generated.
     */
    public function rotate_server_key() {
        RESTAPIJWT::init_server_key(rotate: true);
        $this->print_success_banner(message: 'The JWT server key has been rotated.');
    }
}
