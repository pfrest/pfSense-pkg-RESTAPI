<?php

namespace RESTAPI\Core;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Responses\ServerError;
use const RESTAPI\Core\Globals\HAL_REL_NAMESPACE;

/**
 * Defines a class that allows the storage and processing of many ResourceLink objects at once.
 */
class ResourceLinkSet {
    use BaseTraits;

    /**
     * Constructs a new ResourceLinkSet object
     * @param array $resource_links An array of ResourceLink objects to include in this ResourceLinkSet
     * @param string $rel The HAL related resource name for this ResourceLinkSet. This allows many ResourceLink objects
     * to represented under a single related resource name. If a value is specified here, this ResourceLinkSet is
     * expected to be nested within a root ResourceLinkSet that does not have this value defined.
     * @param bool $use_namespace Set to true to prefix the HAL namespace for this package to the $rel value or set to
     * false to use the $rel value directly. Unless $rel is a standard HAL related resource name, this should be set to
     * true.
     */
    public function __construct(
        public array $resource_links = [],
        public string $rel = '_links',
        public bool $use_namespace = false,
    ) {
        # Throw an error if any $resource_links value is not a ResourceLink or ResourceLinkSet object
        foreach ($this->resource_links as $resource_link) {
            if (!($resource_link instanceof ResourceLink) and !($resource_link instanceof ResourceLinkSet)) {
                throw new ServerError(
                    message: 'ResourceLinkSets can only contain ResourceLink and ResourceLinkSet objects.',
                    response_id: 'RESOURCE_LINK_SET_CONTAINS_NON_RESOURCE_LINK',
                );
            }
        }

        # Prefix the HAL namespace for this package to $rel if requested
        $this->rel = $this->use_namespace ? HAL_REL_NAMESPACE . ':' . $this->rel : $this->rel;
    }

    /**
     * Converts all ResourceLink objects in this ResourceLinkSet into their representation values.
     * @returns array An array representation of each HAL ResourceLink object nested under the HAL standard `_links`
     * array key.
     */
    public function to_representation(): array {
        # If an explicit $this->rel value was not given to this object, this object will be assumed as the root
        # ResourceLinkSet and be represented under the "_links" parent key.
        $representation = [$this->rel => []];

        # Loop through each ResourceLink object and add its representation value
        foreach ($this->resource_links as $resource_link) {
            # When this $resource_link is another ResourceLinkSet object, nest all ResourceLinks under a single array
            if ($resource_link instanceof ResourceLinkSet) {
                # Define an array to store the representation of each resource link within this sub-ResourceLinkSet
                $representation[$this->rel][$resource_link->rel] = [];

                # Loop through each resource link within this sub-ResourceLinkSet
                foreach ($resource_link->resource_links as $nested_resource_link) {
                    $representation[$this->rel][$resource_link->rel][] = $nested_resource_link->to_representation();
                }
            }

            # When this $resource_link is a ResourceLink object, just assign it's representation to this rel
            if ($resource_link instanceof ResourceLink) {
                $representation[$this->rel][$resource_link->rel] = $resource_link->to_representation();
            }
        }

        return $representation;
    }
}
