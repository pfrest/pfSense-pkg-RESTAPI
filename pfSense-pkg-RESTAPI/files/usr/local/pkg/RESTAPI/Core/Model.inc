<?php
//   Copyright 2024 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

namespace RESTAPI\Core;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\ForbiddenError;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ServiceUnavailableError;
use RESTAPI\Responses\ValidationError;
use function RESTAPI\Core\Tools\get_classes_from_namespace;

# Constants
const DEFAULT_CLIENT_USERNAME = '(system)';
const DEFAULT_CLIENT_IP_ADDRESS = '127.0.0.1';

/**
 * Defines a Model object that relates to a pfSense configuration or service. This model defines the necessary steps
 * to validate incoming configuration and create, read, update, and/or delete data from the pfSense configuration this
 * object relates to.
 */
class Model {
    use BaseTraits;

    /**
     * @var string $WRITE_LOCK_FILE
     * The file path used to create a lock when writing to the pfSense configuration file. This prevents multiple
     * simultaneous writes to the config file that could corrupt the config.
     */
    const WRITE_LOCK_FILE = '/tmp/.RESTAPI.write_config.lock';

    /**
     * @var string|null $config_path
     * The configuration path where this Model's object(s) are internally stored. This property is mandatory unless
     * an $internal_callable is explicitly defined. If a $parent_model_class is assigned, this path will be relative
     * to the parent Model's $config_path. Otherwise, this is an absolute config path.
     */
    public string|null $config_path = null;

    /**
     * @var bool $many
     * If set to `true`, this Model will facilitate the creation of multiple objects of its kind and enable
     * IDs for this field. If set to `false`, this Model will exclusively support a single object and abstain from utilizing IDs.
     */
    public bool $many = false;

    /**
     * @var int $many_minimum
     * Specifies the minimum number of associated Models that must be present at all times. If a deletion is requested,
     * and it would result in the number of associated objects falling below this value, the deletion will be denied.
     * This condition is applicable only when the $many property is set to true.
     */
    public int $many_minimum = 0;

    /**
     * @var int $many_maximum
     * Specifies the maximum number of associated Models that can be present at any time. If a new creation is requested,
     * and it would result in the number of associated objects growing beyond this value, the creation will be denied.
     * This condition is applicable only when the $many property is set to true.
     */
    public int $many_maximum = 65535;

    /**
     * @var mixed $id
     * The ID of the current Model object. This applies exclusively to Models with $many enabled. New Model objects
     * will not possess an ID until the `create()` method is executed.
     */
    public mixed $id = null;

    /**
     * @var string $id_type
     * The data type of the ID field for this Model. In most cases, this should be set to 'integer'. In some cases,
     * the ID field may be a 'string'. This field should be set accordingly to ensure documentation and validation
     * processes are accurate. This applies exclusively to Models with $many enabled.
     */
    public string $id_type = 'integer';

    /**
     * @var bool $auto_create_id
     * Enables or disables automatic creation of an ID for this Model object when the `create()` method is called. If
     * set to `true`, the ID will be automatically generated. If set to `false`, an ID must be provided before the
     * `create()` method is called. This applies exclusively to Models with $many enabled and is usually only
     * relevant to Model classes that use an internal callable method.
     */
    public bool $auto_create_id = true;

    /**
     * @var mixed $parent_id
     * For Models acting as children to a different Model class, this property will contain the ID of the parent model
     * object. If a $parent_model_class is defined, this value MUST be specified during object construction.
     */
    public mixed $parent_id = null;

    /**
     * @var string $parent_id_type
     * The data type of the parent ID field for this Model. In most cases, this should be set to 'integer'. In some cases,
     * the parent ID field may be a 'string'. This field should be set accordingly to ensure documentation and validation
     * processes are accurate. This applies exclusively to Models with a $parent_model_class assigned.
     */
    public string $parent_id_type = 'integer';

    /**
     * @var string $parent_model_class
     * The class name of the parent Model class. This is necessary only if this Model is nested under another Model.
     */
    public string $parent_model_class = '';

    /**
     * @var Model|null
     * The parent Model object of this Model. This property is relevant only if a $parent_model_class is assigned.
     * This property is automatically set using the $parent_model_class and $parent_id properties accordingly. If no
     * parent Model class is found, or the parent object does not exist, this value will be `null`.
     */
    public Model|null $parent_model = null;

    /**
     * @var string $cache_class
     * The class name of a Cache object this Model utilizes. This is only relevant to Model who pull data from a local
     * cache file using a Cache object and typically is only used for Models with an `internal_callable` assigned. The
     * value assigned here must be an existing class shortname in the \RESTAPI\Caches\ namespace.
     */
    public string $cache_class = '';

    /**
     * @var Cache|null
     * The Cache object of the class specified in the $cache_class property. This is only applicable if a $cache_class
     * was assigned.
     */
    public Cache|null $cache = null;

    /**
     * @var bool $model_cache_exempt
     * If set to `true`, this Model will be exempt from any global Model caching mechanisms. This is primarily
     * used for Models that frequently change or are not suitable for caching.
     */
    public bool $model_cache_exempt;

    /**
     * @var ModelSet $related_objects
     * A ModelSet containing foreign Model objects related to this Model. These are primarily populated by
     * ForeignModelFields assigned to this Model object. If no related Models are found, this ModelSet will not contain
     * any Model objects. This property is primarily used by Endpoint classes to determine the correct HATEOAS link to
     * include in REST API responses.
     */
    public ModelSet $related_objects;

    /**
     * @var bool $async
     * If set to `true`, this Model will attempt to apply objects in the background. If set to `false`, this Model will
     * wait for the `apply*()` methods to complete before providing a response. In many cases, the `apply*()` methods can
     * take significant time to complete and, in extreme cases, can cause timeouts. Using $async enables these processes
     * to run in the background, allowing the client to proceed without waiting. Polling Models should be made available
     * for Models relying on $async, enabling clients to check the status of the process. Note: Not all Model classes will
     * support or respect this value. Only Models utilizing Dispatcher objects support $async.
     */
    public bool $async;

    /**
     * @var Auth $client
     * The Auth object containing the username, IP address, and other authentication details of the individual interacting
     * with this Model. If no value is assigned, a default Auth object will be defined. Note: Some Models require
     * information from the $client; therefore, this value must always be assigned.
     */
    public Auth $client;

    /**
     * @var Model|null $initial_object
     * Contains the current Model object as stored in config/internally. This can be used to reference values that may
     * have changed during creation, modification, or deletion.
     */
    public Model|null $initial_object = null;

    /**
     * @var array $protected_model_query
     * Defines query parameters for existing objects of this Model that should never be replaced or deleted. This can be
     * used to protect built-in objects that should not be deleted. This property only applies to $many enabled Models.
     */
    public array $protected_model_query = [];

    /**
     * @var array $packages
     * Specifies any add-on packages required for this Model to operate. This must be an array of package full-names
     * (i.e., pfSense-pkg-sudo) or left empty to not require any add-on packages.
     */
    public array $packages = [];

    /**
     * @var array $package_includes
     * Assigns PHP files to include that are specific to one or more of the assigned $packages. These files will be
     * imported during Model validation to prevent an error from being thrown if the Model is referenced on a system
     * without these packages installed.
     */
    public array $package_includes = [];

    /**
     * @var array $unique_together_fields
     * An array specifying field names that must be unique together. This is similar to the Field object's $unique
     * property but requires the combination of each of these field's values to be unique from any other Model object of
     * its kind. Each value specified here must be an existing property for this Model and must be assigned a Field class.
     */
    public array $unique_together_fields = [];

    /**
     * @var string $internal_callable
     * For Models that do not interact with stored pfSense configuration values, specify the name of a callable method here
     * to pull the internal values for this Model. This property cannot be used if a $config_path is already specified.
     * This must be set to an existing method name assigned to this Model class.
     */
    public string $internal_callable = '';

    /**
     * @var int|null $placement
     * Set the config index where this object should be written to. This allows objects to be moved or placed into any
     * arbitrary index in the config. This property is only applicable if the $config_path and $many properties are set.
     */
    public int|null $placement = null;

    /**
     * @var int $sort_order
     * For $many enabled Models, this property can be used to set the PHP sort option used when writing Model objects to
     * config. This property is only applicable if the $sort_by_field property is also defined. This property only applies
     * to Models with a $config_path defined. For valid value options for this property, refer to:
     * https://www.php.net/manual/en/function.array-multisort.php
     */
    public int $sort_order = SORT_ASC;

    /**
     * @var int $sort_flags
     * For $many enabled Models, this property can be used to set the PHP sort flags used when writing Model objects to
     * config. This property is only applicable if the $sort_by_field property is also defined. This property only applies
     * to Models with a $config_path defined. For valid value options for this property, refer to:
     * https://www.php.net/manual/en/function.array-multisort.php
     */
    public int $sort_flags = SORT_REGULAR;

    /**
     * @var array|null $sort_by
     * Sets the field names this Model will use when sorting objects written to the pfSense configuration. These fields must
     * be set to an existing property name from this Model, and that property must be assigned a Field class. Leave empty
     * to disable sorting for this Model. In most cases, sorting should not be used.
     */
    public array|null $sort_by = null;

    /**
     * @var string $subsystem
     * The pfSense subsystem this Model relates to. This is typically only relevant for Models that allow the internal
     * config to be written but applied at a later time. The subsystem specified here will automatically be marked as
     * 'dirty' (i.e., has unapplied changes) whenever this Model writes to the config. It will automatically have the
     * subsystem 'dirty' status cleared after the `apply()` method(s) are called. The name of the pfSense subsystem
     * depends on the pfSense configuration item this Model relates. Refer to the pfSense source files to determine what
     * (if any) subsystem should be set. Typically, it will appear as a parameter value in the mark_subsystem_dirty() or
     * clear_subsystem_dirty() pfSense functions.
     */
    public string $subsystem = '';

    /**
     * @var bool $always_apply
     * Force this Model to always run the `apply()` method after `create()`, `update()`, `delete()`, and `replace_all()`.
     */
    public bool $always_apply = false;

    /**
     * @var string $update_strategy
     * Define how this Model should update existing values in configuration. Set to `merge` to only update the values that
     * were actually changed in config, or set to `replace` to replace the entire Model object in config during updates.
     * In general, `replace` works well for $many enabled Models, and `merge` works well for non $many enabled Models or
     * for Models that share a $config_path with other Models.
     */
    public string $update_strategy = 'merge';

    /**
     * @var bool $skip_init
     * If set to `true`, this Model object will not be loaded from its internal representation during construction. This
     * is useful when you need to create a Model object to be used as a reference without trigger logic that would normally
     * occur by loading the object from its internal representation. This must be set as a parameter during object construction
     * to have an effect.
     */
    public bool $skip_init = false;

    /**
     * @var string $verbose_name
     * Sets the human-readable name for this Model. This value is primarily used where this Model is referenced in Forms
     * and the API documentation. By default, this property will attempt to be determined using the Model class name by
     * separating capitalized words in the class name with a space. In some cases, this may not be accurately determined
     * and can be manually set using this property if needed.
     */
    public string $verbose_name = '';

    /**
     * @var string $verbose_name_plural
     * Manually specifies the plural representation of $verbose_name. By default, this property will be set to $verbose_name
     * with `s` appended to the end, unless $verbose_name already ends in an `s`, in which `es` will be appended to the end
     * instead. In some cases, this may not be accurate; this property can be manually set using this property if needed.
     */

    public string $verbose_name_plural = '';

