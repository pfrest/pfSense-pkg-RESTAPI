<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use Exception;
use Firebase\JWT\JWT;
use Firebase\JWT\Key;
use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RESTAPISettingsSyncDispatcher;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Model that represents a REST API JWT.
 */
class RESTAPIJWT extends Model {
    public StringField $token;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set Model attributes
        $this->verbose_name = 'REST API JWT';

        # Set Model Fields
        $this->token = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Token',
            help_text: 'The generated JWT that can be used for JWT authentication.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Initializes the JWT server key if none is present. Optionally allows the existing JWT server key to be rotated.
     * @param bool $rotate Force the JWT server key to be rotated.
     * @throws Exception When a sufficient source of randomness is not found for `random_bytes`.
     * @return string The API's current JWT server key
     */
    public static function init_server_key(bool $rotate = false): string {
        # Obtain the API package's ID and configuration
        $pkg_id = RESTAPISettings::get_pkg_id();
        $api_config = RESTAPISettings::get_pkg_config();

        # Create a new server key if one is not set
        if (empty($api_config['server_key']) or $rotate === true) {
            # Try to generate the server key, throw a ServerError if we cannot
            try {
                Model::set_config("installedpackages/package/$pkg_id/conf/server_key", bin2hex(random_bytes(32)));
                write_config('API server key created');
            } catch (Exception) {
                throw new ServerError(
                    message: 'Server failed to initialize the JWT server key.',
                    response_id: 'API_JWT_INIT_SERVER_KEY_FAILED',
                );
            }
        }

        # Refresh the API config in case the server key changed
        $api_config = RESTAPISettings::get_pkg_config();
        (new RESTAPISettingsSyncDispatcher(async: true))->spawn_process();
        RESTAPISettings::backup_to_file();

        # Return the server key
        return $api_config['server_key'];
    }

    /**
     * Decodes a provided JWT string using the current server key and extracts its attributes.
     * @param string $jwt The incoming JWT string to decode.
     * @return array An array of decoded JWT attributes. If the JWT could not be decoded, this array will be empty.
     */
    public static function decode(string $jwt): array {
        # Obtain the current JWT server key
        $jwt_server_key = self::init_server_key();

        # Attempt to decode the JWT, if decoding fails return empty array.
        try {
            return (array) JWT::decode($jwt, new Key($jwt_server_key, 'HS256'));
        } catch (Exception) {
            return [];
        }
    }

    /**
     * Generates the JWT and assigns the `token` field.
     */
    public function _create() {
        # Initialize the JWT server key in case it is not present
        $server_key = $this->init_server_key();

        # Pull the API configuration from config
        $api_settings = new RESTAPISettings();

        # Assign the JWT payload values. Assign the configured JWT expiration from the API configuration
        $payload = [
            'iss' => config_get_path('system/hostname'), # TODO: Replace with Model call when developed
            'aud' => config_get_path('system/hostname'), # TODO: Replace with Model call when developed
            'exp' => time() + $api_settings->jwt_exp->value,
            'nbf' => time(),
            'data' => $this->client->username,
        ];

        # Assign the JWT string to `token`
        $this->token->value = JWT::encode($payload, $server_key, 'HS256');

        # Log the action
        $this->log(
            level: LOG_INFO,
            message: "Client {$this->client->username}@{$this->client->ip_address} was issued a new JWT.",
        );
    }
}
