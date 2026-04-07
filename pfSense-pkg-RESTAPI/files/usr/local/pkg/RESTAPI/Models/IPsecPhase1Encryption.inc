<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\IPsecApplyDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model that represents IPsec Phase 1 Encryption settings.
 */
class IPsecPhase1Encryption extends Model {
    public StringField $encryption_algorithm_name;
    public IntegerField $encryption_algorithm_keylen;
    public StringField $hash_algorithm;
    public IntegerField $dhgroup;
    public StringField $prf_algorithm;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Obtain global p1 algorithm variables
        global $p1_ealgos;
        global $p1_halgos;
        global $p1_dhgroups;

        # Set model attributes
        $this->parent_model_class = 'IPsecPhase1';
        $this->config_path = 'encryption/item';
        $this->subsystem = 'ipsec';
        $this->many = true;
        $this->many_minimum = 1;
        $this->verbose_name = 'IPsec Phase 1 Encryption';

        # Set model fields
        $this->encryption_algorithm_name = new StringField(
            required: true,
            choices: array_keys($p1_ealgos),
            internal_name: 'name',
            internal_namespace: 'encryption-algorithm',
            verbose_name: 'Encryption Algorithm',
            help_text: 'The name of the encryption algorithm to use for this P1 encryption item.',
        );
        $this->encryption_algorithm_keylen = new IntegerField(
            required: true,
            internal_name: 'keylen',
            internal_namespace: 'encryption-algorithm',
            conditions: ['encryption_algorithm_name' => $this->get_keylen_enabled_algos()],
            verbose_name: 'Encryption Key Length',
            help_text: 'The key length for the encryption algorithm.',
        );
        $this->hash_algorithm = new StringField(
            required: true,
            choices: array_keys($p1_halgos),
            internal_name: 'hash-algorithm',
            verbose_name: 'Hash Algorithm',
            help_text: 'The hash algorithm to use for this P1 encryption item.',
        );
        $this->dhgroup = new IntegerField(
            required: true,
            choices: array_keys($p1_dhgroups),
            verbose_name: 'DH Group',
            help_text: 'The Diffie-Hellman (DH) group to use for this P1 encryption item.',
        );
        $this->prf_algorithm = new StringField(
            default: 'sha256',
            choices: array_keys($p1_halgos),
            internal_name: 'prf-algorithm',
            verbose_name: 'PRF Algorithm',
            help_text: 'The PRF algorithm to use for this P1 encryption item. This value has no affect unless the ' .
                'P1 entry has PRF enabled.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `encryption_algorithm_keylen` field.
     * @param int $encryption_algorithm_keylen The incoming value to be validated.
     * @returns int The validated value to be assigned.
     * @throws ValidationError When the $encryption_algorithm_keylen is not supported by the
     * `encryption_algorithm_name` field's assigned value.
     */
    public function validate_encryption_algorithm_keylen(int $encryption_algorithm_keylen): int {
        # Variables
        $supported_keylens = $this->get_supported_keylens(name: $this->encryption_algorithm_name->value);

        # Throw a validation error if this $encryption_algorithm_keylen is not supported for the assigned algo
        if (!in_array($encryption_algorithm_keylen, $supported_keylens)) {
            throw new ValidationError(
                message: "Field `encryption_algorithm_keylen` value `$encryption_algorithm_keylen` is not valid for " .
                    "`{$this->encryption_algorithm_name->value}` algorithm.",
                response_id: 'IPSEC_PHASE_1_ENCRYPTION_ALGORITHM_KEYLEN_INVALID_CHOICE',
            );
        }

        return $encryption_algorithm_keylen;
    }

    /**
     * Obtains all supported key lengths for an encryption algorithm with a provided algorithm name.
     * @param string $name The encryption algorithm name to obtain key lengths for.
     * @returns array An array of supported key lengths for this encryption algorithm.
     */
    public static function get_supported_keylens(string $name): array {
        global $p1_ealgos;

        # Throw an error if an encryption algorithm does not exist with this name.
        if (!array_key_exists($name, $p1_ealgos)) {
            throw new ValidationError(
                message: "Could not obtain supported key lengths for unknown algorithm `$name`.",
                response_id: 'IPSEC_PHASE1_ENCRYPTION_COULD_NOT_GET_KEYLENS_FOR_UNKNOWN_ALGO',
            );
        }

        # Obtain the attributes for the encryption algorithm with this $name
        $p1_ealgo = $p1_ealgos[$name];
        $key_lens = [];

        # Determine available key lengths for this algo when selections are available
        if (array_key_exists('keysel', $p1_ealgo)) {
            # Determine the lowest and highest key lengths available, as well as intermittent steps.
            $key_len = $p1_ealgo['keysel']['lo'];
            $key_len_max = $p1_ealgo['keysel']['hi'];
            $key_len_step = $p1_ealgo['keysel']['step'];

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
        global $p1_ealgos;
        $supports_keylen = [];

        # Loop through each support encryption algorithm and capture those who support variable key length selection
        foreach ($p1_ealgos as $p1_ealgo_name => $p1_ealgo) {
            if (array_key_exists('keysel', $p1_ealgo)) {
                $supports_keylen[] = $p1_ealgo_name;
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
