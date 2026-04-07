<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\DateTimeField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for interacting with the local users on this system.
 */
class User extends Model {
    public StringField $name;
    public StringField $password;
    public IntegerField $uid;
    public StringField $scope;
    public StringField $priv;
    public BooleanField $disabled;
    public StringField $descr;
    public DateTimeField $expires;
    public ForeignModelField $cert;
    public Base64Field $authorizedkeys;
    public StringField $ipsecpsk;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'system/user';
        $this->many = true;
        $this->always_apply = true;

        # Set model Fields
        $this->name = new StringField(
            required: true,
            unique: true,
            maximum_length: 32,
            validators: [
                new RegexValidator(pattern: "/^[a-zA-Z0-9\.\-_]+$/", error_msg: 'Value contains invalid characters.'),
            ],
            verbose_name: 'Name',
            help_text: 'The username of this local user.',
        );
        $this->password = new StringField(
            required: true,
            sensitive: true,
            internal_name: $this->get_config('system/webgui/pwhash', 'bcrypt') . '-hash',
            verbose_name: 'Password',
            help_text: 'The password of this local user.',
        );
        $this->uid = new IntegerField(
            unique: true,
            default: (int) $this->get_config('system/nextuid'),
            editable: false,
            read_only: true,
            verbose_name: 'UID',
            help_text: 'The UID of this local user. This value is automatically assigned and cannot be changed.',
        );
        $this->scope = new StringField(
            default: 'user',
            editable: false,
            read_only: true,
            verbose_name: 'Scope',
            help_text: 'The scope of this local user. This value is automatically assigned and cannot be changed.',
        );
        $this->priv = new StringField(
            default: [],
            allow_empty: true,
            many: true,
            delimiter: null,
            verbose_name: 'Privileges',
            help_text: 'The privileges assigned to this local user.',
        );
        $this->disabled = new BooleanField(default: false, verbose_name: 'Disabled', help_text: 'Disable this local user.');
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The full descriptive name for this local user.',
        );
        $this->expires = new DateTimeField(
            default: '',
            datetime_format: 'm/d/Y',
            allow_empty: true,
            verbose_name: 'Expires',
            help_text: 'The expiration date for this user in mm/dd/YYYY format. Use empty string for no ' .
                'expiration',
        );
        $this->cert = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            default: [],
            allow_empty: true,
            many: true,
            delimiter: null,
            verbose_name: 'Certificate',
            help_text: 'The user certificates to assign this user. Items must be existing certificate `refid`s.',
        );
        $this->authorizedkeys = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'Authorized Keys',
            help_text: 'The SSH authorized keys to assign this user.',
        );
        $this->ipsecpsk = new StringField(
            default: '',
            allow_empty: true,
            validators: [
                new RegexValidator(pattern: "/^[[:ascii:]]*$/", error_msg: 'Value contains invalid characters.'),
            ],
            verbose_name: 'IPsec Pre-Shared Key',
            help_text: 'The IPsec pre-shared key to assign this user.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds additional validation to the `name` field.
     * @param string $name The incoming value to be validated
     * @return string The validated value to be assigned.
     */
    public function validate_name(string $name): string {
        # Do not allow users to use usernames that are reserved by the system
        if ($this->is_username_reserved($name) and $name !== $this->initial_object->name->value) {
            throw new ValidationError(
                message: "Field `username` cannot use system reserved username `$name`.",
                response_id: 'USER_CANNOT_USE_RESERVED_USERNAME',
            );
        }

        return $name;
    }

    /**
     * Adds additional validation to the `password` field. This method is responsible for hashing the password.
     * @param string $password The incoming value to be validated
     * @return string The validated value to be assigned.
     */
    public function validate_password(string $password): string {
        # Hash the password if the password has changed
        if ($this->initial_object->password->value !== $password) {
            $hash = [];
            local_user_set_password($hash, $password);
            return $hash['item'][$this->password->internal_name];
        }

        return $password;
    }

    /**
     * Adds additional validation to the `priv` field.
     * @param string $priv The incoming value to be validated
     * @return string The validated value to be assigned.
     */
    public function validate_priv(string $priv): string {
        global $priv_list;

        # Ensure requested privileges are valid privilege names
        if (!array_key_exists($priv, $priv_list)) {
            throw new NotFoundError(
                message: "Field `priv` contains unknown privilege `$priv`",
                response_id: 'USER_PRIV_NOT_FOUND',
            );
        }

        return $priv;
    }

    /**
     * Adds additional validation to the `cert` field.
     * @param string $cert The incoming value to be validated
     * @return string The validated value to be assigned.
     */
    public function validate_cert(string $cert): string {
        # TODO: Remove this function once the Certificate model is developed and this field is converted to a
        # TODO: ForeignModelField
        # Loop through all certificates and check for matches
        foreach ($this->get_config('cert', []) as $cert_config) {
            # Check if this certificate matches the specified ref id
            if ($cert_config['refid'] === $cert) {
                return $cert;
            }
        }

        throw new NotFoundError(
            message: "Field `cert` contains non-existing certificate `refid` value `$cert`",
            response_id: 'USER_CERT_NOT_FOUND',
        );
    }

    /**
     * Extend the _create() method so it bumps the next UID before creating the object
     */
    public function _create() {
        # Bump the nextuid in config to one greater than this UID
        $this->set_config('system/nextuid', $this->uid->value + 1);
        parent::_create();
    }

    /**
     * Extends the _delete() method to ensure system users cannot be deleted.
     */
    public function _delete() {
        # Do not allow system users to be deleted
        # TODO: replace this with a protected_model_query
        if ($this->scope->value !== 'user') {
            throw new ValidationError(
                message: "System user `{$this->name->value}` cannot be deleted.",
                response_id: 'USER_WITH_SYSTEM_SCOPE_CANNOT_BE_DELETED',
            );
        }
        parent::_delete();
    }

    /**
     * Apply the creation of this User.
     */
    public function apply_create() {
        local_user_set($this->to_internal());
    }

    /**
     * Apply the update of this User.
     */
    public function apply_update() {
        local_user_del($this->initial_object->to_internal());
        local_user_set($this->to_internal());
    }

    /**
     * Apply the deletion of this User.
     */
    protected function apply_delete() {
        local_user_del($this->to_internal());
    }

    /**
     * Checks if a provided username is reserved by the system.
     * @param string $username The username to check.
     * @return bool `true` if the username is reserved, `false` if it is not.
     */
    public static function is_username_reserved(string $username): bool {
        # Open the /etc/passwd file to read all system users
        $sys_users = explode(PHP_EOL, file_get_contents('/etc/passwd'));

        # Loop through each system user and check if the username is reserved
        foreach ($sys_users as $sys_user_ent) {
            $sys_username = explode(':', $sys_user_ent)[0];
            if ($sys_username == $username) {
                return true;
            }
        }
        return false;
    }
}
