<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RESTAPISettingsSyncDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;
use function RESTAPI\Core\Tools\get_classes_from_namespace;

# Constants
const API_SETTINGS_BACKUP_FILE_PATH = '/usr/local/share/pfSense-pkg-RESTAPI/backup.json';
const API_SETTINGS_BACKUP_NOT_CONFIGURED = -1;
const API_SETTINGS_BACKUP_SUCCESS = 0;
const API_SETTINGS_RESTORE_SUCCESS = 0;
const API_SETTINGS_RESTORE_NO_CHANGE = 1;
const API_SETTINGS_RESTORE_NO_BACKUP = 2;

/**
 * Defines a Model that represents the REST API settings.
 */
class RESTAPISettings extends Model {
    public BooleanField $enabled;
    public BooleanField $read_only;
    public BooleanField $keep_backup;
    public BooleanField $login_protection;
    public BooleanField $log_successful_auth;
    public StringField $log_level;
    public BooleanField $allow_pre_releases;
    public BooleanField $hateoas;
    public BooleanField $expose_sensitive_fields;
    public BooleanField $mcp_enabled;
    public StringField $override_sensitive_fields;
    public InterfaceField $allowed_interfaces;
    public StringField $represent_interfaces_as;
    public StringField $auth_methods;
    public IntegerField $jwt_exp;
    public BooleanField $ha_sync;
    public BooleanField $ha_sync_validate_certs;
    public StringField $ha_sync_hosts;
    public StringField $ha_sync_username;
    public StringField $ha_sync_password;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Obtain the API config index and set the config path to the API configuration
        $this->id = $this->get_pkg_id();
        $this->config_path = "installedpackages/package/$this->id/conf";
        $this->always_apply = true;
        $this->verbose_name = 'REST API Settings';

