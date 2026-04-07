<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\TrafficShaperApplyDispatcher;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interacting with the traffic shaper limiter bandwidth.
 */
class TrafficShaperLimiterBandwidth extends Model {
    public IntegerField $bw;
    public StringField $bwscale;
    public ForeignModelField $bwsched;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'TrafficShaperLimiter';
        $this->config_path = 'bandwidth/item';
        $this->many = true;
        $this->many_minimum = 1;

        # Set model fields
        $this->bw = new IntegerField(
            required: true,
            minimum: 1,
            verbose_name: 'Bandwidth',
            help_text: 'The amount of bandwidth this profile allows.',
        );
        $this->bwscale = new StringField(
            required: true,
            choices: ['b', 'Kb', 'Mb'],
            verbose_name: 'Bandwidth Scale',
            help_text: 'The scale factor of the `bw` fields value.',
        );
        $this->bwsched = new ForeignModelField(
            model_name: 'FirewallSchedule',
            model_field: 'name',
            allowed_keywords: ['none'],
            default: 'none',
            verbose_name: 'Bandwidth Schedule',
            help_text: 'The schedule to assign this bandwidth profile. When this firewall schedule is active, ' .
                'this bandwidth profile will be used.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Reloads the filter, and resets applicable RRD graphs
     */
    public function apply(): void {
        (new TrafficShaperApplyDispatcher(async: $this->async))->spawn_process();
    }
}
