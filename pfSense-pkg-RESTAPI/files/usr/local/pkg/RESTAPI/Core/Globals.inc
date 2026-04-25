<?php

namespace RESTAPI\Core\Globals;

# Define global constants
const HAL_REL_NAMESPACE = 'pfsense';
const RESERVED_FIELD_NAMES = [
    'id',
    'parent_id',
    '_links',
    'apply',
    'async',
    'limit',
    'offset',
    'extra',
    'placement',
    'append',
    'remove',
    'reverse',
    'api_host', # used by ansible collection
    'api_port', # used by ansible collection
    'api_protocol', # used by ansible collection
    'api_username', # used by ansible collection
    'api_key', # used by ansible collection
    'validate_certs', # used by ansible collection
    'objects', # used by ansible collection
    'lookup_fields', # used by ansible collection
    'parent_lookup_query', # used by ansible collection
    'query_params', # used by ansible collection
];

/**
 * @global array $mock_internal_objects Allows TestCases to define a mock internal data array that will be used by Models
 * instead of the actual internal data array.
 */
$mock_internal_objects = null;
