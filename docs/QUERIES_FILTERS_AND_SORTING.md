# Queries, Filters and Sorting

## Queries

Queries can be used to filter the data that is returned from the API based on specific criteria. Queries are passed as
query parameters in the URL and are formatted as `key=value`. Multiple queries can be passed in a single request by
separating them with an ampersand `&`.

!!! Important
    - Queries are only available for `GET` requests to [plural endpoints](ENDPOINT_TYPES.md#plural-many-endpoints).
    - While it is not standard HTTP practice, the REST API will allow you to pass query parameters in the request body
    for `GET` requests as long as the correct Content-Type header is set. This may be useful when you need to force
    the type of a query parameter or when the query parameter is too long to fit in the URL.

## Query Filters

Below are the available query filters that can be used with the REST API. Each filter has a specific name and format that
must be followed. Filters can be used in combination with each other to create complex queries. To use a filter, simply
add the filter name to the end of the field name in the query parameter separated by two underscores `__`.

### Exact (exact)

Search for objects whose field value matches a given value exactly. This is assumed as the default query filter if no
query filter is specified.

- Name: `exact`
- Examples:
    - `https://pfsense.example.com/api/v2/examples?fieldname=example`
    - `https://pfsense.example.com/api/v2/examples?fieldname__exact=example`

### Starts With (startswith)

Search for objects whose field value starts with a given substring for non-array fields, or search for objects whose field
value array starts with a given value for array fields.

- Name: `startswith`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__startswith=example`

### Ends With (endswith)

Search for objects whose field value ends with a given substring for non-array fields, or search for objects whose field
value array ends with a given value for array fields.

- Name: `endswith`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__endswith=example`

### Contains (contains)

Search for objects whose field value contains a given substring for non-array fields, or search for objects whose field
value array contains a given value for array fields.

- Name: `contains`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__contains=example`

### In (in)

Search for objects whose field value is within a given array of values (when the filter value is an array), or search
for objects whose field value is a substring of a given string (when the filter value is a string).

- Name: `in`
- Examples: 
    - `https://pfsense.example.com/api/v2/examples?fieldname__in=example`
    - `https://pfsense.example.com/api/v2/examples?fieldname__in[]=example1&fieldname__in[]=example2`

### Less Than (lt)

Search for objects whose field value is less than a given integer.

- Name: `lt`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__lt=5`

### Less Than or Equal To (lte)

Search for objects whose field value is less than or equal to a given integer.

- Name: `lte`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__lte=5`

### Greater Than (gt)

Search for objects whose field value is greater than a given integer.

- Name: `gt`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__gt=5`

### Greater Than or Equal To (gte)

Search for objects whose field value is greater than or equal to a given integer.

- Name: `gte`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__gte=5`

### Format (format)

Search for objects whose field value matches a given format.

- Name: `format`
- Examples: 
    - `https://pfsense.example.com/api/v2/examples?fieldname__format=ipv4`
    - `https://pfsense.example.com/api/v2/examples?fieldname__format=email`

#### Supported Formats

- `ipv4`: Search for objects whose field value is a valid IPv4 address.
- `ipv6`: Search for objects whose field value is a valid IPv6 address.
- `ip`: Search for objects whose field value is a valid IP address (IPv4 or IPv6).
- `subnetv4`: Search for objects whose field value is a valid IPv4 subnet.
- `subnetv6`: Search for objects whose field value is a valid IPv6 subnet.
- `subnet`: Search for objects whose field value is a valid IP subnet (IPv4 or IPv6).
- `numeric`: Search for objects whose field value is a valid numeric value.
- `email`: Search for objects whose field value is a valid email address.
- `url`: Search for objects whose field value is a valid URL.
- `mac`: Search for objects whose field value is a valid MAC address.
- `fqdn`: Search for objects whose field value is a valid Fully Qualified Domain Name (FQDN).
- `hostname`: Search for objects whose field value is a valid hostname (without domain).
- `port`: Search for objects whose field value is a valid port number.
- `portrange`: Search for objects whose field value is a valid port range (separated by a colon `:`).
- `alias`: Search for objects whose field value is an existing firewall alias's name.

### Regex (regex)

Search for objects whose field value matches a given PCRE regular expression.

- Name: `regex`
- Example: `https://pfsense.example.com/api/v2/examples?fieldname__regex=^example`

### Custom Query Filters

For advanced users, the REST API's framework allows for custom query filter classes to be added using PHP. Refer to
[Building Custom Query Filters](BUILDING_CUSTOM_QUERY_FILTER_CLASSES.md) for more information.

## Sorting

Sorting can be used to order the data that is returned from the API based on specific criteria, as well as sorting the
objects written to the pfSense configuration. Sorting is controlled by three common control parameters: 
[`sort_by`](COMMON_CONTROL_PARAMETERS.md#sort_by), [`sort_flags`](COMMON_CONTROL_PARAMETERS.md#sort_flags), and [`sort_order`](COMMON_CONTROL_PARAMETERS.md#sort_order).

!!! Note
    - Sorting is only available for model objects that allow many instances, meaning multiple objects of its type can
      exist in the pfSense configuration (e.g. firewall rules, static routes, etc.).
    - Sorting requires additional processing time and may impact performance. Use sorting only when
      necessary.

The behavior of sorting varies based on the request method and endpoint type:

### GET requests to Plural (Many) Endpoints

For `GET` requests to [plural endpoints](ENDPOINT_TYPES.md#plural-many-endpoints), sorting allows to you sort the
objects returned in the `data` section of the API response by a specific field and a specific ordering. This does not
affect the order of the objects stored in the pfSense configuration.

### POST and PATCH requests to Singular Endpoints

For `POST` and `PATCH` requests to [singular endpoints](ENDPOINT_TYPES.md#singular-endpoints), sorting allows you to
sort the relevant objects in the pfSense configuration after creating or updating an object. This is useful when you 
need to control the order of objects in the configuration, especially for objects where the order of objects directly
affects the behavior (like ACLs). Some example use cases for sorting the configuration include:

- Reordering firewall rules based on a custom description.
- Reordering NAT rules based on interface.

!!! Warning
    - Use caution when setting the sort order of objects which may be sensitive to order such as firewall rules. Placing
      the object in the wrong location may have unintended consequences such as blocking all traffic or allowing all traffic.
    - Some endpoints may already have default sorting attributes. Setting the `sort_by` parameter will override these
      defaults which may result in unexpected behavior.

## Pagination

Pagination can be used to limit the number of items returned in a single request. Pagination is controlled by two query
parameters: `limit` and `offset`. The `limit` parameter specifies the number of items to return, and the `offset`
parameter specifies the starting index of the items to return. Pagination is useful when working with large datasets to
reduce the amount of data returned in a single request.

!!! Important
    - Most endpoints do not impose a limit on the number of items returned by default. Refer to the endpoint's
      [documentation](https://pfrest.org/api-docs/) to determine if a limit is imposed by default.
    - Pagination is only available for `GET` and `DELETE` requests to [plural endpoints](ENDPOINT_TYPES.md#plural-many-endpoints).
    - If combined with a query, pagination will be applied after the initial query is executed.
