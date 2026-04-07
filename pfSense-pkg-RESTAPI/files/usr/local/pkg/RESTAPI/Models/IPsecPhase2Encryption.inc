<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\IPsecApplyDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model that represents IPsec Phase 2 Encryption settings.
 */
class IPsecPhase2Encryption extends Model {
    public StringField $name;
    public IntegerField $keylen;

    public function __construct(mixed $id = null, mixed $parent_id = null, array $data = [], mixed ...$options) {
        # Obtain global p2 algorithm variables
        global $p2_ealgos;

        # Set model attributes
        $this->parent_model_class = 'IPsecPhase2';
        $this->config_path = 'encryption-algorithm-option';
        $this->subsystem = 'ipsec';
        $this->many = true;
        $this->many_minimum = 1;
        $this->verbose_name = 'IPsec Phase 2 Encryption';

        # Set model fields
        $this->name = new StringField(
            required: true,
            choices: array_keys($p2_ealgos),
            internal_name: 'name',
            verbose_name: 'Name',
            help_text: 'The name of the encryption algorithm to use for this P2 encryption item.',
        );
        $this->keylen = new IntegerField(
            required: true,
            internal_name: 'keylen',
            conditions: ['name' => $this->get_keylen_enabled_algos()],
            verbose_name: 'Key Length',
            help_text: 'The key length for the encryption algorithm.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `keylen` field.
     * @param int $keylen The incoming value to be validated.
     * @returns int The validated value to be assigned.
     * @throws ValidationError When the $keylen is not supported by the
     * `name` field's assigned value.
     */
    public function validate_keylen(int $keylen): int {
        # Variables
        $supported_keylens = $this->get_supported_keylens(name: $this->name->value);

        # Throw a validation error if this $keylen is not supported for the assigned algo
        if (!in_array($keylen, $supported_keylens)) {
            throw new ValidationError(
                message: "Field `keylen` value `$keylen` is not valid for the `{$this->name->value}` algorithm.",
                response_id: 'IPSEC_PHASE_2_ENCRYPTION_ALGORITHM_KEYLEN_INVALID_CHOICE',
            );
        }

        return $keylen;
    }

    /**
     * Obtains all supported key lengths for an encryption algorithm with a provided algorithm name.
     * @param string $name The encryption algorithm name to obtain key lengths for.
     * @returns array An array of supported key lengths for this encryption algorithm.
     */
    public static function get_supported_keylens(string $name): array {
        global $p2_ealgos;

        # Throw an error if an encryption algorithm does not exist with this name.
        if (!array_key_exists($name, $p2_ealgos)) {
            throw new ValidationError(
                message: "Could not obtain supported key lengths for unknown algorithm `$name`.",
                response_id: 'IPSEC_PHASE_2_ENCRYPTION_COULD_NOT_GET_KEYLENS_FOR_UNKNOWN_ALGO',
            );
        }

        # Obtain the attributes for the encryption algorithm with this $name
        $p2_ealgo = $p2_ealgos[$name];
        $key_lens = [];

        # Determine available key lengths for this algo when selections are available
        if (array_key_exists('keysel', $p2_ealgo)) {
            # Determine the lowest and highest key lengths available, as well as intermittent steps.
            $key_len = $p2_ealgo['keysel']['lo'];
            $key_len_max = $p2_ealgo['keysel']['hi'];
            $key_len_step = $p2_ealgo['keysel']['step'];

            # Loop until key length steps are exhausted
            while ($key_len <= $key_len_max) {
                $key_lens[] = $key_len;
                $key_len = $key_len + $key_len_step;
            }
        }

        return $key_lens;
    }

    /**
     * Obtains an array of encryption algorithm names that support variable key length selection.
     * @returns array An array of encryption algorithm names that support variable key length selection.
     */
    public static function get_keylen_enabled_algos(): array {
        global $p2_ealgos;
        $supports_keylen = [];

        # Loop through each support encryption algorithm and capture those who support variable key length selection
        foreach ($p2_ealgos as $p2_ealgo_name => $p2_ealgo) {
            if (array_key_exists('keysel', $p2_ealgo)) {
                $supports_keylen[] = $p2_ealgo_name;
            }
        }

        return $supports_keylen;
    }

    /**
     * Applies pending changes to IPsec.
     */
    public function apply(): void {
        (new IPsecApplyDispatcher(async: $this->async))->spawn_process();
    }
}
