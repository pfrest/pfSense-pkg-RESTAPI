<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a model that interacts with BIND zone records.
 */
class BINDZoneRecord extends Model {
    public StringField $name;
    public StringField $type;
    public StringField $rdata;
    public IntegerField $priority;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'BINDZone';
        $this->config_path = 'row';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'BIND Zone Record';

        # Set model fields
        $this->name = new StringField(
            required: true,
            internal_name: 'hostname',
            help_text: 'The domain name for this record.',
        );
        $this->type = new StringField(
            required: true,
            choices: ['A', 'AAAA', 'CNAME', 'MX', 'NS', 'LOC', 'PTR', 'SRV', 'TXT', 'SPF'],
            internal_name: 'hosttype',
            help_text: 'The type of record.',
        );
        $this->rdata = new StringField(
            required: true,
            allow_empty: true,
            internal_name: 'hostdst',
            help_text: 'The data for this record. This can be an IP address, domain name, or other data depending on the record type.',
        );
        $this->priority = new IntegerField(
            required: true,
            internal_name: 'hostvalue',
            conditions: ['type' => ['MX', 'SRV']],
            help_text: 'The priority for this record.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the 'type' field.
     * @param string $type The incoming value being validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When the parent zone is a reverse zone and the record type is not NS or PTR.
     */
    public function validate_type(string $type): string {
        # Ensure reverse zones only have NS or PTR records
        if ($this->parent_model->reversev4->value and !in_array($type, ['NS', 'PTR'])) {
            throw new ValidationError(
                message: 'Field `type` must be `NS` or `PTR` when the parent zone is a reverse zone.',
                response_id: 'BIND_ZONE_RECORD_INVALID_REVERSE_TYPE',
            );
        }

        return $type;
    }

    /**
     * Applies changes to the BIND zone records
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
