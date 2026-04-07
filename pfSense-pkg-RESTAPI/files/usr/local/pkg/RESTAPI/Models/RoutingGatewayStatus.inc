<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\FloatField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the status of gateways on this system.
 */
class RoutingGatewayStatus extends Model {
    public ForeignModelField $name;
    public StringField $srcip;
    public StringField $monitorip;
    public FloatField $delay;
    public FloatField $stddev;
    public FloatField $loss;
    public StringField $status;
    public StringField $substatus;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_gateway_statuses';
        $this->many = true;

        # Set model fields
        $this->name = new ForeignModelField(
            model_name: 'RoutingGateway',
            model_field: 'name',
            unique: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Name',
            help_text: 'The name of the gateway this status corresponds to.',
        );
        $this->srcip = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Source IP',
            help_text: 'The current source IP being used to send monitoring probes to this gateway.',
        );
        $this->monitorip = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Monitor IP',
            help_text: 'The current IP being monitored for this gateway.',
        );
        $this->delay = new FloatField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Delay',
            help_text: 'The current latency (in milliseconds) for this gateway.',
        );
        $this->stddev = new FloatField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Stddev',
            help_text: 'The standard deviation in latency (in milliseconds) for this gateway.',
        );
        $this->loss = new FloatField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Loss',
            help_text: 'The current amount of packet loss (in percentage) for this gateway. This uses a whole ' .
                'percentage (0.0-100.0).',
        );
        $this->status = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Status',
            help_text: 'The current status of this gateway. Typically, this will indicate if the gateway is ' .
                'consider online or offline.',
        );
        $this->substatus = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Sub-Status',
            help_text: 'The current sub-status of this gateway. Typically, this provide details into problems this ' .
                'gateway is seeing such as latency or packet loss.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal gateway status values. This is this Model's internal callable.
     * @return array The array of
     */
    protected function get_gateway_statuses(): array {
        # Variables
        $gw_metrics_raw = return_gateways_status(true);
        $gw_metrics = [];

        # Loop through each gateway's metrics, format numeric values so they can be converted to floats
        foreach ($gw_metrics_raw as $gw) {
            $gw['delay'] = str_replace(['%', 'ms'], '', $gw['delay']);
            $gw['stddev'] = str_replace(['%', 'ms'], '', $gw['stddev']);
            $gw['loss'] = str_replace(['%', 'ms'], '', $gw['loss']);
            $gw_metrics[] = $gw;
        }

        return $gw_metrics;
    }
}
