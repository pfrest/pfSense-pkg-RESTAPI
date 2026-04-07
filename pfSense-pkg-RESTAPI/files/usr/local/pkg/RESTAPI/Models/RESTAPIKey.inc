<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\RESTAPISettingsSyncDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model that represents REST API keys.
 */
class RESTAPIKey extends Model {
    public StringField $descr;
    public StringField $username;
    public StringField $hash_algo;
    public IntegerField $length_bytes;
    public StringField $hash;
    public StringField $key;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Gather information about current API settings
        $api_pkg_id = RESTAPISettings::get_pkg_id();

        # Set Model attributes
        $this->config_path = "installedpackages/package/$api_pkg_id/conf/keys/key";
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'REST API Key';
        $this->verbose_name_plural = 'REST API Keys';

        # Set Model Fields
        $this->username = new StringField(
            default: '',
            allow_empty: true, # We will force this field to be populated in validate_username()
            read_only: true,
            verbose_name: 'Username',
            help_text: 'The username this API key is issued to.',
        );
        $this->length_bytes = new IntegerField(
            default: 24,
            choices: [16, 24, 32, 64],
            verbose_name: 'Length Bytes',
            help_text: 'The length of the API key (in bytes). Greater key lengths provide greater security, but ' .
                'also increase the number of characters used in the key string.',
        );
        $this->hash_algo = new StringField(
            default: 'sha256',
            choices: ['sha256' => 'SHA256', 'sha384' => 'SHA384', 'sha512' => 'SHA512'],
            verbose_name: 'key hashing algorithm',
            help_text: 'The hash algorithm used for this API key. It is recommended to increase the strength of the ' .
                'algorithm for keys assigned to privileged users.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            maximum_length: 128,
            verbose_name: 'description',
            help_text: "Sets a description for this API key. This is used to identify the key's purpose and cannot be " .
                'changed once created.',
        );
        $this->key = new StringField(
            allow_null: true,
            representation_only: true, # Don't store the actual API key internally
            verbose_name: 'Key',
            help_text: 'The real API key. This value is not stored internally and cannot be recovered if lost.',
        );
        $this->hash = new StringField(
            allow_null: true,
            sensitive: true, # Don't allow the hash to be read via API
            verbose_name: 'Hash',
            help_text: 'The hash of the generated API key',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds custom validation to the `username` field.
     * @param string $username The incoming username value to be validated.
     * @return string The validated username value to be assigned.
     */
    public function validate_username(string $username): string {
        # Force username to match the authenticating user's name
        $username = $this->client->username;

        # Require this username to match an existing user
        # TODO: Replace this loop with a User model query when developed
        if (User::query(['name' => $username])->exists()) {
            return $username;
        }

        throw new ValidationError(
            message: "Field `username` must be an existing user's username, received `$username`",
            response_id: 'API_KEY_USERNAME_MUST_EXIST',
        );
    }

    /**
     * Generates the API key and hash values and saves the Model to config.
     */
    public function _create() {
        # Generate the API key and its hash
        $this->key->value = bin2hex(random_bytes($this->length_bytes->value));
        $this->hash->value = hash($this->hash_algo->value, $this->key->value);

        # Call the parent _create() method to save the Model object to config
        parent::_create();
    }

    /**
     * Override the default _update() method to prevent RESTAPIKeys from being updated.
     */
    public function _update() {
        # Do not allow RESTAPIKey objects to be updated.
        throw new RESTAPI\Responses\ServerError(
            message: 'RESTAPIKey model objects cannot be updated.',
            response_id: 'API_KEY_CANNOT_BE_UPDATED',
        );
    }

    /**
     * Attempts to authenticate an API key against this Model object.
     * @param string $key The real API key to authenticate with.
     * @return bool true if the hashed $key matches this Model object's `hash` property, otherwise false.
     */
    public function authenticate(string $key): bool {
        # Hash the incoming key according to the algorithm of this stored key
        $hash = hash($this->hash_algo->value, $key);

        # Authentication is successful when incoming key matches the hashed key stored in config
        if ($hash === $this->hash->value) {
            return true;
        }

        return false;
    }

    /**
     * Refresh the API backup file and sync changes to API clients if configured.
     */
    public function apply(): void {
        RESTAPISettings::backup_to_file();
        (new RESTAPISettingsSyncDispatcher(async: $this->async))->spawn_process();
    }
}