    /**
     * Values to assign when a Model object is first constructed.
     * @param mixed $id If specified, the internal object with this ID will be obtained as the Model object.
     * @param mixed|null $parent_id The ID of the parent object this object is nested under. This is only applicable
     * to Models with a `parent_model_class` assigned that is also a `many` Model.
     * @param mixed $data Allows objects to be obtained, created and/or updated using an array of
     * field-value pairs. If this array contains an `id` field, the internal object with this ID will be obtained as
     * the Model object first. Any additional Fields will be used to set the Model object's values.
     * @param mixed ...$options Additional arguments to provide to this Model. Any additional arguments provided will
     * merged into the Model's $data parameter. This can be used to set Model field values using parameters instead
     * of an array format via $data. Note: In the event that a field value is set in $data and $args, the value
     * found in $args will be used.
     */
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Throw an error if both $id and data are set.
        if (isset($id) and $data) {
            throw new ServerError(
                message: 'Model objects cannot be constructed with both an ID and representation data.',
                response_id: 'MODEL_WITH_ID_AND_REPRESENTATION',
            );
        }

        # Ensure the representation data is an array
        if (!is_array($data)) {
            throw new ValidationError(
                message: 'Representation data must be of type `array`.',
                response_id: 'MODEL_NON_ARRAY_REPRESENTATION_DATA',
            );
        }

        # Set the model cache exemption if it was not explicitly defined
        if (!isset($this->model_cache_exempt)) {
            $this->model_cache_exempt = (bool) $this->internal_callable;
        }

        # Check for known $options and map them to their Model properties
        $options = $this->set_construct_options($options);

        # Merge the remaining options into the $data array so they can be used to populate field values
        $data = array_merge($data, $options);

        # Obtain the Cache object for this Model if assigned
        $this->cache = $this->get_cache();

        # Obtain the current object from internal if an ID was given or this is not a `many` model.
        $this->construct_from_internal(id: $id, parent_id: $parent_id);

        # Use the representation to obtain or construct this object if present.
        if ($data) {
            $this->from_representation($data);
        }

        # Auto-set the verbose name and plural verbose name for this object if it was not specified by a parent class
        $this->verbose_name = $this->verbose_name ?: $this->generate_verbose_name();
        $this->verbose_name_plural = $this->verbose_name_plural ?: $this->generate_verbose_name_plural();

        # Setup Field properties assigned to this Model
        $this->setup_fields();