        # Set Fields for this Model
        $this->enabled = new BooleanField(
            default: true,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'Enabled',
            help_text: "Enables or disables the API. If set to `false`, the API will no longer respond to API requests 
            and can only be re-enabled via webConfigurator.",
        );
        $this->read_only = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'Read Only',
            help_text: "Enables or disables read-only API access. If enabled, the API will only respond to GET requests
            and can only be disabled via webConfigurator.",
        );
        $this->keep_backup = new BooleanField(
            default: true,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'Keep Backup',
            help_text: "Enables or disables keeping a persistent backup of the API configuration that can be used
            to restore the API configuration after package and systems updates.",
        );
        $this->login_protection = new BooleanField(
            default: true,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'Login Protection',
            help_text: "Enables or disables Login Protection for API authentication. When enabled, Login Protection will
            monitor API attempts and temporarily block clients who fail API authentication too many times within a
            period of time. When disabled, Login Protection will not monitor API authentication but will continue to 
            monitor webConfigurator and SSH logins (if configured). Login Protection can be configured globally in
            System > Advanced.",
        );
        $this->log_successful_auth = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'log successful authentication',
            help_text: "Enables or disables logging of API authentication attempts that are successful. By default, only
            failed API authentication attempts are logged to prevent flooding the authentication logs. This field is
            only applicable when the API `login_protection` setting is enabled.",
        );
        $this->log_level = new StringField(
            default: 'LOG_WARNING',
            choices: [
                'LOG_DEBUG' => 'Debug',
                'LOG_INFO' => 'Info',
                'LOG_NOTICE' => 'Notice',
                'LOG_WARNING' => 'Warning',
                'LOG_ERR' => 'Error',
                'LOG_CRIT' => 'Critical',
                'LOG_ALERT' => 'Alert',
                'LOG_EMERG' => 'Emergency',
            ],
            verbose_name: 'log level',
            help_text: 'Sets the log level for API logging. The log level determines the minimum severity of messages 
            that should be logged.',
        );
        $this->allow_pre_releases = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'allow pre-releases',
            help_text: "Enables or disables displaying pre-releases in available API updates. Pre-releases contain fixes
            and features that are currently under development and may not be fully stable. Use of pre-release versions
            is at your own risk.",
        );
        $this->hateoas = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'enable HATEOAS',
            help_text: 'Enables or disables HATEOAS. Enabling HATEOAS will allow the API to include links to related ' .
                'resources in API responses. This is primarily useful for frontend web applications and self-navigating ' .
                'client scripts that integrate with HAL standards. Enabling HATEOAS may increase API response times, ' .
                'especially on systems with large configurations.',
        );
        $this->expose_sensitive_fields = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'expose sensitive fields',
            help_text: 'Enables or disables exposing sensitive fields in API responses. When enabled, sensitive fields ' .
                'such as passwords, private keys, and other sensitive data will be included in API responses.',
        );
        $this->mcp_enabled = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'enable MCP server',
            help_text: 'Enables or disables the Model Context Protocol (MCP) server at /api/v2/mcp. When enabled, ' .
                'AI agents and other MCP clients can dynamically discover and invoke REST API operations as MCP ' .
                'tools using the same authentication, authorization and privileges as the standard REST API. The ' .
                'MCP server is disabled by default and must be explicitly enabled here.',
        );
        $this->override_sensitive_fields = new StringField(
            default: [],
            choices_callable: 'get_override_sensitive_fields_choices',
            many: true,
            conditions: ['expose_sensitive_fields' => false],
            verbose_name: 'Override Sensitive Fields',
            help_text: 'Specifies a list of fields (formatted as ModelName:FieldName) that should have their sensitive ' .
                'attribute overridden. Fields selected here will not be considered sensitive and will be included in ' .
                'API responses regardless of the `expose_sensitive_fields` setting.',
        );
        $this->allowed_interfaces = new InterfaceField(
            default: [],
            allow_localhost_interface: true,
            allow_empty: true,
            many: true,
            delimiter: ',',
            verbose_name: 'Allowed Interfaces',
            help_text: 'Sets the interfaces allowed to accept incoming API calls.',
        );
        $this->represent_interfaces_as = new StringField(
            default: 'descr',
            choices: ['descr' => 'Description', 'id' => 'Internal ID', 'if' => 'Real interface'],
            verbose_name: 'Represent Interfaces As',
            help_text: "Specifies how the API should represent interface names. Use `descr` to represent 
            interface objects by their description name, use `id` to represent interface objects by their
            internal pfSense ID (e.g. wan, lan, opt1), or use `if` to represent interface objects by their
            real interface name (e.g. em0, igb1, bxe3).",
        );
        $this->auth_methods = new StringField(
            default: ['BasicAuth'],
            choices_callable: 'get_auth_methods_choices',
            many: true,
            many_minimum: 1,
            verbose_name: 'authentication methods',
            help_text: 'Sets the API authentication methods allowed to authenticate API calls.',
        );
        $this->jwt_exp = new IntegerField(
            default: 3600,
            minimum: 300,
            maximum: 86400,
            verbose_name: 'JWT expiration',
            help_text: 'Sets the amount of time (in seconds) JWTs are valid for.',
        );
        $this->ha_sync = new BooleanField(
            default: false,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'HA sync',
            help_text: "Enables or disables syncing API settings to HA peers. When enabled, API settings from this
            host will automatically be synced to any hosts defined in `ha_sync_hosts`.",
        );
        $this->ha_sync_validate_certs = new BooleanField(
            default: true,
            indicates_true: 'enabled',
            indicates_false: 'disabled',
            verbose_name: 'HA sync validate certs',
            help_text: "Enables or disables certificate validation when syncing API configurations to HA sync peers. If
            enabled, all hosts defined in `ha_sync_hosts` must have their webConfigurator configured with a certificate
            trusted by this system. It is strongly recommended this be enabled at all times to help mitigate 
            Man-in-the-Middle attacks.",
        );
        $this->ha_sync_hosts = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            many_minimum: 0,
            many_maximum: 10,
            delimiter: ' ',
            verbose_name: 'HA sync hosts',
            validators: [new IPAddressValidator(allow_fqdn: true)],
            help_text: 'Set a list of IP addresses or hostnames to sync API settings to.',
        );
        $this->ha_sync_username = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'HA username',
            help_text: "Sets the username to use when authenticating for HA sync processes. This user must be the present
            on all hosts defined in `ha_sync_hosts`.",
        );
        $this->ha_sync_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            verbose_name: 'HA sync password',
            help_text: "Sets the password to use when authenticating for HA sync processes. This must be the password
            for the user defined in `ha_sync_username` and must be the same on all hosts defined in `ha_sync_hosts`.",
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the choices for the `auth_methods` field. This will dynamically populate a choice for each Auth class
     * in the \RESTAPI\Auth\ namespace.
     */
    public function get_auth_methods_choices(): array {
        # Variables
        $choices = [];

        # Format choices so they include the class's verbose name as the choice's verbose name
        foreach (get_classes_from_namespace(namespace: '\\RESTAPI\\Auth\\') as $auth_class) {
            $auth = new $auth_class();
            $choices[$auth->get_class_shortname()] = $auth->verbose_name;
        }

        return $choices;
    }

    /**
     * Obtains the choices for the `override_sensitive_fields` field. This will dynamically populate a choice for each
     * Model and Field in the \RESTAPI\Models namespace.
     */
    public function get_override_sensitive_fields_choices(): array {
        # Variables
        $choices = [];

        # Format choices so they include the class's verbose name as the choice's verbose name
        foreach (get_classes_from_namespace(namespace: '\\RESTAPI\\Models\\') as $model_class) {
            $model = new $model_class(skip_init: true);
            foreach ($model->get_fields() as $field) {
                if ($model->$field->sensitive) {
                    $choices[$model->get_class_shortname() . ':' . $field] =
                        $model->get_class_shortname() . ':' . $field;
                }
            }
        }

        return $choices;
    }

    /**
     * Creates a backup of the current API configuration at /usr/local/share/pfSense-pkg-RESTAPI/backup.json.
     * @returns integer Returns 0 when backup was successful, or -1 when the API's `keep_backup` field is disabled.
     */
    public static function backup_to_file(): int {
        # Read current API settings
        $api_config = RESTAPISettings::get_pkg_config();

        # Don't allow restore if the `keep_backup` is not enabled
        if ($api_config['keep_backup'] !== 'enabled') {
            return API_SETTINGS_BACKUP_NOT_CONFIGURED;
        }

        file_put_contents(API_SETTINGS_BACKUP_FILE_PATH, json_encode($api_config));
        return API_SETTINGS_BACKUP_SUCCESS;
    }

    /**
     * Restores the API configuration using the current backup file at /usr/local/share/pfSense-pkg-RESTAPI/backup.json
     * @returns integer Returns 0 when the restore was successful, 1 when there was no chanaged between the current
     * API configuration and the backup file, 2 when no backup file exists current, or -1 when the API's `keep_backup`
     * field is disabled.
     */
    public static function restore_from_backup(): int {
        # Variables
        $backup_api_conf_json = is_file(API_SETTINGS_BACKUP_FILE_PATH)
            ? file_get_contents(API_SETTINGS_BACKUP_FILE_PATH)
            : '';
        $backup_api_conf = $backup_api_conf_json ? json_decode($backup_api_conf_json, true) : [];
        $current_api_id = RESTAPISettings::get_pkg_id();
        $current_api_conf = RESTAPISettings::get_pkg_config();

        # Don't allow restore if the `keep_backup` is not enabled
        if ($current_api_conf['keep_backup'] !== 'enabled') {
            return API_SETTINGS_BACKUP_NOT_CONFIGURED;
        }

        # Don't allow restore if there is no backup present
        if (!$backup_api_conf) {
            return API_SETTINGS_RESTORE_NO_BACKUP;
        }

        # Don't allow restore if there is no change to backup
        if ($current_api_conf === $backup_api_conf) {
            return API_SETTINGS_RESTORE_NO_CHANGE;
        }

        Model::set_config("installedpackages/package/$current_api_id/conf", $backup_api_conf);
        write_config('Synchronized persistent API configuration');
        return API_SETTINGS_RESTORE_SUCCESS;
    }

    /**
     * Obtains the internal pfSense-pkg-RESTAPI configuration.
     * @return array An associative array containing the pfSense-pkg-RESTAPI configuration
     */
    public static function get_pkg_config(): array {
        foreach (self::get_config('installedpackages/package') as $installed_package) {
            if ($installed_package['name'] === 'RESTAPI') {
                return $installed_package['conf'];
            }
        }
        return [];
    }

    /**
     * Obtains the `installedpackage` configuration ID for the pfSense-pkg-RESTAPI package.
     * @return int The pfSense-pkg-RESTAPI package's `installedpackage` configuration ID in integer format. If the
     * package cannot be found, this will return `-1`
     */
    public static function get_pkg_id(): int {
        return get_package_id('RESTAPI');
    }

    /**
     * Refresh the API backup file and sync changes to API clients if configured.
     */
    public function apply(): void {
        $this->backup_to_file();
        (new RESTAPISettingsSyncDispatcher(async: $this->async))->spawn_process();
    }
}
