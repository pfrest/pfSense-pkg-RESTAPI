<?php

namespace RESTAPI\Core;

use RESTAPI\Responses\ServerError;

require_once 'RESTAPI/autoloader.inc';

/**
 * Defines a ModelSet object that acts as a container for multiple Model objects. This can be used to represent a
 * set of Model objects at once. Additionally, ModelSets can be further filtered using queries.
 */
class ModelSet {
    use BaseTraits;

    /**
     * @var string $signature A unique signature for this ModelSet used to index this ModelSet in the query cache
     * while retaining all query chains. Signatures are compromised of a root signature (the Model class name) and
     * a series of query signatures that represent each query that has been applied to this ModelSet to this point
     * (separated by a colon). Example 'RESTAPI\Models\User:queryhash1:queryhash2'
     */
    public string $signature;

    /**
     * @var array $model_objects An array of Model objects contained by this ModelSet.
     */
    public array $model_objects;

    /**
     * Creates a ModelSet object that contains multiple Model objects.
     * @param array $model_objects An array of model objects to include in this model set
     */
    public function __construct(array $model_objects = [], string $signature = '') {
        # Assign the signature if provided
        $this->signature = $signature;

        # Throw an error if any Models are not a Model object
        foreach ($model_objects as $model_object) {
            if (!is_object($model_object) or !in_array('RESTAPI\Core\Model', class_parents($model_object))) {
                throw new ServerError(
                    message: 'Model sets can only contain model objects.',
                    response_id: 'MODEL_SET_CONTAINS_NON_MODEL',
                );
            }
        }

        # Assign the model objects to this model set
        $this->model_objects = $model_objects;
    }

    /**
     * Checks if Model objects exist in this ModelSet. This helpful when determining if an API query found matches.
     * @return bool true if Model objects exist in this ModelSet, false if they do not.
     */
    public function exists(): bool {
        return $this->count() > 0;
    }

    /**
     * Returns the number of Model objects in this ModelSet.
     * @return int The number of Model objects in this ModelSet.
     */
    public function count(): int {
        return count($this->model_objects);
    }

    /**
     * Returns the first Model object in the ModelSet. This is helpful when locating the first Model object that matched
     * a query.
     */
    public function first(): Model {
        # Ensure there is at least 1 Model object in this ModelSet
        if ($this->exists()) {
            # Return the first Model object in the ModelSet
            return $this->model_objects[0];
        }

        # Throw an error if there are no Model objects in this ModelSet
        throw new ServerError(
            message: 'Cannot locate first Model object in a ModelSet with no Model objects',
            response_id: 'MODELSET_FIRST_REQUESTED_WITH_NO_MODEL_OBJECTS',
        );
    }

    /**
     * Reverse the order of the Model objects in this ModelSet.
     * @return ModelSet A new ModelSet object with the Model objects in reverse order.
     */
    public function reverse(): ModelSet {
        return new ModelSet(array_reverse($this->model_objects));
    }

    /**
     * Sorts the Model objects in this ModelSet by a specific field and order.
     * @param string|array $fields The field(s) to sort the Model objects by.
     * @param int $order The order to sort the Model objects by. This must be a PHP sort order constant.
     * @param int $flags The flags to use when sorting the Model objects. This must be a PHP sort flags constant.
     * @param bool $retain_ids Retain the original Model object IDs when sorting.
     * @return ModelSet A new ModelSet object with the Model objects sorted by the specified field and order.
     */
    public function sort(
        string|array $fields,
        int $order = SORT_ASC,
        int $flags = SORT_REGULAR,
        bool $retain_ids = false,
    ): ModelSet {
        # Ensure fields is an array
        $fields = is_array($fields) ? $fields : [$fields];

        # Extract sorting values for each field
        $sort_criteria = [];
        foreach ($fields as $field) {
            $sort_criteria[] = array_map(
                fn($model) => $field === 'id' ? $model->id : $model->$field->value,
                $this->model_objects,
            );
            $sort_criteria[] = $order;
            $sort_criteria[] = $flags;
        }

        # Append the actual model objects array for sorting
        $sort_criteria[] = &$this->model_objects;

        # Sort the array
        array_multisort(...$sort_criteria);

        # Re-assign model object IDs if they should not be retained
        if (!$retain_ids) {
            foreach ($this->model_objects as $id => $model_object) {
                $model_object->id = $id;
            }
        }

        return new ModelSet($this->model_objects);
    }

    /**
     * Runs the to_representation() method for each model object within the model set and returns them all in an array.
     * @return array A serializable representation of all model objects within this model set.
     */
    public function to_representation(): array {
        return $this->run_model_method(method_name: 'to_representation');
    }

    /**
     * Runs the to_internal() method for each model object within the model set and returns them all in an array.
     * @return array The internal configu values of all model objects within this model set.
     */
    public function to_internal(): array {
        return $this->run_model_method(method_name: 'to_internal', use_id_as_key: true);
    }

    /**
     * Deletes all Model objects within this ModelSet.
     * @param bool $dry_run Run through the delete request but don't actually delete anything
     * @return ModelSet The ModelSet object with all Model objects that were deleted.
     */
    public function delete(bool $dry_run = false): ModelSet {
        # Run the delete method for each model object in this ModelSet
        # Note: We have to do this in reverse order to avoid index issues when deleting objects.
        $this->run_model_method(method_name: 'delete', reverse: true, dry_run: $dry_run);
        return $this;
    }