        # Set the default $related_objects ModelSet so it can be populated later
        $this->related_objects = new ModelSet();
    }

    /**
     * Magic method to serialize only the necessary properties of this Model object.
     * @return int[]|string[]
     */
    public function __sleep() {
        # Variables
        $excluded_properties = ['initial_object', 'client', 'parent_model', 'related_objects'];
        $properties = array_keys(get_object_vars($this));

        # Filter out excluded properties from the list of properties to serialize
        return array_filter($properties, function ($prop) use ($excluded_properties) {
            return !in_array($prop, $excluded_properties, true);
        });
    }

    /**
     * Creates an unlinked copy of this Model object.
     * @return Model The cloned Model object.
     */
    public function copy(): Model {
        # Create a copy of this object by serializing and unserializing it
        $copy = unserialize(serialize($this));

        # Reassign the parent Model object if it exists
        if ($this->parent_model) {
            $copy->parent_model = $this->parent_model;
        }

        return $copy;
    }

    /**
     * Checks for options passed in during object construction and maps known options to Model properties. Any
     * options that are not known options to the core Model will be returned so they can be merged into
     * the Model data during construction.
     * @param array $options The $options passed into the Model's __construct() method.
     * @return array An array of additional options that do not relate to core Model properties.
     */
    private function set_construct_options(array $options): array {
        # Set the async flag if given. Ensure this defaults to true.
        $this->async = $options['async'] ?? true;
        unset($options['async']);

        # Set the placement index if given and is a positive integer
        if (is_integer($options['placement']) and $options['placement'] >= 0) {
            $this->placement = $options['placement'];
            unset($options['placement']);
        }

        # Obtain a default $client Auth object for this Model object to use if one was not provided
        $this->client = $options['client'] ?: $this->get_default_client();
        unset($options['client']);

        # Obtain the skip_init flag if given. Ensure this defaults to false.
        $this->skip_init = $options['skip_init'] ?? false;
        unset($options['skip_init']);

        return $options;
    }

    /**
     * Attempts to construct this Model object by fetching an existing object from config/internally.
     * @param mixed $id The ID of the internal object to construct this object from
     */
    private function construct_from_internal(mixed $id = null, mixed $parent_id = null): void {
        # Do not try to load the object from internal if the skip_init flag is set
        if ($this->skip_init) {
            $this->id = $id;
            $this->parent_id = $parent_id;
            return;
        }

        # Obtain the parent Model if a parent_id was provided and this Model has a parent Model class assigned
        if (isset($parent_id) and $this->parent_model_class) {
            $this->parent_id = $parent_id;
            $this->get_parent_model();
        }
        # Only proceed if an ID was provided or this Model is not a $many enabled Model.
        if (isset($id) or !$this->many) {
            # Don't pull internal values unless there is a `config_path` or `internal_callable` defined.
            if ($this->config_path or $this->internal_callable) {
                $this->id = $id;
                $this->from_internal();
                $this->initial_object = $this->copy();
            }
        }
    }

    /**
     * Auto-generates a verbose name for this Model class using its class name.
     * @returns string The human-readable name for this Model class.
     */
    private function generate_verbose_name(): string {
        # Separate the class name's words with spaces, allow consecutive capital characters like 'DNS'
        $model_name = $this->get_class_shortname();
        return preg_replace('/(?<=[a-z])(?=[A-Z])|(?<=[A-Z])(?=[A-Z][a-z])/', ' ', $model_name);
    }

    /**
     * Auto-generates the plural version of this Model's $verbose_name for $many enabled Models
     * @returns string The human-readable name for this Model class in plural form.
     */
    private function generate_verbose_name_plural(): string {
        # Only add a plural suffix to the verbose name if this is a $many enabled Model
        if ($this->many) {
            # When the verbose name ends with 'y', remove the 'y' and append 'ies'.
            if (str_ends_with($this->verbose_name, 'y')) {
                return substr($this->verbose_name, 0, -1) . 'ies';
            }
            # When the verbose name ends with 's', append 'es' to the end.
            elseif (str_ends_with($this->verbose_name, 's')) {
                return $this->verbose_name . 'es';
            }
            # Otherwise, just append 's'
            else {
                return $this->verbose_name . 's';
            }
        }

        # If this is not a $many enabled Model, just use the $verbose_name
        return $this->verbose_name;
    }

    /**
     * Obtains a default $client Auth object for this Model object to use.
     * @returns Auth An Auth object that uses the default client username and IP address.
     */
    private function get_default_client(): Auth {
        $client = new Auth();
        $client->username = DEFAULT_CLIENT_USERNAME;
        $client->ip_address = DEFAULT_CLIENT_IP_ADDRESS;
        return $client;
    }

    /**
     * Obtains the Cache object of the class assigned in the $cache_class property.
     * @returns Cache|null Returns a new Cache object of the $cache_class property assigned or null if no $cache_class
     * property was assigned.
     */
    private function get_cache(): Cache|null {
        # Only continue if this Model has a $cache_class assigned
        if (!$this->cache_class) {
            return null;
        }

        # Format the class's FQN
        $class = "\\RESTAPI\\Caches\\$this->cache_class";

        # Throw an error if there is no cache class with this name
        if (!class_exists($class)) {
            throw new ServerError(
                message: "Model cannot be assigned non-existent \$cache_class `$class`.",
                response_id: 'MODEL_CACHE_CLASS_INVALID',
            );
        }

        # Otherwise, return the Cache object
        return new $class();
    }

    /**
     * Provides access to the ModelCache singleton instance.
     * @return ModelCache The ModelCache singleton instance.
     */
    public static function get_model_cache(): ModelCache {
        return ModelCache::get_instance();
    }

    /**
     * Clears the object cache for this Model class.
     */
    public static function clear_model_cache(): void {
        # Clear the object cache
        self::get_model_cache()::clear();
    }

    /**
     * Provides this Model's Field object properties relevant information like its field name and provides this Model
     * object as this Field's $context Model object.
     */
    private function setup_fields(): void {
        # Assign names and model context to each field in this Model
        foreach ($this->get_fields() as $field) {
            $this->$field->set_names($field);
            $this->$field->context = $this;
        }
    }

    /**
     * Retrieves the full config path including any assigned parent Model paths.
     */
    public function get_config_path(): string|null {
        # Do not allow this method to be called if a parent Model class is assigned, but the parent hasn't been obtained
        if ($this->parent_model_class and !$this->parent_model) {
            throw new ServerError(
                message: 'Model `get_config_path()` cannot be called until the parent Model has been constructed. ' .
                    'Run the `get_parent_model()` method first.',
                response_id: 'MODEL_CANNOT_GET_CONFIG_PATH_WITHOUT_PARENT_MODEL',
            );
        }

        # Format the config path accordingly when a `parent_model_class` is assigned
        if ($this->parent_model) {
            # Do not allow a parent Model to be set if this Model has an `internal_callable` assigned
            if ($this->internal_callable) {
                throw new ServerError(
                    message: 'Model ' .
                        get_called_class() .
                        " cannot assign `parent_model_class` $this->parent_model_class " .
                        'while also having an `internal_callable` assigned.',
                    response_id: 'MODEL_CANNOT_HAVE_PARENT_MODEL_CLASS_AND_INTERNAL_CALLABLE',
                );
            }

            # Do not allow the parent Model to be a child of any other model
            if ($this->parent_model->parent_model_class) {
                throw new ServerError(
                    message: 'Model ' .
                        get_called_class() .
                        " cannot assign `parent_model_class` $this->parent_model_class " .
                        "because itself has a `parent_model_class` {$this->parent_model->parent_model_class}",
                    response_id: 'MODEL_PARENT_MODEL_CLASS_CANNOT_HAVE_PARENT',
                );
            }

            # Do not allow the parent Model to be assigned if itself does not have a config path
            if (!$this->parent_model->config_path) {
                throw new ServerError(
                    message: 'Model ' .
                        get_called_class() .
                        " cannot assign `parent_model_class` $this->parent_model_class " .
                        'because it does not have a `config_path` set.',
                    response_id: 'MODEL_PARENT_MODEL_CLASS_MUST_HAVE_CONFIG_PATH',
                );
            }

            # Return a config path including the parent ID if the parent Model is a `many` model
            if ($this->parent_model->many) {
                return "{$this->parent_model->config_path}/$this->parent_id/$this->config_path";
            }

            # Otherwise, just prefix the parent Model's config path to this Model's config path
            return "{$this->parent_model->config_path}/$this->config_path";
        }

        # Just use this Model's config path if no parent Model is assigned
        return $this->config_path;
    }

    /**
     * Normalizes a config path by removing extra slashes and ensuring it is not empty.
     * @param string $path The config path to normalize. This is usually a string with '/' as separators.
     * @return string The normalized config path
     */
    public static function normalize_config_path(string $path): string {
        # Remove leading and trailing slashes, and duplicate slashes, if slashes are present
        if (str_contains($path, '/')) {
            $path = trim($path, '/');
            $path = preg_replace('/\/+/', '/', $path);
        }

        # Return the normalized config path
        return $path;
    }

    /**
     * Initialize the configuration array of a specific config path
     * @param $path string config path with '/' as separators
     */
    protected static function init_config(string $path): void {
        # Initialize the configuration array of a specified path
        config_init_path(self::normalize_config_path($path));
    }

    /**
     * Read the value of specific configuration section by path.
     * @param $path string config path with '/' as separators
     * @param $default mixed value to return if the path is not found
     * @returns mixed value at path or $default if the path does not exist or if the
     *          path keys an empty string and $default is non-null
     */
    public static function get_config(string $path, mixed $default = null): mixed {
        return config_get_path(self::normalize_config_path($path), $default);
    }

    /**
     * Set a value by path in the config.
     * @param $path string path with '/' separators
     * @param $value mixed value to set
     * @param $default mixed value to return if the path is not found
     * @returns mixed $val or $default if the path prefix does not exist
     */
    public static function set_config(string $path, mixed $value, mixed $default = null): mixed {
        # Clear the object cache for this Model class since config is being modified
        self::clear_model_cache();
        return config_set_path(self::normalize_config_path($path), $value, $default);
    }

    /**
     * Merges changed Model field values into the config. This is similar to `set_config()` but instead of replacing
     * the entire config path with a set value, it only sets Field's known to this Model. Any Fields in the stored
     * object that are not defined in a Field assigned to this Model will be left unchanged.
     * @param $path string The Model config path including any Model ID
     */
    public function merge_config(string $path): void {
        # Remove leading slashes and extra slashes
        $path = self::normalize_config_path($path);

        # Loop through each field known to this Model
        foreach ($this->get_fields() as $field) {
            # Determine the field path
            if ($this->$field->internal_namespace) {
                $field_path = "$path/{$this->$field->internal_namespace}/{$this->$field->internal_name}";
            } else {
                $field_path = "$path/{$this->$field->internal_name}";
            }

            # Do not merge `representation_only` Fields
            if ($this->$field->representation_only) {
                continue;
            }

            # Remove config values to Fields that are `null`
            if ($this->$field->to_internal() === null) {
                $this->del_config(path: $field_path);
                continue;
            }

            # Otherwise, set the value normally
            $this->set_config(path: $field_path, value: $this->$field->to_internal());
        }
    }

    /**
     * Delete a key from the configuration
     * @param $path string path with '/' separators
     * @returns array copy of the removed value or null
     */
    public static function del_config(string $path): mixed {
        # Clear the object cache for this Model class since config is being modified
        self::clear_model_cache();
        return config_del_path(self::normalize_config_path($path));
    }

    /**
     * Checks if a specific config value is set. Toggle values are often set when
     * enabled, or missing when they are not. This check is better suited for
     * pfSense configurations than isset().
     * @param $path string path with '/' separators
     * @param $enable_key string an optional alternative key value for the enable key
     * @returns bool true if $enable_key exists in the array at $path, and has a
     * non-null value, otherwise false.
     */
    public static function is_config_enabled(string $path, string $enable_key = 'enable'): bool {
        return config_path_enabled(self::normalize_config_path($path), $enable_key);
    }

    /**
     * Write configuration changes to the config file
     * @param string $change_note The message to write to the change log.
     * @param int $attempts The number of write attempts to make before giving up in the event that a lock is present.
     */
    final public function write_config(string $change_note, int $attempts = 60): void {
        # Ensure there is not an API config lock, loop until the lock is released
        foreach (range(0, $attempts) as $attempt) {
            # Only write to the config if there is not an active lock
            if (!file_exists(self::WRITE_LOCK_FILE)) {
                # Create a lock on API config writes while we write changes
                file_put_contents(self::WRITE_LOCK_FILE, microtime());

                # Start a temporary session to write the config that contains the user's username in the config history
                session_start();
                $_SESSION['Username'] = $this->client->username;
                $_SERVER['REMOTE_ADDR'] = $this->client->ip_address;
                write_config(sprintf(gettext(' ' . $change_note)));
                session_destroy();

                # Clear the object cache for this Model class since config has been modified
                self::clear_model_cache();

                # If a subsystem is specified for this Model, mark it as dirty
                if ($this->subsystem) {
                    mark_subsystem_dirty($this->subsystem);
                }

                # Remove the temporary session and write lock
                unlink(self::WRITE_LOCK_FILE);
                return;
            }

            # Delay attempts by 1 second
            sleep(1);
        }

        # If the lock was not released after the attempts, throw an error
        throw new ServiceUnavailableError(
            message: 'Failed to write to the configuration file after multiple attempts.',
            response_id: 'MODEL_WRITE_CONFIG_LOCKED',
        );
    }

    /**
     * Reloads the configuration to include any changes that may have occurred since the last config reload
     * @param bool $force_parse Force an entire reparse of the config.xml file instead of the cached config.
     */
    public static function reload_config(bool $force_parse = false): void {
        global $config;

        # Clear the object cache for all Model classes since config is being reloaded
        self::clear_model_cache();

        $config = parse_config(parse: $force_parse);
    }

    /**
     * Default method for obtaining the next ID from the pfSense configuration. This searches for the next index ID for
     * the model in the pfSense configuration and requires the model's `config_path` and `many` properties to be set.
     * For Models that do not use a `config_path` or does not use config index IDs, this method can be overridden by
     * the child model class to add custom processes to obtain the next available ID.
     * @throws ServerError When the path leads to an existing non-empty, non-sequential array value
     * @returns string|int the next array key (ID) for the config path specified.
     */
    public function get_next_id(): string|int {
        # Since this references config IDs, this method requires `config_path` to be set. Throw an error if it's not.
        if (!$this->config_path) {
            throw new ServerError(
                message: "Model cannot use default method to obtain next available ID without 'config_path'",
                response_id: 'MODEL_NEXT_ID_WITHOUT_CONFIG_PATH',
            );
        }

        # IDs are only applicable for `many` Models. Throw an error if `many` is not set.
        if (!$this->many) {
            throw new ServerError(
                message: "Models cannot obtain next ID when 'many' is not enabled.",
                response_id: 'MODEL_NEXT_ID_WITHOUT_MANY',
            );
        }

        # If this Model has a parent Model assigned, get the parent Model object so we can accurately determine the ID
        if ($this->parent_model_class) {
            $this->get_parent_model();
        }

        # First, get the configuration at the specific path
        $conf = $this->get_config($this->get_config_path(), []);

        # Ensure config is an array
        if (is_array($conf) and array_key_exists(0, $conf)) {
            return count($conf);
        }
        # If the value is empty, start the array at ID 0
        elseif (empty($conf)) {
            return 0;
        }
        # Otherwise, this action could be dangerous, throw an error
        else {
            throw new ServerError(
                message: "Model failed to obtain the next available ID because the config path leads to an existing, 
                non-indexed array value.",
                response_id: 'MODEL_DANGEROUS_CONFIG_PATH',
            );
        }
    }

    /**
     * Converts this Model object into it's internal pfSense configuration form.
     * @return array|string The internal value suitable for writing to the pfSense configuration.
     */
    public function to_internal(): array|string {
        # Variables
        $internal_object = [];

        # Loop through each of this Model's Fields and add its value to a serializable array
        foreach ($this->get_fields() as $field) {
            # Don't add this field if it's value is `null`. This is how values are unset.
            if (is_null($this->$field->value) or is_null($this->$field->to_internal())) {
                continue;
            }

            # Init the namespace for this field if a namespace is specified and does not exist yet.
            if (
                $this->$field->internal_namespace and
                !array_key_exists($this->$field->internal_namespace, $internal_object)
            ) {
                $internal_object[$this->$field->internal_namespace] = [];
            }

            # Nest this field under the namespace if specified and continue through the loop.
            if ($this->$field->internal_namespace) {
                $internal_object[$this->$field->internal_namespace][
                    $this->$field->internal_name
                ] = $this->$field->to_internal();
                continue;
            }

            # Otherwise, just store the field directly to the object
            $internal_object[$this->$field->internal_name] = $this->$field->to_internal();
        }

        return $internal_object;
    }

    /**
     * Recursively obtains all internal objects for this Model from all parent objects in config. This method is only
     * applicable to Models with a `parent_model_class` assigned.
     * @return array An array containing the internal object and its context information needed to reconstruct the
     * Model object later. This array will always be structured as the following:
     * [
     *   "_parent_model" => Model, // The parent Model object
     *   "_id" => string|int,      // The ID of the internal object
     *   "_internal_object" => array, // The internal object data
     * ]
     */
    protected function get_internal_objects_from_all_parents(): array {
        /**@var Model $parent_model_class The parent Model class assigned to this Model. */
        $parent_model_class = "\\RESTAPI\\Models\\$this->parent_model_class";
        $parent_modelset = $parent_model_class::read_all();
        $internal_objects = [];

        # Check for internal objects from all parents
        foreach ($parent_modelset->model_objects as $parent_model) {
            # Obtain the internal config for
            $child_configs = $this->get_config("$parent_model->config_path/$parent_model->id/$this->config_path", []);

            foreach ($child_configs as $child_id => $child_config) {
                # Retain internal object context information needed to reconstruct the Model object later
                $internal_objects[] = [
                    '_parent_model' => $parent_model,
                    '_id' => $child_id,
                    '_internal_object' => $child_config,
                ];
            }
        }

        return $internal_objects;
    }

    /**
     * Obtains all internal objects for this Model. When a `config_path` is specified, this method will obtain the
     * internal objects directly from config. When an `internal_callable` is assigned, this method will return
     * the output of the assigned callable.
     * @param bool $from_all_parents When set to `true`, this method will obtain internal objects from all parent
     * objects in config. This is only applicable to Models with a `parent_model_class` assigned.
     * @throws ServerError When neither a `config_path` nor a `internal_callable` are assigned to this model, OR both a
     * `config_path` and a `internal_callable` are assigned to this model
     * @return array The array of internal objects without any additional processing performed.
     */
    /**
     * @param bool $from_all_parents Whether to obtain objects from all parent Models.
     * @param int $limit Pagination hint for internal callables (0 = no limit). When non-zero, callables that accept
     *                    a $limit parameter (e.g. log-backed Models) can use it to avoid loading entire datasets
     *                    into memory. Callables that do not accept a $limit parameter are called without it.
     */
    public function get_internal_objects(bool $from_all_parents = false, int $limit = 0): array {
        global $mock_internal_objects;

        # Throw an error if both `config_path` and `internal_callable` are set.
        if ($this->config_path and $this->internal_callable) {
            throw new ServerError(
                message: 'Models cannot define both a config path and an internal callable.',
                response_id: 'MODEL_WITH_CONFIG_PATH',
            );
        }
        # Use mock data if the $mock_internal_objects global is set
        elseif ($mock_internal_objects) {
            $internal_objects = $mock_internal_objects;
        }
        # Obtain all internal objects from all parents if a parent Model class is assigned and all parents is requested
        elseif ($from_all_parents and $this->parent_model_class) {
            $internal_objects = $this->get_internal_objects_from_all_parents();
        }
        # Obtain the internal objects from the config path if specified
        elseif ($this->config_path) {
            $internal_objects = $this->get_config($this->get_config_path(), []);
        }
        # Obtain the internal objects by calling the `internal_callable` if specified
        elseif ($this->internal_callable) {
            $callable = $this->internal_callable;

            # Forward the pagination limit to callables that accept it. This allows log-backed
            # Models to stop reading early instead of loading entire log files into memory.
            # Callables that don't accept a $limit parameter continue to work unchanged.
            # Reflection results are cached per class+callable to avoid repeated introspection.
            if ($limit > 0 && $this->callable_accepts_limit($callable)) {
                $internal_objects = $this->$callable(limit: $limit);
            } else {
                $internal_objects = $this->$callable();
            }
        }
        # Otherwise, throw an error. Either a `config_path` or an `internal_callable` is required.
        else {
            throw new ServerError(
                message: "Model requires a 'config_path' or 'internal_callable' value to be defined before
                    obtaining internal objects.",
                response_id: 'MODEL_WITH_NO_INTERNAL_METHOD',
            );
        }

        return $internal_objects;
    }

    /**
     * Checks whether an internal callable method accepts a `limit` parameter. Results are cached
     * per class+callable combination to avoid repeated reflection overhead on paginated requests.
     * @param string $callable The method name to check.
     * @return bool True if the method has a parameter named 'limit'.
     */
    private function callable_accepts_limit(string $callable): bool {
        static $cache = [];
        $key = static::class . '::' . $callable;

        if (!isset($cache[$key])) {
            $accepts = false;
            $ref = new \ReflectionMethod($this, $callable);
            foreach ($ref->getParameters() as $param) {
                if ($param->getName() === 'limit') {
                    $accepts = true;
                    break;
                }
            }
            $cache[$key] = $accepts;
        }

        return $cache[$key];
    }

    /**
     * Obtain this Model object from the internal pfSense configuration by object ID. If the specified ID exists in
     * config, this Model object will be overwritten with the contents of that object.
     * @throws ServerError When this Model does not have a `config_path` set.
     * @throws NotFoundError When an object with the specified $id does not exist.
     */
    public function from_internal(): void {
        # Require a `parent_id` if a `many` parent Model is assigned
        if ($this->parent_model_class and $this->parent_model->many and !isset($this->parent_id)) {
            throw new ServerError(
                message: "A parent ID is required when refreshing objects from config on model with a 'many' parent.",
                response_id: 'MODEL_PARENT_ID_REQUIRED_FOR_REFRESH',
            );
        }

        # Require an $id if `many` is set
        if ($this->many and !isset($this->id)) {
            throw new ServerError(
                message: "An ID is required when refreshing objects from config on model with 'many'.",
                response_id: 'MODEL_ID_REQUIRED_FOR_REFRESH',
            );
        }

        # Get the internal object(s) from config and keep track of Fields with custom from internal methods
        $internal_objects = $this->get_internal_objects();

        # Require there to be an internal object with this ID, throw a not found error otherwise
        if ($this->many and !array_key_exists($this->id, $internal_objects)) {
            throw new NotFoundError(
                message: "Object with ID '$this->id' does not exist.",
                response_id: 'MODEL_OBJECT_NOT_FOUND',
            );
        }

        # When `many` is enabled, obtain the object with our ID. Otherwise, just assign the direct object
        $internal_object = $this->many ? $internal_objects[$this->id] : $internal_objects;
        $this->from_internal_object($internal_object);
    }

    /**
     * Converts an internal object to this Model object's representation. This method is used to convert the internal
     * object obtained from the pfSense configuration to the representation values used by this Model object.
     * @param mixed $internal_object The internal object obtained from the pfSense configuration. This should be an
     * associative array, in the event that it's not (uninitialized configuration), it will become an empty array.
     */
    public function from_internal_object(mixed $internal_object): void {
        # Variables
        $custom_from_internal_fields = [];

        # Default to an empty array if the internal object is not an array
        if (!is_array($internal_object)) {
            $internal_object = [];
        }

        # Loop through each of this Model's Fields and convert the internal value to the representation value
        foreach ($this->get_fields() as $field) {
            # Check if a custom from internal method was implemented for this model field.
            if (method_exists($this, "from_internal_$field")) {
                # Capture Fields with custom from internal methods. We will run them after this loop completes.
                $custom_from_internal_fields[] = $field;
            }

            # Check the internal object for this field directly and convert it's value from internal to representation
            if (!$this->$field->internal_namespace) {
                $this->$field->from_internal($internal_object[$this->$field->internal_name]);
            }

            # Check if this field has a namespace and check if the internal object has the namespace
            if (
                $this->$field->internal_namespace and
                array_key_exists($this->$field->internal_namespace, $internal_object)
            ) {
                # Ensure the internal namespace is an array if the namespace is empty
                if (!$internal_object[$this->$field->internal_namespace]) {
                    $internal_object[$this->$field->internal_namespace] = [];
                }

                # Check the namespace for the field and convert it's value from internal to representation
                if (
                    array_key_exists($this->$field->internal_name, $internal_object[$this->$field->internal_namespace])
                ) {
                    $this->$field->from_internal(
                        $internal_object[$this->$field->internal_namespace][$this->$field->internal_name],
                    );
                }
            }
        }

        # Run any custom from internal methods found during the loop.
        foreach ($custom_from_internal_fields as $field) {
            $custom_from_internal_method = "from_internal_$field";
            $this->$field->value = $this->$custom_from_internal_method($this->$field->value);
        }
    }

    /**
     * Creates a serializable array representation of this object. This array is suitable for content
     * serialization such as JSON, YAML, etc.
     * @return array The serializable array representation of this object.
     */
    final public function to_representation(): array {
        # Variables
        $representation = [];
        $pkg_config = RESTAPI\Models\RESTAPISettings::get_pkg_config();
        $hateos_enabled = $pkg_config['hateoas'] === 'enabled';
        $expose_sensitive_fields = $pkg_config['expose_sensitive_fields'] === 'enabled';

        # Embed this object's parent ID if set
        if (isset($this->parent_id)) {
            $representation['parent_id'] = $this->parent_id;
        }

        # Embed this object's ID if set
        if (isset($this->id)) {
            $representation['id'] = $this->id;
        }

        # Loop through each of this Model's Fields and add its value to a serializable array
        foreach ($this->get_fields() as $field) {
            # Skip adding this field if it's hidden
            if ($this->is_field_hidden($field)) {
                continue;
            }

            # Add this field's representation value to the representation array
            $representation[$field] = $this->$field->value;
        }

        # Append any related HATEOAS links if HATEOAS is enabled
        if ($hateos_enabled) {
            $representation += $this->get_related_links()->to_representation();
        }

        return $representation;
    }

    /**
     * Loads the Model object using an array representation of the object.
     * @param array $data The array of key-value pairs to use when loading the Model object from
     * representation.
     * @param mixed ...$options Additional arguments to provide to this Model. Any additional arguments provided will
     * merged into the Model's $data parameter. This can be used to set Model field values using parameters instead
     * of an array format via $data. Note: In the event that a field value is set in $data and $args, the value
     * found in $args will be used.
     */
    final public function from_representation(array $data = [], mixed ...$options) {
        # Merge additional arguments into the $data array
        $data = array_merge($data, $options);

        # Set the parent ID if provided and obtain the parent model object if applicable
        if (array_key_exists('parent_id', $data) and $this->parent_model_class) {
            $this->parent_id = $data['parent_id'];
            $this->get_parent_model();
        }

        # When an ID is provided, fetch this objects from the internal configuration using the ID.
        if (array_key_exists('id', $data)) {
            # Extract the ID from the data
            $this->id = $data['id'];

            # Obtain the object from its internal form
            $this->from_internal();
            $this->initial_object = $this->copy();
        }

        # Loop through each field in this Model and assign their values using the `representation_data`.
        foreach ($this->get_fields() as $field) {
            # Ensure a value for this field is in the representation data before continuing.
            if (array_key_exists($field, $data)) {
                $this->$field->value = $data[$field];
            }
        }
    }

    /**
     * Converts this Model object to a PHP array representation of its OpenAPI schema. This is used when
     * auto-generating API documentation. This method can be extended to add additional options to the OpenAPI
     * schema when necessary
     * @link https://swagger.io/docs/specification/data-models/
     * @return array A PHP array containing this Model as an OpenAPI schema.
     */
    public function to_openapi_schema(): array {
        # Assign primitive schema attributes
        $openapi_schema = ['type' => 'object', 'properties' => []];

        # Loop through each Field in this Model and obtain its OpenAPI property config
        foreach ($this->get_fields() as $field) {
            # Generate and add the OpenAPI property from the Field object
            $openapi_schema['properties'][$field] = $this->$field->to_openapi_property();
        }

        return $openapi_schema;
    }

    /**
     * Obtains the parent of this Model object if a parent Model is assigned.
     * @return Model|null The parent Model object with the provided `parent_id` or `null` if no parent Model is set.
     * @throws NotFoundError If the parent Model object does not exist.
     */
    public function get_parent_model(): Model|null {
        # If a parent Model is assigned, create the parent object
        if ($this->parent_model_class) {
            # Set the FQN parent class name
            $parent_model_class = "\\RESTAPI\\Models\\$this->parent_model_class";

            # Throw an error if the given parent ID does not exist
            try {
                $this->parent_model = new $parent_model_class(id: $this->parent_id);
            } catch (NotFoundError) {
                # Throw a NotFoundError but change some of the verbage
                throw new NotFoundError(
                    message: "Parent object with ID '$this->parent_id' does not exist.",
                    response_id: 'MODEL_PARENT_OBJECT_NOT_FOUND',
                );
            }

            # Set the parent Model object and return it
            return $this->parent_model;
        }

        return null;
    }

    /**
     * Checks if the parent Model class assigned to this Model class is a $many enabled Model.
     * @return bool|null Returns true if a $parent_model_class is assigned to this Model and the parent Model has
     * $many enabled, false if a $parent_model_class is assigned to this Model and the parent Model does not have
     * $many enabled, or null if no $parent_model_class is assigned to this Model.
     */
    public function is_parent_model_many(): bool|null {
        # If a parent Model is assigned, create the parent object
        if ($this->parent_model_class) {
            # Set the FQN parent class name
            $parent_model_class = "\\RESTAPI\\Models\\$this->parent_model_class";
            $parent_model = new $parent_model_class();
            return $parent_model->many;
        }

        return null;
    }

    /**
     * Determines if a given Model field should be hidden in responses.
     * @param string $field The field name to check for sensitivity.
     * @return bool Returns true if the field is sensitive, false if it is not.
     */
    private function is_field_hidden(string $field): bool {
        # Variable
        $pkg_config = RESTAPI\Models\RESTAPISettings::get_pkg_config();
        $expose_sensitive_fields = $pkg_config['expose_sensitive_fields'] === 'enabled';
        $overridden_sensitive_fields = explode(',', $pkg_config['override_sensitive_fields']);
        $model_field_mapping = "{$this->get_class_shortname()}:$field";

        # Field is hidden if it is 'write_only'
        if ($this->$field->write_only) {
            return true;
        }

        # Field is not hidden if the sensitive property is not set
        if (!$this->$field->sensitive) {
            return false;
        }

        # Field is not hidden if the 'expose_sensitive_fields' setting is enabled
        if ($expose_sensitive_fields) {
            return false;
        }

        # Field is not hidden if the field is in the 'override_sensitive_fields' setting
        if (in_array($model_field_mapping, $overridden_sensitive_fields)) {
            return false;
        }

        # Field is hidden if none of the above conditions are met
        return true;
    }

    /**
     * Determines if this Model applies changes immediately or if they must applied manually after changes are made.
     * This is intended to be used to more accurately determine the 'Applies immediately' value in the API documentation
     * to prevent misleading information.
     * @return bool|null Returns true if this Model applies changes immediately, false if changes must be applied
     * manually, or null if this does not apply to this Model.
     */
    public function does_apply_immediately(): ?bool {
        # If this Model does not have a `config_path` set, this is not relevant. Return null.
        if (!$this->config_path) {
            return null;
        }

        # If this Model has the `always_apply` property set, changes always apply immediately. Return true.
        if ($this->always_apply) {
            return true;
        }

        # Otherwise, changes must be applied manually. Return false.
        return false;
    }

    /**
     * Populates an array of field names for this model.
     * @param bool $required_only Only obtain required Fields
     * @return array An array of field names for this model.
     */
    final public function get_fields(bool $required_only = false): array {
        # Variables
        $fields = [];

        # Loop through each var in this object and extract the objects with a base class of RESTAPI\Core\Field.
        foreach (get_object_vars($this) as $var => $value) {
            # Skip this var if it's not an object
            if (!is_object($value)) {
                continue;
            }

            # Skip this item if it is not child class of the RESTAPI\Core\Field class
            if (!in_array('RESTAPI\Core\Field', class_parents($value))) {
                continue;
            }

            # Skip this item if we are only gathering required Fields and this isn't a required Field
            if ($required_only and !$this->$var->required) {
                continue;
            }

            # Assign the name of this field and context to this Field object
            $this->$var->name = $var;
            $this->$var->internal_name = $this->$var->internal_name ?: $this->$var->name;
            $this->$var->context = $this;

            # Otherwise, this is a valid Field object. Include it.
            $fields[] = $var;
        }

        return $fields;
    }

    /**
     * Checks if the deletion of this object would put the number of objects for this Model below the $many_minimum value.
     * @throws ForbiddenError When a new deletion is not allowed because it will put the number of objects below
     * the $many_minimum value.
     */
    public function check_many_minimum(): void {
        # Only perform this check if this is a $many enabled Model with a $many_minimum value assigned
        if (!$this->many or !$this->many_minimum) {
            return;
        }

        # Obtain the current number of stored objects for this Model
        $total_objects = count($this->get_internal_objects());

        # Throw a forbidden error if we are already at or under the minimum objects allowed
        if ($total_objects <= $this->many_minimum) {
            throw new ForbiddenError(
                message: 'Object deletion denied. Deleting this object would result in the number of existing objects ' .
                    "falling below the minimum number allowed for this Model, $this->many_minimum.",
                response_id: 'MODEL_MANY_MINIMUM_REACHED',
            );
        }
    }

    /**
     * Checks if the creation of a new object would put the number of objects for this Model over the $many_maximum value.
     * @throws ForbiddenError When a new creation is not allowed because it will put the number of objects over
     * the $many_maximum value.
     */
    public function check_many_maximum(): void {
        # Only perform this check if this is a $many enabled Model with a $many_maximum value assigned
        if (!$this->many or !$this->many_maximum) {
            return;
        }

        # Obtain the current number of stored objects for this Model
        $total_objects = count($this->get_internal_objects());

        # Throw a forbidden error if we are already at or over the maximum objects allowed
        if ($total_objects >= $this->many_maximum) {
            throw new ForbiddenError(
                message: 'Object creation denied. Creating this object would result in the number of existing objects ' .
                    "increasing beyond the maximum number allowed for this Model, $this->many_maximum.",
                response_id: 'MODEL_MANY_MAXIMUM_REACHED',
            );
        }
    }

    /**
     * Checks that any Fields which must be unique together are unique from all other existing objects for this Model.
     * @param ModelSet|null $modelset Sets a specific ModelSet to use when validations require the use of all existing
     * objects for this Model. The only current use case for this is in \RESTAPI\Fields\NestedModelField where we need
     * to validate many objects before they are created.
     * @throws ServerError When the Fields specified in `unique_together_fields` are not Field objects known to this
     * Model.
     * @throws ValidationError When the unique together Fields are not unique and are present in another Model object.
     */
    private function check_unique_together(?ModelSet $modelset = null): void {
        # Skip this validation for non `many` Models or Models without unique together Fields
        if (!$this->many or !$this->unique_together_fields) {
            return;
        }

        # Throw an error if Fields were provided that are not known to the Model
        foreach ($this->unique_together_fields as $unique_together_field) {
            if (!in_array($unique_together_field, $this->get_fields())) {
                throw new RESTAPI\Responses\ServerError(
                    message: "Unknown Model field `$unique_together_field` specified in `unique_together_fields`",
                    response_id: 'MODEL_UNIQUE_TOGETHER_FIELDS_WITH_UNKNOWN_FIELD',
                );
            }
        }

        # Use the $modelset if provided, otherwise obtain a ModelSet of all existing objects for this Model
        $modelset = $modelset ?: $this->query(parent_id: $this->parent_id);

        # Use this variable to keep track of query parameters to use when checking uniqueness
        $query_params = [];

        # Exclude this Model object from the query if this Model has an ID
        if (isset($this->id)) {
            $query_params = ['id__except' => $this->id];
        }

        # Format the query parameters as an associative array
        foreach ($this->unique_together_fields as $field) {
            $query_params[$field] = $this->$field->value;
        }

        # Query for existing objects with the same Field values
        $model_query = $modelset->query(query_params: $query_params);

        # If any existing Models were found matching the query, Fields are not unique together, throw an error
        if ($model_query->model_objects) {
            $unique_fields_string = implode(', ', $this->unique_together_fields);
            $in_use_by_id = $model_query->model_objects[0]->id;
            throw new ValidationError(
                message: "Fields [$unique_fields_string] together must be unique from all other objects of its kind. " .
                    "Values in use by object with ID `$in_use_by_id`.",
                response_id: 'MODEL_FIELDS_NOT_UNIQUE_TOGETHER',
            );
        }
    }

    /**
     * Allows child classes to define extra validation for their Models. This can be used to build complex validations
     * that check conditions between multiple Model fields.
     */
    public function validate_extra(): void {}

    /**
     * Performs a full validation on the API model. This includes checking that required packages are installed and
     * validating all assigned Fields. Note: Fields are validated in the order they are defined in the class.
     * @param bool $requires_id Whether the `id` field should be validated in this request. This is typically only
     * necessary for update and delete actions.
     * @param bool $create_id If true, a new ID will be created for this object if this is a $many Model. This is
     * primarily used for creations.
     * @param bool $only_id Only validate the `id` field. This is primarily used for deletions.
     * @param bool $skip_parent Skip validating the assigned parent ID. This only applies to Models that have a
     * `parent_model_class` value assigned. In most cases, this should not be changed from the default. This is
     * only intended for nested Model validation.
     * @param ModelSet|null $modelset Sets a specific ModelSet to use when validations require the use of all existing
     * objects for this Model. The only current use case for this is in \RESTAPI\Fields\NestedModelField where we need
     * to validate many objects before they are created.
     * @return bool `true` if the model's data is valid.
     */
    final public function validate(
        bool $requires_id = false,
        bool $create_id = false,
        bool $only_id = false,
        bool $skip_parent = false,
        ?ModelSet $modelset = null,
    ): bool {
        # When an ID is required and this is a `many` model, check that an ID is set.
        if ($requires_id and $this->many and !isset($this->id)) {
            throw new ValidationError(message: "Field 'id' is required.", response_id: 'MODEL_REQUIRES_ID');
        }

        # Validate `parent_id` field if a parent model class is assigned and it was not requested we skip this step.
        if ($this->parent_model_class and !$skip_parent) {
            # Require a parent ID if the parent model is a `many` model
            if ($this->is_parent_model_many() and !isset($this->parent_id)) {
                throw new ValidationError(
                    message: 'Field `parent_id` is required.',
                    response_id: 'MODEL_PARENT_ID_REQUIRED',
                );
            }

            # Obtain the exact parent Model object
            $this->get_parent_model();
        }

        # Create an ID for this object if requested
        if ($create_id and $this->config_path and $this->many) {
            $this->id = $this->get_next_id();
        }

        # Check for required packages
        $this->check_packages();

        # Don't validate field values if `only_id` is enabled
        if ($only_id) {
            return true;
        }

        # Use a variable to keep track of Fields with custom validate methods
        $custom_validate_fields = [];

        # For 'many' Models, capture all current Models in a ModelSet if one was not already given
        if ($this->many and !$modelset) {
            $modelset = $this->query(parent_id: $this->parent_id);
        }

        # Loop through each of this object's assigned Fields and validate them.
        foreach ($this->get_fields() as $field) {
            # Assign this object as the Fields parent Model context
            $this->$field->context = $this;

            # Ensure this field's internal name is unique to its namespace
            $this->is_fields_internal_name_unique($field);

            # Run validation on this field
            $this->$field->validate(modelset: $modelset);

            # Check if a custom validation method was implemented for this model field. Skip if null.
            if (method_exists($this, "validate_$field") and !is_null($this->$field->value)) {
                # Store the custom validation field so we can call it afterward
                $custom_validate_fields[] = $field;
            }
        }

        # Loop through Fields that had custom validation methods and call those methods.
        # Running these after the initial validation loop above allows all Fields to be available to these methods.
        foreach ($custom_validate_fields as $field) {
            # Format the name of the validate method for this field so we can call the function by variable
            $custom_validate_method = "validate_$field";

            # Validate each value individually for `many` Fields
            if ($this->$field->many) {
                $value_array = [];
                foreach ($this->$field->value as $value) {
                    $value_array[] = $this->$custom_validate_method($value);
                }
                $this->$field->value = $value_array;
            }
            # Otherwise, just validate the primitive value
            else {
                $this->$field->value = $this->$custom_validate_method($this->$field->value);
            }
        }

        # Validate unique together Fields
        $this->check_unique_together(modelset: $modelset);

        # Run the `validate_extra()` method for this Model to include any custom validation for the entire Model.
        $this->validate_extra();

        return true;
    }

    /**
     * Checks to ensure each of this Model's Field object has an internal name that is unique to it's namespace.
     * @param string $field The name of the field to check for a unique internal name
     * @throws ServerError When this Model contains a Field object with an internal name that is not unique to it's
     * namespace.
     */
    private function is_fields_internal_name_unique(string $field): void {
        # Loop to compare each field's names against each other to ensure there are no conflicts
        foreach ($this->get_fields() as $eval_field) {
            # Ignore if we are evaluating the same field, they will always have the same names.
            if ($field === $eval_field) {
                continue;
            }

            # Throw an error if the namespace and the internal name are the same. These must be unique.
            if ($this->$field->internal_namespace === $this->$eval_field->internal_namespace) {
                if ($this->$field->internal_name === $this->$eval_field->internal_name) {
                    throw new ServerError(
                        message: "Model field internal names must be unique to it's namespace.",
                        response_id: 'MODEL_FIELDS_WITH_CONFLICTING_INTERNAL_NAMES',
                    );
                }
            }
        }
    }

    /**
     * Obtains external Models that reference this Model. This method checks Field objects assigned to this Model object
     * for the `referenced_by` property. Any Model objects found that reference this Model object will be returned in
     * a ModelSet object.
     * @return ModelSet A ModelSet containing any Model objects that reference this Model object.
     */
    public function get_model_references(): ModelSet {
        # Variables
        $references = new ModelSet();

        # Loop through each field in this Model object and obtain any Model objects that reference the field
        foreach ($this->get_fields() as $field) {
            $references->model_objects = $references->model_objects + $this->$field->is_referenced_by()->model_objects;
        }

        return $references;
    }

    /**
     * Obtains the Endpoint this Model class is assigned to.
     * @param bool $many Set to true to obtain the $many enabled Endpoint for this Model, set to false to obtain the
     * Endpoint for this Model that does not have $many enabled.
     * @return Endpoint|null The Endpoint object that has this Model class assigned or null if none was found
     * @throws ServerError When no Endpoint could be found for this Model.
     */
    public function get_related_endpoint(bool $many): Endpoint|null {
        # Obtain the shortname for this class
        $model_name = $this->get_class_shortname();

        # Loop through each Endpoint class
        foreach (get_classes_from_namespace('\\RESTAPI\\Endpoints\\') as $endpoint_class) {
            # Construct the Endpoint object
            $endpoint = new $endpoint_class();

            # If the $many values and $model_name values match, we've found our endpoint. Return the object.
            if ($many === $endpoint->many and $model_name === $endpoint->model_name) {
                return $endpoint;
            }
        }

        return null;
    }

    /**
     * Obtains a HAL resource link to a specific Model object.
     * @param Model $model The Model object to obtain a link for.
     * @param string $rel The HAL resource link name to assign this ResourceLink object
     * @param bool $use_namespace Set to true to prefix this $rel with this project's HAL prefix value. This should
     * be used for non-standard rel values.
     * @return ResourceLink The ResourceLink object containing the links and attributes of the given $model.
     */
    public static function get_related_link(
        Model $model,
        string $rel,
        string $method = 'GET',
        bool $use_namespace = true,
    ): ResourceLink {
        # Get the endpoint associated with this object
        $related_url = $model->get_related_endpoint(many: false)->url;

        # Determine the URL with associated query for this link if it is a GET or DELETE method link
        if ($model->many and in_array($method, ['GET', 'DELETE'])) {
            # Add the parent ID to the URL if present
            if (isset($model->parent_id)) {
                $related_url .= "?parent_id=$model->parent_id&id=" . $model->id;
            } else {
                $related_url .= '?id=' . $model->id;
            }
        }

        # Generate a verbose title that makes sense given the context of this model
        $title = $model->many ? "$model->verbose_name: $model->id" : $model->verbose_name;
        return new ResourceLink(
            rel: $rel,
            href: $related_url,
            method: $method,
            use_namespace: $use_namespace,
            id: $model->id,
            parent_id: $model->parent_id,
            title: $title,
        );
    }

    /**
     * Obtains REST API links to resources related to this Model object.
     * @return ResourceLinkSet A ResourceLinkSet containing REST API links to related Model objects.
     */
    public function get_related_links(): ResourceLinkSet {
        # Variables
        $linkset = new ResourceLinkSet();
        $endpoint = $this->get_related_endpoint(many: false);

        # This Model does not support related links if it doesn't have a non-many enabled Endpoint
        if (!$endpoint) {
            return $linkset;
        }

        # Add a 'self' link if this Model supports single reads
        if (in_array('GET', $endpoint->request_method_options)) {
            $linkset->resource_links[] = $this->get_related_link($this, 'self', use_namespace: false);
        }
        # Add a 'delete' link if this Model supports updates
        if (in_array('PATCH', $endpoint->request_method_options)) {
            $linkset->resource_links[] = $this->get_related_link(
                $this,
                rel: 'update',
                method: 'PATCH',
                use_namespace: false,
            );
        }
        # Add a 'delete' link if this Model supports deletions
        if (in_array('DELETE', $endpoint->request_method_options)) {
            $linkset->resource_links[] = $this->get_related_link(
                $this,
                rel: 'delete',
                method: 'DELETE',
                use_namespace: false,
            );
        }
        # Loop through each field and check for any objects related to those fields.
        foreach ($this->get_fields() as $field) {
            # Determine the HAL rel value for this Field's Model relation
            $related_rel = "field:$field";

            # Obtain all related Models
            $related_model = $this->$field->get_related_model();
            $related_models = $this->$field->get_related_models();

            # Generate a resource link for related Models that are $many enabled
            if ($related_model and $related_model->many) {
                $linkset->resource_links[] = $this->get_related_link($related_model, rel: $related_rel);
            }

            # If this field has multiple Model object relations, generate a resource link for each
            if ($related_models->model_objects and $this->$field->many) {
                # Generate a nested ResourceLinkSet for related Models that are $many enabled
                $nested_linkset = new ResourceLinkSet(rel: $related_rel, use_namespace: true);

                # Loop through each Model object related to this Field's values
                foreach ($related_models->model_objects as $related_model) {
                    if ($related_model and $related_model->many) {
                        $nested_linkset->resource_links[] = $this->get_related_link($related_model, rel: $related_rel);
                    }
                }

                # Add this nested ResourceLinkSet in our root ResourceLinkSet
                $linkset->resource_links[] = $nested_linkset;
            }
        }

        return $linkset;
    }

    /**
     * Checks if the required pfSense packages for this model are installed.
     * @throws NotFoundError When the Model requires a pfSense package that is not installed.
     * @throws ServerError When a package requires a PHP include file that could not be found.
     */
    private function check_packages() {
        # Loop through each required package and ensure it is present on the system.
        foreach ($this->packages as $pkg) {
            # Return an error if the package is not installed
            if (!is_pkg_installed($pkg)) {
                throw new NotFoundError(
                    message: "The requested action requires the '$pkg' package but it is not installed.",
                    response_id: 'MODEL_MISSING_REQUIRED_PACKAGE',
                );
            }
        }

        # Loop through each package dependency and add it.
        foreach ($this->package_includes as $include) {
            if (!include_once $include) {
                throw new ServerError(
                    message: "Model requires include file $include but it could not be found in the PHP path.",
                    response_id: 'MODEL_WITH_FAILED_INCLUDE',
                );
            }
        }
    }

    /**
     * Initializes the default 'pre_apply' method. This method is intended to be overidden by a child model class and
     * is called immediately before the 'apply' method. This method runs regardless of whether an apply was requested.
     */
    protected function pre_apply() {}

    /**
     * Initializes the default 'apply' method. This method is intended to be overridden by a child model class and is
     * responsible for reloading/restarting services after changes occur.
     */
    public function apply() {}

    /**
     * Initializes the default 'pre_apply_create' method. This method is intended to be overridden by a child model class and
     * is called immediately before the 'apply' method for create actions only. This method runs regardless of whether
     * an apply was requested. By default, this method simply calls the global 'pre_apply' method.
     */
    protected function pre_apply_create() {
        $this->pre_apply();
    }

    /**
     * Performs steps and processes required to apply changes after the `create()` method runs. This can be overridden
     * to add custom steps applying for just create actions. Defaults to simply calling `apply()`.
     */
    protected function apply_create() {
        $this->apply();
    }

    /**
     * Initializes the default 'pre_apply_update' method. This method is intended to be overridden by a child model class and
     * is called immediately before the 'apply' method for update actions only. This method runs regardless of whether
     * an apply was requested. By default, this method simply calls the global 'pre_apply' method.
     */
    protected function pre_apply_update() {
        $this->pre_apply();
    }

    /**
     * Performs steps and processes required to apply changes after the `update()` method runs. This can be overridden
     * to add custom steps to apply after update actions only. Defaults to simply calling `apply()`.
     */
    protected function apply_update() {
        $this->apply();
    }

    /**
     * Initializes the default 'pre_apply_delete' method. This method is intended to be overridden by a child model class and
     * is called immediately before the 'apply' method for delete actions only. This method runs regardless of whether
     * an apply was requested. By default, this method simply calls the global 'pre_apply' method.
     */
    protected function pre_apply_delete() {
        $this->pre_apply();
    }

    /**
     * Performs steps and processes required to apply changes after the `delete()` method runs. This can be overridden
     * to add custom steps to apply after delete actions only. Defaults to simply calling `apply()`.
     */
    protected function apply_delete() {
        $this->apply();
    }

    /**
     * Performs steps and processes required to apply changes after the `replace_all()` method runs. This can be
     * overridden to add custom steps to apply after replace all actions only. Defaults to simply calling `apply()`.
     * @param ModelSet $initial_objects A ModelSet containing each existing object for this Model as they were when
     * the `replace_all()` method was initially called. This is useful when a function or service needs to be restarted
     * for each object before replacing the set.
     * @param ModelSet $new_objects A ModelSet containing each of the new objects for this Model. Note: at this
     * point, each Model object within this ModelSet has already been written to config. This method is responsible
     * for ensuring it is applied to any backend services.
     */
    protected function apply_replace_all(ModelSet $initial_objects, ModelSet $new_objects) {
        $this->apply();
    }

    /**
     * Sorts `many` Model entries internally before writing the changes to config. This is useful for Model's whose
     * internal objects must be written in a specific order.
     */
    protected function sort(): void {
        # Do not sort if there is no `sort_by` fields set
        if (!$this->sort_by) {
            return;
        }

        # Do not allow non `many` Models to be sorted
        if (!$this->many) {
            throw new ServerError(
                message: 'Only `many` Models can be sorted.',
                response_id: 'MODEL_CANNOT_BE_SORTED_WITHOUT_MANY',
            );
        }

        # Obtain all Model objects for this Model and sort them by the requested criteria
        $modelset = $this->query(
            sort_by: $this->sort_by,
            sort_order: $this->sort_order,
            sort_flags: $this->sort_flags,
            parent_id: $this->parent_id,
        );

        # Loop through the sorted object and assign it's internal value
        $internal_objects = [];
        $id_found = false;
        foreach ($modelset->model_objects as $id => $sorted_model_object) {
            # Assign the internal object to the internal objects array
            $internal_objects[] = $this->get_config($sorted_model_object->get_config_path())[$sorted_model_object->id];

            # If the sorted object's ID matches this object's ID, update this object's ID.
            if (!$id_found and $this->id === $sorted_model_object->id) {
                $this->id = $id;
                $id_found = true;
            }
        }

        # Sets the sorted internal objects to the pfSense config
        $this->set_config($this->get_config_path(), $internal_objects);
    }

    /**
     * For $many enabled models, this method will place this object in a specific location in the list of stored
     * objects. This is primarily used to
     */
    protected function set_placement(): void {
        # Only perform this action for $many enabled Models with a $config_path and $placement assigned
        if (!$this->many or is_null($this->placement) or !$this->config_path) {
            return;
        }

        # Do not allow the placement to be set if this object does not have an ID
        if (!isset($this->id)) {
            throw new ServerError(
                message: 'Model `id` is required to set placement.',
                response_id: 'MODEL_REQUIRES_ID_TO_SET_PLACEMENT',
            );
        }

        # Obtain the current internal objects for this Model
        $objects = $this->get_config(path: $this->get_config_path(), default: []);

        # Skip placement if the requested placement value does not exist in the current internal objects
        $this->placement = min($this->placement, count($objects));

        # Remove the object from the original index and insert it to the new index
        $object = array_splice($objects, $this->id, length: 1);
        array_splice($objects, $this->placement, length: 0, replacement: $object);

        # Ensure the object is in the correct index now
        if ($objects[$this->placement] === $object[0]) {
            # Set the new internal objects to the config and assign this object's new ID
            $this->set_config(path: $this->get_config_path(), value: $objects);
            $this->id = $this->placement;
            $this->placement = null;
        } else {
            throw new ServerError(
                message: 'Failed to set placement for Model object.',
                response_id: 'MODEL_FAILED_TO_SET_PLACEMENT',
            );
        }
    }

    /**
     * Appends changes between the current object and the initial object for `many` enabled fields. This allows for
     * changes to array field values to be appended to the existing array during updates instead of replaced.
     */
    private function append_array_changes(): void {
        # Loop through each field in this Model object and append changes to array fields
        foreach ($this->get_fields() as $field) {
            # Skip this field if it is not a `many` Field
            if (!$this->$field->many) {
                continue;
            }

            # Skip this field if it is not an array
            if (!is_array($this->$field->value)) {
                continue;
            }

            # Append the changes to the existing array if they are not the same
            if ($this->$field->value !== $this->initial_object->$field->value) {
                $this->$field->value = array_merge($this->initial_object->$field->value, $this->$field->value);
            }
        }
    }

    /**
     * Removes changes between the current object and the initial object for `many` enabled fields. This allows for
     * changes to array field values to be removed from the existing array during updates instead of being replaced.
     */
    private function remove_array_changes(): void {
        # Loop through each field in this Model object and remove changes from array fields
        foreach ($this->get_fields() as $field) {
            # Skip this field if it is not a `many` Field
            if (!$this->$field->many) {
                continue;
            }

            # Skip this field if it is not an array
            if (!is_array($this->$field->value)) {
                continue;
            }

            # Remove the changes from the existing array if they are not the same
            if ($this->$field->value !== $this->initial_object->$field->value) {
                $this->$field->value = array_diff($this->initial_object->$field->value, $this->$field->value);
            }
        }
    }

    /**
     * Fetches Model objects for all objects stored in the internal pfSense values. If `config_path` is set, this will
     * load Model objects for each object stored at the config path. If `internal_callable` is set, this will create
     * Model objects for each object returned by the specified callable.
     * @param int $offset The starting point in the dataset to be used with $limit. This is only applicable to $many
     * enabled Models.
     * @param int $limit The maximum number of Model objects to retrieve. This is only applicable to $many
     * enabled Models.
     * @param bool $reverse Set to true to reverse the order of the ModelSet. This is only applicable to $many enabled
     * Models.
     * @return ModelSet|Model Returns a ModelSet of Models if `many` is enabled or a single Model object if `many` is
     * not enabled.
     */
    public static function read_all(int $limit = 0, int $offset = 0, bool $reverse = false): ModelSet|Model {
        # Variables
        $model_name = self::get_class_fqn();
        $model = new $model_name();
        $model_objects = [];
        $requests_pagination = ($limit or $offset);
        $cache_exempt = ($requests_pagination or $reverse or $model->model_cache_exempt);

        # Throw an error if pagination was requested on a Model without $many enabled
        if (!$model->many and $requests_pagination) {
            throw new ValidationError(
                message: "Model `$model->verbose_name` does not support pagination. Please remove the `limit` and/or. " .
                    '`offset`parameters and try again.',
                response_id: 'MODEL_DOES_NOT_SUPPORT_PAGINATION',
            );
        }

        # Load from cache if it is not exempt and cached objects exist
        if (!$cache_exempt and self::get_model_cache()::has_modelset($model_name)) {
            return Model::get_model_cache()::fetch_modelset($model_name);
        }

        # Obtain all of this Model's internally stored objects, including those from parent Models if applicable.
        # Pass the maximum number of objects needed so that callables (e.g. log readers) can stop early.
        # When $reverse is true, the caller wants the oldest entries, so we cannot pre-limit to the newest -
        # the full dataset must be loaded to reverse correctly.
        $max_needed = ($limit > 0 && !$reverse) ? ($limit + $offset) : 0;
        $internal_objects = $model->get_internal_objects(from_all_parents: true, limit: $max_needed);

        # For non `many` Models, wrap the internal object in an array so we can loop
        $internal_objects = $model->many ? $internal_objects : [$internal_objects];

        # Reverse the order we read the objects and/or paginate if requested
        $internal_objects = $reverse ? array_reverse($internal_objects, preserve_keys: true) : $internal_objects;
        $internal_objects = self::paginate($internal_objects, $limit, $offset, preserve_keys: true);

        # Loop through each internal object and create a Model object for it
        foreach ($internal_objects as $internal_id => $internal_object) {
            # For Models with parent Models, the internal object always contains extra context data about
            # the parent Model class. See the get_internal_objects_from_all_parents() method for more information.
            $internal_id = $model->parent_model_class ? $internal_object['_id'] : $internal_id;
            $parent_model = $model->parent_model_class ? $internal_object['_parent_model'] : null;
            $parent_id = $parent_model ? $parent_model->id : null;
            $internal_object = $parent_model ? $internal_object['_internal_object'] : $internal_object;

            # Normalize IDs
            $internal_id = is_numeric($internal_id) ? (int) $internal_id : $internal_id;

            # Create a new Model object for this internal object and assign its ID
            $model_object = new $model(id: $internal_id, parent_id: $parent_id, skip_init: true);
            $model_object->parent_model = $parent_model;

            # Populate the Model object using its current internal values and add it to the array of all Model objects
            $model_object->from_internal_object($internal_object);

            $model_objects[] = $model_object;
        }

        # Load the ModelSet with all obtained Model objects
        $modelset = new ModelSet(model_objects: $model_objects, signature: $model_name);

        # For many models, cache the ModelSet if not exempt
        if ($model->many and !$cache_exempt) {
            self::get_model_cache()::cache_modelset($modelset);
        }

        # For many enabled Models return a ModelSet, otherwise return a single Model object
        return $model->many ? $modelset : $modelset->first();
    }

    /**
     * Performs a query on all Model objects for this Model. This is essentially a shorthand way of calling
     * `query()`. This method is only applicable to `many` Models.
     * @param array $query_params An array of query parameters.
     * @param array $excluded An array of field names to exclude from the query. This is helpful when
     * query data may have extra values that you do not want to include in the query.
     * @param int $offset The starting point in the dataset to be used with $limit. This is only applicable to $many
     * enabled Models.
     * @param int $limit The maximum number of Model objects to retrieve. This is only applicable to $many
     * enabled Models.
     * @param bool $reverse Set to true to reverse the order of the ModelSet. This is only applicable to $many enabled
     * Models.
     * @param mixed ...$vl_query_params Qyery parameters that are passed in using variable-length arguments
     * https://www.php.net/manual/en/functions.arguments.php#functions.variable-arg-list
     * @return ModelSet The queried ModelSet containing only Model objects that matched the query.
     */
    public static function query(
        array $query_params = [],
        array $excluded = [],
        int $limit = 0,
        int $offset = 0,
        bool $reverse = false,
        ?array $sort_by = null,
        int $sort_order = SORT_ASC,
        int $sort_flags = SORT_REGULAR,
        ...$vl_query_params,
    ): ModelSet {
        # Merge the $query_params and any provided variable-length arguments into a single variable
        $query_params = array_merge($query_params, $vl_query_params);

        # If no query or sort parameters were provided, just run read_all() with pagination for optimal performance
        if (!$query_params and $sort_by === null) {
            return self::read_all(limit: $limit, offset: $offset, reverse: $reverse);
        }

        # Perform the query against all Model objects for this Model first
        $modelset = self::read_all()->query(query_params: $query_params, excluded: $excluded);

        # Sort the set if a sort field was provided
        if ($sort_by) {
            $modelset = $modelset->sort(fields: $sort_by, order: $sort_order, flags: $sort_flags, retain_ids: true);
        }

        # Apply pagination to limit the number of objects returned and/or reverse the order if requested
        $modelset->model_objects = self::paginate($modelset->model_objects, $limit, $offset);
        return $reverse ? $modelset->reverse() : $modelset;
    }

    /**
     * Paginates a given array but obtaining a smaller subset of the array based on the provided limit and offset values.
     * @param array $array The array to paginate.
     * @param int $limit The maximum number of items to return in the paginated array. Use 0 to impose no limit.
     * @param int $offset The starting point in the array to begin the paginated array.
     * @param bool $preserve_keys Set to true to preserve the keys of the array. This is useful when the array is
     * associative and you want to maintain the key-value pairs.
     * @return array The paginated array containing only the subset of items that match the limit and offset values.
     */
    public static function paginate(array $array, int $limit, int $offset, bool $preserve_keys = false): array {
        # Return the entire array if no limit or offset is set
        if (!$limit and !$offset) {
            return $array;
        }

        # Return an empty array if the offset is greater than the array length
        if ($offset >= count($array)) {
            return [];
        }

        # Return the entire array if the limit is set to 0
        if (!$limit) {
            return array_slice($array, $offset);
        }

        # Return the subset of the array that matches the limit and offset values
        return array_slice($array, $offset, $limit, preserve_keys: $preserve_keys);
    }

    /**
     * Initializes the default 'read' method. By default, this method loads the Model object from its internal
     * object using the current $this->id value and returns the ($this) Model object. Running this method will overwrite
     * $this object.
     * @return Model Loads the Model object from its internal value using the current $this->id value and
     * returns that object.
     */
    final public function read(): Model {
        # When an ID is required and this is a `many` model, check that an ID is set.
        if ($this->many and !isset($this->id)) {
            throw new ValidationError(message: "Field 'id' is required.", response_id: 'MODEL_REQUIRES_ID');
        }

        # Ensure the parent Model exists if configured
        $this->get_parent_model();

        # Obtain the requested Model object from config by ID
        $this->from_internal();

        return $this;
    }

    /**
     * Initializes the default '_create' method. For Models with a `config_path` and `many` set, this method will
     * automatically obtain the next model ID for the object and write this object to the pfSense config. Other Models
     * will require this method to be overridden to support creations for this object.
     * @throws ServerError When the Model requesting creation does not have `many` enable and/or does not have a
     * `config_path` set. These Models' child class must override this method to add context-specific creations.
     */
    protected function _create() {
        # Check if this model has a `config_path` and `many` set, we will apply a default behavior for these Models
        if ($this->config_path and $this->many) {
            # Write the new object to the internal config
            $this->set_config(path: "{$this->get_config_path()}/$this->id", value: $this->to_internal());
            $this->set_placement();
            $this->sort();
            $this->write_config("Added $this->verbose_name via API");
            return;
        }

        # Otherwise, this method needs to be overridden by a child class to add specific context. Throw an error.
        throw new ServerError(
            message: "Models without a 'config_path' or 'many' require an overridden _create method",
            response_id: 'MODEL_REQUIRE_OVERRIDDEN_CREATE_METHOD',
        );
    }

    /**
     * Perform validation, create this object internally and restart any associated services.
     * @param bool $apply Set to `true` if you would like to immediately apply the changes after they are made.
     * @return Model Returns the created Model object (this object).
     */
    final public function create(bool $apply = false): Model {
        # Ensure all object Fields and validations succeed for proceeding.
        if ($this->validate(requires_id: !$this->auto_create_id, create_id: $this->auto_create_id)) {
            # Check if creating this object would put the total number of objects over the $many_maximum
            $this->check_many_maximum();

            # Run the _create method
            $this->_create();

            # Mark the subsystem as dirty if set
            if ($this->subsystem) {
                mark_subsystem_dirty($this->subsystem);
            }

            # Run the pre-apply method
            $this->pre_apply_create();

            # Only apply this change immediately if requested
            if ($apply or $this->always_apply) {
                $this->apply_create();
                clear_subsystem_dirty($this->subsystem);
            }

            # Refresh the initial object
            $this->initial_object = $this->copy();
        }

        # Reset the object cache as the config has changed
        self::clear_model_cache();

        # Return the current representation of this object
        return $this;
    }

    /**
     * Initializes the default '_update' method. For Models with a `config_path`, this method will automatically
     * write changes to the existing object in configuration. Other Models must override this method with context
     * specific updates.
     * @throws ServerError When the Model requesting update does not have a `config_path` set. These Models' child
     * class must override this method to add context-specific updates.
     */
    protected function _update() {
        # Check if this model has a `config_path`, we will apply a default behavior for these Models
        if ($this->config_path) {
            # Only use the ID in the config path if 'many' is enabled. Otherwise, just use the base config path.
            $config_path = $this->many ? "{$this->get_config_path()}/$this->id" : $this->get_config_path();

            # Merge differences between the stored object and updated internal object when update strategy is 'merge'
            if ($this->update_strategy === 'merge') {
                $this->merge_config($config_path);
            }
            # Replace the internal object entirely when update strategy is 'replace'
            elseif ($this->update_strategy === 'replace') {
                $this->set_config(path: $config_path, value: $this->to_internal());
            }
            # Throw an error if the update strategy is unknown
            else {
                throw new ServerError(
                    message: "Model has unknown update strategy '$this->update_strategy'",
                    response_id: 'MODEL_WITH_UNKNOWN_MERGE_STRATEGY',
                );
            }

            # Sort and write the changes to the object in config.
            $this->set_placement();
            $this->sort();
            $this->write_config("Modified $this->verbose_name via API");
            return;
        }

        # Otherwise, this method needs to be overridden by a child class to add specific context. Throw an error.
        throw new ServerError(
            message: "Models without a 'config_path' require an overridden _update method",
            response_id: 'MODEL_REQUIRE_OVERRIDDEN_UPDATE_METHOD',
        );
    }

    /**
     * Perform validation, update this object internally and restart any associated services.
     * @param bool $apply Set to `true` if you would like to immediately apply the changes after they are made.
     * @param bool $append Set to `true` if you would like to append changes to arrays in the existing object instead
     * of replacing the entire array. This only applies to `many` enabled Fields.
     * @param bool $remove Set to `true` if you would like to remove values from arrays in the existing object instead
     * of replacing the entire array. This only applies to `many` enabled Fields.
     * @return Model Returns the updated Model object (this object).
     */
    final public function update(bool $apply = false, bool $append = false, bool $remove = false): Model {
        # Append changes to the many enabled fields if requested. This must be done before validation.
        if ($append) {
            $this->append_array_changes();
        }

        # Remove changes from the many enabled fields if requested. This must be done before validation.
        if ($remove) {
            $this->remove_array_changes();
        }

        # Ensure all object Fields and validations succeed for proceeding.
        if ($this->validate(requires_id: true)) {
            # Run the _update method
            $this->_update();

            # Mark the subsystem as dirty if set
            if ($this->subsystem) {
                mark_subsystem_dirty($this->subsystem);
            }

            # Run the pre-apply method
            $this->pre_apply_update();

            # Only apply this change immediately if requested
            if ($apply or $this->always_apply) {
                $this->apply_update();
                clear_subsystem_dirty($this->subsystem);
            }

            # Refresh the initial object
            $this->initial_object = $this->copy();
        }

        # Reset the object cache as the config has changed
        self::clear_model_cache();

        # Return the current representation of this object
        return $this;
    }

    /**
     * Perform validation and replace all existing objects for this Model with a specified set of Model representations.
     * This method is only available for $many Models.
     * @param mixed $data An array of many Model representations to set. Any existing Model objects will
     * be replaced with the objects defined in this array.
     * @param bool $apply Apply this change immediately after replacing all Model objects. Note: Models with the
     * $always_apply property set to `true` will ignore this value.
     * @return ModelSet A ModelSet containing all the Model objects found in config after replacement.
     */
    final public function replace_all(mixed $data, bool $apply = false): ModelSet {
        # Ensure this is a `many` model before continuing
        if (!$this->many) {
            throw new ServerError(
                message: 'Model `replace_all()` method is only available for Models with `many` enabled.',
                response_id: 'MODEL_UPDATE_ALL_WITHOUT_MANY',
            );
        }

        # Ensure the representation data is an array
        if (!is_array($data)) {
            throw new ValidationError(
                message: 'Representation data must be of type `array`.',
                response_id: 'MODEL_NON_ARRAY_REPRESENTATION_DATA',
            );
        }

        # Do not allow this replacement unless the number of objects is equal to or greater than the $many_minimum
        if (count($data) < $this->many_minimum) {
            throw new ValidationError(
                message: "A minimum of $this->many_minimum new objects is required to replace all existing objects.",
                response_id: 'MODEL_REPLACE_ALL_MINIMUM_NOT_MET',
            );
        }

        # Do not allow this replacement unless the number of objects is equal to or greater than the $many_minimum
        if (count($data) > $this->many_maximum) {
            throw new ValidationError(
                message: 'The request to replace all existing objects exceeds the maximum limit of ' .
                    "$this->many_maximum new objects.",
                response_id: 'MODEL_REPLACE_ALL_MAXIMUM_EXCEEDED',
            );
        }

        # Keep track of all existing objects for this Model before anything is changed. This will be passed back
        # into `apply_replace_all()` so that method can gracefully bring down these objects before replacing them
        # if needed.
        $initial_objects = $this->query(parent_id: $this->parent_id);

        # Obtain any Models that are deemed protected to ensure they do not removed in the next step.
        $new_objects = new ModelSet();
        if ($this->protected_model_query) {
            $new_objects = $initial_objects->query(query_params: $this->protected_model_query);
        }

        # Before beginning validation, empty the stored values in the config path. Retain protected Models (if any).
        $this->set_config($this->get_config_path(), $new_objects->to_internal());

        # Loop through each Model representation included in the array
        foreach ($data as $representation_object) {
            # Remove any ID for this object if it was passed in, new IDs will be determined later
            unset($representation_object['id']);

            # Define a new Model object using this representation and validate
            $model_object = new $this(data: $representation_object, skip_init: true, client: $this->client);
            $model_object->id = $model_object->get_next_id();
            $model_object->validate(modelset: $new_objects);

            # If an error was not thrown during Model object creation or validation, give it an ID and set the config
            $this->set_config("{$this->get_config_path()}/$model_object->id", $model_object->to_internal());

            # Add the new Model object to the ModelSet
            $new_objects->model_objects[] = $model_object;
        }

        # Sort and write the changes to config
        $this->sort();
        $this->write_config("Replaced all $this->verbose_name_plural via API");

        # Mark the subsystem as dirty if set
        if ($this->subsystem) {
            mark_subsystem_dirty($this->subsystem);
        }

        # Apply if requested
        if ($apply or $this->always_apply) {
            $this->apply_replace_all($initial_objects, $new_objects);
            clear_subsystem_dirty($this->subsystem);
        }

        # Reset the object cache as the config has changed
        self::clear_model_cache();

        return $new_objects;
    }

    /**
     * Initializes the default '_delete' method. For Models with a `config_path` and `many`, this method will
     * automatically remove the existing object from configuration. Other Models must override this method with
     * context-specific deletions.
     * @throws ServerError When the Model requesting delete does not have a `config_path` or `many` set. These Models'
     * child class must override this method to add context-specific deletions.
     */
    protected function _delete() {
        # Check if this model has a `config_path`, we will apply a default behavior for these Models
        if ($this->config_path and $this->many) {
            # Write the changes to the object in config.
            $this->del_config(path: "{$this->get_config_path()}/$this->id");
            $this->write_config("Deleted $this->verbose_name via API");
            return;
        }

        # Otherwise, this method needs to be overridden by a child class to add specific context. Throw an error.
        throw new ServerError(
            message: "Models without a 'config_path' require an overridden _delete method",
            response_id: 'MODEL_REQUIRE_OVERRIDDEN_DELETE_METHOD',
        );
    }

    /**
     * Perform validation, delete this object internally and restart any associated services.
     * @param bool $apply Set to `true` if you would like to immediately apply the changes after they are made.
     * @return Model Returns the deleted Model object (this object).
     * @throws ConflictError When this Model object is referenced by another existing Model object.
     */
    final public function delete(bool $apply = false): Model {
        # Ensure all object Fields and validations succeed for proceeding.
        if ($this->validate(requires_id: true, only_id: true)) {
            # For $many enabled Models with protected Models, ensure the value being deleted is not a protected model
            if ($this->many and $this->protected_model_query) {
                $protected_models = $this->query(
                    query_params: $this->protected_model_query,
                    parent_id: $this->parent_id,
                );

                # Throw an error if this is a protected Model
                if ($protected_models->query(id: $this->id)->exists()) {
                    throw new ForbiddenError(
                        message: "Model object with ID `$this->id` cannot be deleted because it is protected.",
                        response_id: 'MODEL_CANNOT_DELETE_PROTECTED_OBJECT',
                    );
                }
            }

            # Block the deletion if this Model is still referenced by other Model objects
            $referenced_by = $this->get_model_references();
            if ($referenced_by->model_objects) {
                throw new ConflictError(
                    message: 'Object cannot be deleted because it is referenced by another existing object.',
                    response_id: 'MODEL_CANNOT_BE_DELETED_WHILE_REFERENCED',
                    data: $referenced_by,
                );
            }

            # Block the deletion if will put us below the minimum number of objects required for this Model
            $this->check_many_minimum();

            # Run the _delete method
            $this->_delete();

            # Mark the subsystem as dirty if set
            if ($this->subsystem) {
                mark_subsystem_dirty($this->subsystem);
            }

            # Run the pre-apply method
            $this->pre_apply_delete();

            # Only apply this change immediately if requested or configured to do so
            if ($apply or $this->always_apply) {
                $this->apply_delete();
                clear_subsystem_dirty($this->subsystem);
            }

            # Refresh the initial object
            $this->initial_object = $this->copy();
        }

        # Reset the object cache as the config has changed
        self::clear_model_cache();

        # Return the current representation of this object
        return $this;
    }

    /**
     * Uses a query to delete many Model objects at once. This method is only available for $many Models.
     * @param array $query_params An array of query parameters.
     * @param array $excluded An array of field names to exclude from the query. This is helpful when
     *  query data may have extra values that you do not want to include in the query.
     * @param mixed|null $parent_id Specifies the ID of the parent Model to read all objects from. This is required for
     *  $many Models with a $parent_model_class. This value has no affect otherwise.
     * @param int $offset The starting point in the dataset to be used with $limit. This is only applicable to $many
     *  enabled Models.
     * @param int $limit The maximum number of Model objects to retrieve. This is only applicable to $many
     *  enabled Models.
     * @param mixed ...$vl_query_params Qyery parameters that are passed in using variable-length arguments
     *  https://www.php.net/manual/en/functions.arguments.php#functions.variable-arg-list
     * @return ModelSet The queried ModelSet containing only Model objects that matched the query.
     */
    public static function delete_many(
        array $query_params = [],
        array $excluded = [],
        int $limit = 0,
        int $offset = 0,
        ...$vl_query_params,
    ): ModelSet {
        # Merge the $query_params and any provided variable-length arguments into a single variable
        $query_params = array_merge($query_params, $vl_query_params);

        # As a safety net, always require at least one query parameter to perform mass deletion.
        if (!$query_params) {
            throw new ValidationError(
                message: 'At least one query parameter is required to perform mass deletion.',
                response_id: 'MODEL_DELETE_MANY_REQUIRES_QUERY_PARAMS',
            );
        }

        # Obtain all Model objects that match the query of objects to be deleted
        $model_objects = self::query(query_params: $query_params, excluded: $excluded, limit: $limit, offset: $offset);

        # Reset the object cache as the config has changed
        self::clear_model_cache();

        # Delete the Model objects that matched the query
        return $model_objects->delete();
    }

    /**
     * Deletes all Model objects for this Model. This method is only available for $many Models without a $parent_model_class.
     */
    public static function delete_all(): ModelSet|Model {
        # Obtain all Model objects for this Model
        $model_objects = self::read_all();

        # Reset the object cache as the config has changed
        self::clear_model_cache();

        # Delete all Model objects for this Model
        return $model_objects->delete();
    }

    /**
     * Obtains an array of all Model classes.
     * @param $shortnames bool Set to true to obtain the shortnames of all Model classes.
     * @return array An array of all Model classes.
     */
    public static function get_all_model_classes(bool $shortnames = false): array {
        $model_classes = get_classes_from_namespace('\\RESTAPI\\Models\\');
        return $shortnames
            ? array_map(fn($class) => (new $class())->get_class_shortname(), $model_classes)
            : $model_classes;
    }
}
