# Building Custom Model Classes

Model classes are a core component of the REST API. They define the structure of the data that the API will work with
and provide an simpler way to define pfSense configuration data in a consistent and easy to work with manner. Notable
responsibilities of Model classes include:

- Defining the data structure and typing of the data that the API will work with.
- Defining the XML configuration path that the Model represents and will read and write objects from.
- Defining the validation rules for the data that the Model represents.

This guide will walk you through the process of creating a custom Model class for the REST API.

## Getting Started

Use the following class template to initialize your custom Model class:

```php
<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;

/**
 * TODO: Add a description of your Model class here.
 */
class MyCustomModel extends Model {
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # TODO: Define Model attributes here
        
        # TODO: Define Model fields here
        
        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
```

!!! Important
    - Be sure to place this class file within `/usr/local/pkg/RESTAPI/Models/` directory and name the file after your 
    Model class name with a `.inc` extension.
!!! Warning
    The `__construct()` method signature and the parent constructor call MUST remain exactly as shown in the template.
    The parent constructor call also must be the last line in the `__construct()` method. Failure to do so will result in
    unexpected behavior and errors.
    

## Define __construct() Method Properties

The `__construct()` method is used to initialize the Model class. It is responsible for setting the various Model
properties such as the configuration path, sorting properties, etc., as well as [defining the various Field objects](#define-field-objects)
that will be used to define the data structure of the Model. Below are the properties available to help configure your Model:

### config_path

The `config_path` property is used to define the XML configuration path that the Model represents. This path is used to
read and write objects from the pfSense configuration. The `config_path` property is required unless you define an 
`internal_callable` property instead.

Example:

```php
$this->config_path = 'unbound/hosts';
```

!!! Notes
    - If your model has a `parent_model_class` property defined, the `config_path` property will be appended to the
      parent model's `config_path` property to form the full configuration path.

### internal_callable

The `internal_callable` property is used to define a callable function that will be used to read the internal data for
the Model's objects. This is typically pfSense data that is not stored in the configuration, such as real-time metrics.
The value assigned here must be a method available in the Model class. See the [Non-Configuration Based Models](#non-configuration-based-models) 
section for more information.

Example:

```php
$this->internal_callable = 'get_internal_data';
```

!!! Notes
    - The `internal_callable` property is an alternative to the `config_path` property. Both cannot be defined at the same time.
    - For `many` enabled Models, the callable function must return an array of objects, where each object represents an
      instance of the Model. For `many` disabled Models, the callable function must return a single object representing 
      the Model instance.
    - The callable function must be defined in the Model class and must be scoped as a public or protected method.

### many

The `many` property is used to define whether the Model represents a pfSense configuration object that can have multiple
instances (e.g. firewall rules, static routes) or a single instance (e.g. system settings, web GUI settings).

Example:

```php
$this->many = true;
```

!!! Notes
    - Setting the `many` property to `true` will enable IDs for the Model, allowing you to retrieve, update, and delete
      individual instances of the Model. Setting the `many` property to `false` will disable IDs for the Model.

### many_minimum

The `many_minimum` property is used to define the minimum number of instances that the Model must have. Any attempts to
delete or replace instances which would result in fewer instances than the `many_minimum` property will be rejected.

Example:

```php
$this->many_minimum = 1;
```

!!! Notes
    - The `many_minimum` property is only applicable when the `many` property is set to `true`.

### many_maximum

The `many_maximum` property is used to define the maximum number of instances that the Model can have. Any attempts to
create or replace instances which would result in more instances than the `many_maximum` property will be rejected.

Example:

```php
$this->many_maximum = 10;
```

!!! Notes
    - The `many_maximum` property is only applicable when the `many` property is set to `true`.

### parent_model_class

The `parent_model_class` property is used to define the parent Model class of the Model. Setting the `parent_model_class`
property will cause the Model to be nested under the parent Model in the pfSense configuration. Some examples of Models
with parent Models include DNS Resolver Host Override Aliases, IPsec Phase 1 Encryption Algorithms, etc.

Example:

```php
$this->parent_model_class = 'ParentModelClass';
```

!!! Notes
    - This property must be assigned the short name of the parent Model class, not the fully qualified class name.
    - After your Model object is constructed, you can access the parent Model object using the `parent_model` property.
    - When the `parent_model_class` property is defined, the `config_path` property will be appended to the parent Model's
      `config_path` property to form the full configuration path.
    - If a parent Model class is defined, the Model will require a `parent_id` when interacting with the object UNLESS the
      parent Model class's `many` property is set to `false`.

### cache_class

The `cache_class` property is used to define the Cache class that the Model will use to read data from. After the Model
object is constructed, you can access the Cache object using the `cache` property.

Example:

```php
$this->cache_class = 'CacheClassShortname';
```

!!! Notes
    - This property must be assigned the short name of the Cache class, not the fully qualified class name.
    - Assigning a `cache_class` does not mean the Model will automatically use the Cache. You must manually use the
      Cache object to read data from the cache in your Model's logic.

### protected_model_query

The `protected_model_query` property is used to define a array of query parameters that will be used to determine if
objects of the Model can be deleted. This is primarily used to prevent the deletion of built-in objects that should not
be deleted.

Example:

```php
$this->protected_model_query = [
    'name' => 'built_in_object',
];
```

!!! Notes
    - The `protected_model_query` property is only applicable when the `many` property is set to `true`.
    - The query parameters defined in the `protected_model_query` property must match the query parameters of the object
      that should not be deleted.

### packages

The `packages` property is used to define the pfSense packages that are required for the Model to function. An error will
be thrown if the required packages are not installed and attempts to create, update or delete objects of the Model are made.

Example:

```php
$this->packages = ['pfSense-pkg-nmap', 'pfSense-pkg-sudo'];
```

!!! Notes
    - The packages referenced here must include the `pfSense-pkg-` prefix.

### package_includes

The `package_includes` property is used to define the PHP files that are required for the Model to function. These dependencies
will be automatically included during object construction if the assigned `packages` are installed.

Example:

```php
$this->package_includes = ['nmap.inc', 'sudo.inc'];
```

!!! Notes
    - The files referenced here must be relative to one of the PHP include paths on pfSense.

### unique_together_fields

The `unique_together_fields` property is used to define an array of fields that must be unique together for objects of the
Model. An error will be thrown if an object is created or updated with a combination of values that already exist.

Example:

```php
$this->unique_together_fields = ['name', 'port'];
```

!!! Important
    Only use this property for two or more fields that must be unique together. If a single field must be unique, use the
    [`unique` property in the Field object](#unique) instead.
!!! Notes
    - The `unique_together_fields` property is only applicable when the `many` property is set to `true`.
    - The fields defined in the `unique_together_fields` property must be defined in the Model's Field objects.

### sort_by

The `sort_by` property is used to define the fields that objects of the Model will be sorted by. When this property is
set, objects created and updated will be sorted according to the assigned field using the assigned `sort_order`.

Example:

```php
$this->sort_by = ['name'];
```

!!! Warning
    The use of sorting in a Model may cause IDs to be re-ordered when objects are created or updated.
!!! Notes
    - The `sort_by` property is only applicable when the `many` property is set to `true`.
    - The field defined in the `sort_by` property must be defined in the Model's Field objects.

### sort_order

The `sort_order` property is used to define the PHP sorting option for objects of the Model. When this property is set,
objects created and updated will be sorted according to the assigned option. For valid sorting options, refer to: For valid value options for this property, refer to the
[PHP multi-sort function type flags](https://www.php.net/manual/en/function.array-multisort.php).

Example:

```php
$this->sort_order = SORT_ASC;
```

!!! Warning
    The use of sorting in a Model may cause IDs to be re-ordered when objects are created or updated.
!!! Notes
    - The `sort_order` property is only applicable when the `many` property is set to `true`.
    - The `sort_order` property is only applicable when a `sort_by` is defined.

### subsystem

The `subsystem` property is used to define the pfSense subsystem that the Model belongs to. When Models with a `config_path`
assigned are created, updated or delete, this subsystem will automatically be marked as dirty (changed, awaiting an apply).

Example:

```php
$this->subsystem = 'filter';
```

!!! Tip
    You will need to look through the pfSense source code to determine the subsystem to use for your Model. They will 
    typically be found in calls to the `mark_subsystem_dirty()` or `is_subsystem_dirty()` function.

!!! Notes
    - The `subsystem` property is only applicable when the `config_path` property is defined.

### always_apply

The `always_apply` property is used to define whether the Model should always apply changes to the pfSense configuration
when objects are created, updated or deleted. When this property is set to `true`, changes will always be applied
immediately. When set to `false`, changes will only be applied when the `apply` method is called.

Example:

```php
$this->always_apply = true;
```

### verbose_name

The `verbose_name` property is used to define the human-readable name of the Model. This name will be used in documentation
and other areas where a human-readable name is required. If not defined, the name will be automatically generated on a best
guess basis using the Model class name.

Example:

```php
$this->verbose_name = 'My Custom Model';
```

!!! Notes
    - A `verbose_name` is typically only required when the automatically generated name is not suitable.

### verbose_name_plural

The `verbose_name_plural` property is used to define the human-readable plural name of the Model. This name will be used
in documentation and other areas where a human-readable name is required. If not defined, the plural name will be 
automatically generated on a best guess basis.

Example:

```php
$this->verbose_name_plural = 'My Custom Models';
```

!!! Notes
    - A `verbose_name_plural` is typically only required when the automatically generated plural name is not suitable.

## Define Field Objects

Field objects are used to set the data structure of individual fields within the Model. They define the data type, 
validation rules, and other properties of the field. Fields must be defined in the Model's `__construct()` method and be
assigned a unique property name.

Below is an example of how to define Field objects in a Model class:

```php
<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * TODO: Add a description of your Model class here.
 */
class MyCustomModel extends Model {
    public StringField $name;
    public IntegerField $timeout;
    public BooleanField $enabled;
    
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'path/to/model/objects/in/config';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            help_text: 'Demonstrates an example StringField.',
        );
        $this->timeout = new IntegerField(
            default: 30,
            help_text: 'Demonstrates an example IntegerField.',
        );
        $this->enabled = new BooleanField(
            default: true,
            indicates_true: "on", // The value stored in the XML configuration when the field is true
            indicates_false: "off", // The value stored in the XML configuration when the field is false
            help_text: 'Demonstrates an example BooleanField.',
        );
        
        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
```

!!! Important
    - All Field objects must be scoped as public properties of the Model class.
    - In general, the property name you choose for the Field object should match the field name stored in the XML configuration. If you cannot match the field name, you must use the `internal_name` property in the Field object to define the XML configuration field's name.


Field objects have many properties that can be set to define the data structure of the field. Some properties are unique
to the type of Field object being used, for a full list of available Field classes and their associated properties, 
refer to the [PHP reference](https://pfrest.org/php-docs/namespaces/restapi-fields.html). Below are some common properties
that can be set on Field objects:

### required

The `required` property is used to define whether the field is required when creating or updating objects of the Model.
If `true`, the field must always have a value assigned to pass validation.

!!! Notes
    - The `required` and `default` properties are mutually exclusive and cannot both be assigned to single Field object.

### many

The `many` property is used to define whether the field can have multiple values. If `true`, the field's value must be an array
of values. Each value within the array will be subject to the same property constraints and validation rules defined in the Field object individually.

!!! Notes
    - The `many` property is not applicable to some fields such as the `BooleanField` class.

### many_minimum

The `many_minimum` property is used to define the minimum number of values that the field must have when the `many` property is set to `true`.

!!! Notes
    - The `many_minimum` property value must be an integer less than or equal to the `many_maximum` property value.

### many_maximum

The `many_maximum` property is used to define the maximum number of values that the field can have when the `many` property is set to `true`.

!!! Notes
    - The `many_maximum` property value must be an integer greater than or equal to the `many_minimum` property value.

### delimiter

The `delimiter` property is used to define the delimiter that will be used to separate values for this field when the values are stored in the XML configuration. 
This property is only applicable to `many` enabled Field objects.

!!! Notes
    - In most cases, pfSense stores array values as a comma-separated string in the XML configuration.
    - In the situation that pfSense stores the values as an actual array in the XML configuration, set this property to `null`

### default

The `default` property is used to define the default value of the field when creating instances of the Model. If no value is
assigned to the field, the default value will be used. This value must be of the same type as the Field object.

!!! Notes
    - In the case of a `many` enabled Field, the default value must be an array of the same type as the Field object.

### default_callable

The `default_callable` property is used to define a callable function that will be used to determine the default value of the
field when creating instances of the Model. The value assigned here must be a method available in the Model class. This
should be used instead of the `default` property when the default value is dynamic.

!!! Notes
    - The `default_callable` property is an alternative to the `default` property. Both cannot be defined at the same time.

### choices

The `choices` property is used to define a list of valid choices for the field. If defined, the field will only accept values
that are included in the list. You can define choices as a simple array or as an associative array where the keys are the
stored values and the values are the human-readable names for the choices. 

Example:

```php
$this->field = new StringField(
    required: true, 
    choices: ['choice1', 'choice2', 'choice3']
);
```
or
```php
$this->field = new StringField(
    required: true, 
    choices: ['choice1' => 'Choice 1', 'choice2' => 'Choice 2', 'choice3' => 'Choice 3']
);
```
!!! Notes
    - The `choices` property may not be relevant to some Field classes such as the `BooleanField` class.
    - The human-readable choice names will only be used for documentation purposes where applicable.
    - In the case of a `many` enabled Field, choices defines which values are valid for any item within the array.

### choices_callable

The `choices_callable` property is used to define a callable function that will be used to determine the valid choices for the
field. The value assigned here must be a method available in the Model class. This should be used instead of the `choices`
property when the valid choices are dynamic.

!!! Notes
    - The `choices_callable` property is an alternative to the `choices` property. Both cannot be defined at the same time.

### allow_empty

The `allow_empty` property is used to define whether the field can be empty when creating or updating objects of the Model.
Empty refers to a value that is an empty string or an empty array in the case of a `many` enabled Field.

!!! Notes
    - If set to `true`, the `allow_empty` property will bypass other validations if the value assigned is empty.
    - pfSense represents empty values in the XML configuration as `<field></field>`. Use `allow_null` to allow `null` values and omit the field from the XML configuration altogether.

### allow_null

The `allow_null` property is used to define whether the field can be `null` when creating or updating objects of the Model.

!!! Notes
    - If set to `true`, the `allow_null` property will bypass other validations if the value assigned is `null`.
    - pfSense represents `null` values in the XML configuration as an omitted field. Use `allow_empty` to allow empty values and represent them as `<field></field>` in the XML configuration.

### unique

The `unique` property is used to define whether the field must have a unique value when creating or updating objects of the Model.
If any other object of the Model already has the same value assigned to the field, the validation will fail.

### editable

The `editable` property is used to define whether the field can be edited when updating objects of the Model. If `false`, the
field will be read-only and cannot be changed after the object is initially created.

### read_only

The `read_only` property is used to define whether the field is read-only and cannot be set when creating or updating objects.

!!! Notes
    - The `read_only` and `write_only` properties are mutually exclusive and cannot both be assigned to single Field object.

### write_only

The `write_only` property is used to define whether the field is write-only and cannot be read when retrieving objects.

!!! Notes
    - The `read_only` and `write_only` properties are mutually exclusive and cannot both be assigned to single Field object.

### sensitive

The `sensitive` property is used to define whether the field contains sensitive information that should be masked when
displayed in the web GUI. This only applies to Models that are displayed via a [Form](https://pfrest.org/php-docs/classes/RESTAPI-Core-Form.html).

### representation_only

The `representation_only` property is used to define whether the field is only used for representation purposes and does
not have a corresponding field in the XML configuration. This is useful for fields that are calculated or derived from
other fields or methods.

!!! Important
    Fields with the `representation_only` property must have a corresponding `from_internal_<field_name>()` method defined in the Model class OR must be assigned by another field's `validate_FIELD_NAME()` method.

### verbose_name

The `verbose_name` property is used to define the human-readable name of the field. This name will be used if the Field is
displayed via a [Form](https://pfrest.org/php-docs/classes/RESTAPI-Core-Form.html). If not defined, the name will be
automatically generated on a best guess basis using the Field property name.

!!! Notes
    - A `verbose_name` is typically only required when the automatically generated name is not suitable.

### verbose_name_plural

The `verbose_name_plural` property is used to define the human-readable plural name of the field. This name will be used
if the Field is displayed via a [Form](https://pfrest.org/php-docs/classes/RESTAPI-Core-Form.html). If not defined, the plural name will be
automatically generated on a best guess basis.

!!! Notes
    - A `verbose_name_plural` is typically only required when the automatically generated plural name is not suitable.

### internal_name

The `internal_name` property is used to define the XML configuration field name that the Field object represents. When
the Model writes or reads objects from the XML configuration, this will define the field name that is used.

!!! Notes
    - The `internal_name` property is only required when the Field object property name does not match the XML configuration field name.

### internal_namespace

When the XML configuration has this Field nested under a parent XML element, the `internal_namespace` property is used to
define the parent XML element name. For example, if the XML configuration is structured as follows:

```xml
<parent>
    <child>value</child>
</parent>
```

The `internal_namespace` property would be set to `parent` and your Field object would be named `child`.

!!! Notes
    - The `internal_namespace` property is only required when the Field object is nested under a parent XML element.

### referenced_by

The `referenced_by` property is used to define other Model classes that reference this Field object. This can be used to
block the deletion of the Model object if it is referenced by another Model object. This property must be an associative
array where the key is the Model class name that references this Field object, and the value is the Field object name in
the referencing Model class.

!!! Notes
    - The Model class names defined in the `referenced_by` property must be class's short name, not the fully qualified class name.

### conditions

In the case that your Field should only be included if another Field in the Model has (or does not have) a specific value,
you can define `conditions` for the Field. 

!!! Notes
    - If a Field's conditions are not met, the Field will be ignored by the Model and automatically assume a `null` value.

There are multiple types of conditions you can define:

#### Exact Value

If you need the Field to be included only if another Field has an exact value, you can define the condition as follows:

```php
$this->type = new StringField(
    required: true,
    choices: ['specific_value', 'other_value']
);
$this->field = new StringField(
    required: true,
    conditions: [
        'type' => 'specific_value'
    ]
);
```

#### Not Equal

If you need the Field to be included only if another Field does not have a specific value, you can define the condition
with the conditions key prefixed with an exclamation mark `!`:

```php
$this->type = new StringField(
    required: true,
    choices: ['specific_value', 'other_value']
);
$this->field = new StringField(
    required: true,
    conditions: [
        '!type' => 'specific_value'
    ]
);
```

#### One of Many

If you need the Field to be included only if another Field's value is one of many values, you can define the condition where 
the condition's value is an array of values:

```php
$this->type = new StringField(
    required: true,
    choices: ['this_value', 'or_this_value', 'but_not_this_value']
);
$this->field = new StringField(
    required: true,
    conditions: [
        'type' => ['this_value', 'or_this_value']
    ]
);
```

#### Not One of Many

If you need the Field to be included only if another Field's value is not one of many values, you can define the condition
where the condition's value is an array of values and the condition's key is prefixed with an exclamation mark `!`:

```php
$this->type = new StringField(
    required: true,
    choices: ['not_this_value', 'and_not_this_value', 'this_value']
);
$this->field = new StringField(
    required: true,
    conditions: [
        '!type' => ['not_this_value', 'and_not_this_value']
    ]
);
```

### validators

The `validators` property is used to define an array of Validator objects that will be used to validate the field's value.
Validators are reusable objects that make it simple to apply specific validations to Field objects. An up-to-date list of
available Validator classes and their associated properties can be found in the [PHP reference](https://pfrest.org/php-docs/namespaces/restapi-validators.html).
You can also build your own custom Validator classes, see the [Building Custom Validator Classes](BUILDING_CUSTOM_VALIDATOR_CLASSES.md) guide for more information.

### help_text

The `help_text` property is used to define a human-readable description of the field. This description will be used in
documentation and other areas where a human-readable description is required.

## Adding Extra Validation

In some cases, you may need to add extra validation to the Model class that cannot be achieved using the Field objects
alone. This can be done by implementing the `validate_FIELD_NAME()` and/or `validate_extra()` methods in the Model class:

### validate_FIELD_NAME()

The `validate_FIELD_NAME()` method is used to define custom validation logic for a specific Field object. This method is
automatically called after the initial Field object validation has been performed, but before `validate_extra()` is 
called. Simply create a new method within your Model class with the target Field object's name prefixed by `validate_`. 
The method must accept a single argument that matches the Field object's type, returns the validated value. During 
validation, you can throw any applicable [Response](https://pfrest.org/php-docs/namespaces/restapi-responses.html) 
exception class to return errors, but typically the [ValidationError](https://pfrest.org/php-docs/classes/RESTAPI-Responses-ValidationError.html) 
exception is used here.

!!! Important
    In the case of a `many` enabled Field, the `validate_FIELD_NAME()` will be called for each item in the array value individually.

Example:

```php
<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * TODO: Add a description of your Model class here.
 */
class MyCustomModel extends Model {
    public StringField $name;
    public IntegerField $timeout;
    public BooleanField $enabled;
    
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'path/to/model/objects/in/config';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            help_text: 'Demonstrates an example StringField.',
        );
        $this->timeout = new IntegerField(
            default: 30,
            help_text: 'Demonstrates an example IntegerField.',
        );
        $this->enabled = new BooleanField(
            default: true,
            indicates_true: "on", // The value stored in the XML configuration when the field is true
            indicates_false: "off", // The value stored in the XML configuration when the field is false
            help_text: 'Demonstrates an example BooleanField.',
        );
        
        parent::__construct($id, $parent_id, $data, ...$options);
    }
    
    /**
    * Adds extra validation to the `name` field.
    * @param string $name The incoming `name` field value to be validated.
    * @return string The validated `name` field value to be assigned.
    */
    public function validate_name(string $name): string {
        if ($name === 'INVALID_NAME') {
            throw new ValidationError(
                message: "The `name` field value `$name` is invalid.",
                response_id: 'MY_CUSTOM_MODEL_INVALID_NAME'
            );
        }
        return $name;
    }
}
```

!!! Tips
    - The `validate_FIELD_NAME()` method can be used to modify the value of the Field if necessary.
    - Since this method is called after the initial Field object validation, you can refer to other Field object values
      for validation by calling the desired Field object's `value` property directly (e.g. `$this->some_field->value`).

### validate_extra()

The `validate_extra()` method is used to define custom validation logic that is not specific to a single Field object, 
but rather validates the Model object as a whole. This method is automatically called after all other Field object 
validation has been performed. During validation, you can throw any applicable [Response](https://pfrest.org/php-docs/namespaces/restapi-responses.html)
exception class to return errors, but typically the [ValidationError](https://pfrest.org/php-docs/classes/RESTAPI-Responses-ValidationError.html)
exception is used here. There are no arguments or return values for this method.

!!! Tips
    - It is recommended to use `validate_extra()` instead of `validate_FIELD_NAME()` when the validation logic requires
    the values of multiple Field objects to be considered.

Example:

```php
<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * TODO: Add a description of your Model class here.
 */
class MyCustomModel extends Model {
    public StringField $name;
    public IntegerField $timeout;
    public BooleanField $enabled;
    
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'path/to/model/objects/in/config';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            help_text: 'Demonstrates an example StringField.',
        );
        $this->timeout = new IntegerField(
            default: 30,
            help_text: 'Demonstrates an example IntegerField.',
        );
        $this->enabled = new BooleanField(
            default: true,
            indicates_true: "on", // The value stored in the XML configuration when the field is true
            indicates_false: "off", // The value stored in the XML configuration when the field is false
            help_text: 'Demonstrates an example BooleanField.',
        );
        
        parent::__construct($id, $parent_id, $data, ...$options);
    }
    
    /**
    * Adds extra validation to the entire Model object
    */
    public function validate_extra(): void {
        if ($this->name->value === 'INVALID_NAME' and $this->enabled->value === true) {
            throw new ValidationError(
                message: "This Model object cannot be enabled with the `name` field value `INVALID_NAME`.",
                response_id: 'MY_CUSTOM_CANNOT_ENABLE_MODEL_WITH_INVALID_NAME'
            );
        }
    }
```

## Implementing Apply Methods

In most cases, simply writing a Model object to the XML configuration is not enough to apply the changes to the system.
The `apply()` method is used to apply the changes made to the Model object to the system. This method is automatically
called after a Model object is created, updated, or deleted. The `apply()` method should contain the logic to apply the
changes made to the Model object to the system. There are a few different types of `apply()` methods that can be used:

### apply()

The standard `apply()` method is used to apply changes to the system when a Model object is created, updated, or deleted.
This method will be used by default if no other `apply_*()` method is defined.

### apply_create()

The `apply_create()` method is used to apply changes to the system when a Model object is created. This method will 
default to the `apply()` method if not defined.

### apply_update()

The `apply_update()` method is used to apply changes to the system when a Model object is updated. This method will
default to the `apply()` method if not defined.

### apply_delete()

The `apply_delete()` method is used to apply changes to the system when a Model object is deleted. This method will
default to the `apply()` method if not defined.

### apply_replace_all()

The `apply_replace_all()` method is used to apply changes to the system when all Model objects are replaced. This method
will default to the `apply()` method if not defined.

### Utilizing Dispatchers

When implementing any apply method, it is important to consider the amount of time it may take to apply the changes. If
the apply method will take a long time to complete, it is recommended to create a new [Dispatcher class](https://pfrest.org/php-docs/classes/RESTAPI-Core-Dispatcher.html) for your Model
and call the Dispatcher in your apply method like so:

```php
public function apply(): void {
    $dispatcher = new MyCustomDispatcher(async: $this->async);
    $dispatcher->spawn_process();
}
```

For more information on how to build custom Dispatcher classes, refer to the [Building Custom Dispatcher Classes](https://pfrest.org/docs/BUILDING_CUSTOM_DISPATCHER_CLASSES.html) documentation.

!!! Notes
    - The `async` property is used to define whether the Dispatcher should run asynchronously. If `true`, the Dispatcher
      will run in the background and the apply method will return immediately. If `false`, the Dispatcher will run
      synchronously and the apply method will wait for the Dispatcher to complete before returning.

### Tracking Original Model State

In certain situations, you may need to reference the Model object as it was before any changes were made. This can be
done by using the `$this->initial_object` property of your Model class. This property will contain the Model object as
it was before any changes were made.

!!! Note
    After the Model is created, updated, or delete, the `$this->initial_object` property will be updated to reflect the
    current state of the Model object.

## Utilizing the Model Class

After you have defined your custom Model class, you can call the Model within PHP code to interact with the pfSense
configuration it represents. 

!!! Important
    This documentations only applies to interacting the Model objects within the package internally (PHP). It does not cover
    how to interact with the Model objects via the REST API.

Below are examples of how to create, read, update, and delete objects of the Model class:



### Create an Object

Once you have constructed the Model object and assigned the desired field values, you can call the `create()` method to
validate the object and write the object to the XML configuration:

```php
<?php

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Models\MyCustomModel;

# Create a new Model object
$my_custom_model = new MyCustomModel(name: "EXAMPLE_NAME", timeout: 60, enabled: true);
$my_custom_model->create();
```

### Read an Object

To read an existing object, simply construct the Model object with an `id` for `many` enabled Models or without an `id`
for `many` disabled Models. Then call the `read()` method to read the object from the XML configuration:

```php
<?php

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Models\MyCustomModel;

# Read an existing Model object
$my_custom_model = new MyCustomModel(id: 1);
```

### Update an Object

To update an existing object, construct the Model object with an `id` for `many` enabled Models or without an `id` for
`many` disabled Models. Then assign the desired field values and call the `update()` method to validate the object and
write the object to the XML configuration:

```php
<?php

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Models\MyCustomModel;

# Update an existing Model object
$my_custom_model = new MyCustomModel(id: 1, name: "NEW_NAME", timeout: 90, enabled: false);
$my_custom_model->update();
```

### Delete an Object

To delete an existing object, construct the Model object with an `id` for `many` enabled Models or without an `id` for
`many` disabled Models. Then call the `delete()` method to delete the object from the XML configuration:

```php
<?php

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Models\MyCustomModel;

# Delete an existing Model object
$my_custom_model = new MyCustomModel(id: 1);
$my_custom_model->delete();
```

## Non-configuration Based Models

In some cases, you may need to create a Model class that does not interact with the pfSense configuration. Some common
examples of non-configuration based Models include Models that fetch real-time data, interact with services directly, or
execute certain commands. To create a non-configuration based Model, you can omit the `config_path` property and define
an `internal_callable` property instead. The `internal_callable` property is used to define a callable function that will
be used to fetch the Model's data internally. This function must be defined in the Model class and must return an array 
of objects for `many` enabled Models or a single object for `many` disabled Models.

Below is an example of how to define a non-configuration based Model class:

```php
<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;

/**
TODO: Add a description of your Model class here.
*/
class MyCustomModel extends Model {
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'fetch_data';
        $this->many = true;
        
        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            help_text: 'Demonstrates an example StringField.',
        );
        $this->timeout = new IntegerField(
            default: 30,
            help_text: 'Demonstrates an example IntegerField.',
        );
        $this->enabled = new BooleanField(
            default: true,
            indicates_true: "on", // The value stored in the XML configuration when the field is true
            indicates_false: "off", // The value stored in the XML configuration when the field is false
            help_text: 'Demonstrates an example BooleanField.',
        );
        
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
    * Fetches data for the Model object.
    * @return array The data for the Model object.
    */
    public function fetch_data(): array {
        return [
            [
                'name' => 'EXAMPLE_NAME_1',
                'timeout' => 60,
                'enabled' => true,
            ],
            [
                'name' => 'EXAMPLE_NAME_2',
                'timeout' => 90,
                'enabled' => false,
            ],
        ];
    }
}
```

!!! Tip
    In some cases, it may be desirable to have your internal callable handle pagination to ensure the large data sets
    are handled more efficiently. You can delegate pagination control to your internal callable by defining the 
    `limit`, `offset` and `reverse` parameters in your internal callable. All three parameters must be present.
    When these parameters are defined, is the responsibility of the internal callable to handle pagination and return 
    the correct subset of data based on the parameters' values.

### Defining CRUD Methods

When creating a non-configuration based Model, the default `create()`, `update()`, and `delete()` methods will throw an error. To enable these methods, you must define the `_create()`, `_update()` and `_delete()` yourself. Below is an
example of how to define custom CRUD methods for a non-configuration based Model:

```php
<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;

/**
TODO: Add a description of your Model class here.
*/
class MyCustomModel extends Model {
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'fetch_data';
        $this->many = true;
        
        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            help_text: 'Demonstrates an example StringField.',
        );
        $this->timeout = new IntegerField(
            default: 30,
            help_text: 'Demonstrates an example IntegerField.',
        );
        $this->enabled = new BooleanField(
            default: true,
            indicates_true: "on", // The value stored in the XML configuration when the field is true
            indicates_false: "off", // The value stored in the XML configuration when the field is false
            help_text: 'Demonstrates an example BooleanField.',
        );
        
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
    * Fetches data for the Model object.
    * @return array The data for the Model object.
    */
    public function fetch_data(): array {
        return [
            [
                'name' => 'EXAMPLE_NAME_1',
                'timeout' => 60,
                'enabled' => true,
            ],
            [
                'name' => 'EXAMPLE_NAME_2',
                'timeout' => 90,
                'enabled' => false,
            ],
        ];
    }
    
    /**
    * Defines steps to 'create' an instance of this Model.
    */
    public function _create(): void {
        # TODO: Add custom logic to create an object
    }
    
    /**
    * Defines steps to 'update' an instance of this Model.
    */
    public function _update(): void {
        # TODO: Add custom logic to update an object
    }   
    
    /**
    * Defines steps to 'delete' an instance of this Model.
    */
    public function _delete(): void {
        # TODO: Add custom logic to delete an object
    }
}
```

Once you have defined the applicable CRUD methods, you can call them in the same way as the standard CRUD methods:

```php
<?php

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Models\MyCustomModel;

# Create a new Model object
$my_custom_model = new MyCustomModel();
$my_custom_model->create();
```

!!! Important
    You should not call the `_create()`, `_update()`, or `_delete()` methods directly. These methods are automatically called
    when the create(), update(), or delete() methods are called.

## Exposing Model via REST API Endpoint

Once you have defined your Model class, you can expose it via a REST API endpoint. To do this, you must create a new Endpoint
class. Refer to the [Building Custom Endpoint Classes](BUILDING_CUSTOM_ENDPOINT_CLASSES.md) documentation for more information.

## Examples

You can find examples of fully implemented Model classes in the [PHP reference](https://pfrest.org/php-docs/namespaces/restapi-models.html).
Select the Model class you are interested in to view the class's PHPDoc documentation, and then click on the
`<>` symbol next to the class name to view the class's source code.