    /**
     * Runs a specified Model object method for each model object within this ModelSet.
     * @param string $method_name The name of the Model method to run for each model object in this model set.
     * @param bool $use_id_as_key Use each Model object's `id` property value as the return array keys.
     * @param bool $reverse Reverse the order of which the model objects are processed.
     * @param bool $dry_run Sets the `dry_run` property of each Model object to true before running the method.
     * @return array The return value of method call for each model object. Results are stored in the same order as
     * the model objects were passed in during construction.
     */
    private function run_model_method(
        string $method_name,
        bool $use_id_as_key = false,
        bool $reverse = false,
        bool $dry_run = false,
    ): array {
        # Variables
        $results = [];
        $model_objects = $reverse ? array_reverse($this->model_objects) : $this->model_objects;

        # Loop through each model object and call its to_representation() method
        foreach ($model_objects as $loop_count => $model_object) {
            $id = $use_id_as_key ? $model_object->id : $loop_count;
            $model_object->dry_run = $dry_run;
            $results[$id] = $model_object->$method_name();
        }

        return $results;
    }

    /**
     * Provides access to the ModelCache singleton instance.
     * @return ModelCache The ModelCache singleton instance.
     */
    public static function get_model_cache(): ModelCache {
        return ModelCache::get_instance();
    }

    /**
     * Determines the signature for a given query. This signature is used to index queried ModelSets in the
     * ModelCache while retaining all query chains.
     * @param array $query_params An associative array of query parameters to use to generate the signature.
     * @return string The signature for the given query parameters.
     */
    public function get_query_signature(array $query_params = []): string {
        # ModelSet is cache exempt if there is no existing signature
        if (!$this->signature) {
            return '';
        }

        # Sort the query parameters by key to ensure consistent signatures
        ksort($query_params);

        # Encode the query parameters as JSON and generate an xxh hash
        $query_json = json_encode($query_params);
        $query_hash = hash(algo: 'xxh64', data: $query_json);

        # Append the query hash to the existing signature to retain query chains
        return "$this->signature:$query_hash";
    }

    /**
     * Filters the ModelSet to only include Model objects that match a specific query.
     * @param array $query_params An associative array of query targets and values. The array key will be the query
     * target and the array value will be the query value.
     * @param array $excluded An array of field names to exclude from the query. This is helpful when
     * query data may have extra values that you do not want to include in the query.
     * @param mixed ...$vl_query_params Qyery parameters that are passed in using variable-length arguments
     * https://www.php.net/manual/en/functions.arguments.php#functions.variable-arg-list
     * @return ModelSet The filtered ModelSet object containing only Model objects that matched the query.
     */
    public function query(array $query_params = [], array $excluded = [], ...$vl_query_params): ModelSet {
        # Variables
        $queried_model_set = [];
        $query_params = array_merge($query_params, $vl_query_params);
        $query_signature = $this->get_query_signature($query_params);
        $cache_exempt = !$query_signature;

        # Fetch the cached query if it exists and this ModelSet is not cache exempt
        if (!$cache_exempt and self::get_model_cache()::has_modelset($query_signature)) {
            return self::get_model_cache()::fetch_modelset($query_signature);
        }

        # Loop through each model object in the provided model set and check it against the query parameters
        foreach ($this->model_objects as $model_object) {
            # Assign variable to track matches
            $is_match = true;

            # Loop through each query parameter to determine if this object is a match
            foreach ($query_params as $query_target => $filter_value) {
                # Loop variables
                $query_options = $this->get_query_options($query_target);
                $field = $query_options['field'];
                $filter_name = $query_options['filter'];

                # Skip this field if it's excluded or not present in the model object
                if (in_array($field, $excluded) or !property_exists($model_object, $field)) {
                    continue;
                }

                # Obtain the value for this field. If it is the ID or parent ID, handle accordingly.
                $field_value = $field === 'id' ? $model_object->id : $model_object->$field->value;
                $field_value = $field === 'parent_id' ? $model_object->parent_id : $field_value;

                # Ensure the filter matches
                $filter = QueryFilter::get_by_name($filter_name);
                if (!$filter->evaluate($field_value, $filter_value)) {
                    $is_match = false;
                    break;
                }
            }
            # Add this model object to the filtered model set if it matched the query
            if ($is_match) {
                $queried_model_set[] = $model_object;
            }
        }

        # Create a new ModelSet with the queried model objects
        $modelset = new ModelSet($queried_model_set, signature: $query_signature);

        # Cache the queried ModelSet if this ModelSet is not cache exempt
        if (!$cache_exempt) {
            self::get_model_cache()::cache_modelset($modelset);
        }

        return $modelset;
    }

    /**
     * Checks for query target options. Query target options allow for more specific queries by limiting the scope to
     * values that match a specific filter.
     * @param string $query_target The query target string (e.g. `field_name`, `field_name__contains`)
     * @return array An array of query target options found in the $query_target string.
     */
    private function get_query_options(string $query_target): array {
        # Variables
        $query_options = ['field' => $query_target, 'filter' => 'exact'];

        # Check if the double-underscore indicator is present
        if (str_contains($query_target, '__')) {
            # Separate query target parameters by __ to determine query options
            $query_target = explode('__', $query_target);

            # Check if a valid query filter was specified
            if (QueryFilter::get_by_name(end($query_target))) {
                # Store the filter and remove the last item of the array.
                $query_options['filter'] = array_pop($query_target);
            }

            # Add the remaining values as the query field
            $query_options['field'] = implode('__', $query_target);
        }

        return $query_options;
    }
}
