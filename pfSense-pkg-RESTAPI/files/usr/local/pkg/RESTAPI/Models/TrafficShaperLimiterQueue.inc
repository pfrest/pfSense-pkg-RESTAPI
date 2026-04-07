<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\TrafficShaperApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FloatField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\FilterNameValidator;
use RESTAPI\Validators\UniqueFromForeignModelValidator;

/**
 * Defines a Model for interacting with the traffic shaper limiter queues.
 */
class TrafficShaperLimiterQueue extends Model {
    public StringField $name;
    public IntegerField $number;
    public BooleanField $enabled;
    public StringField $mask;
    public IntegerField $maskbits;
    public IntegerField $maskbitsv6;
    public IntegerField $qlimit;
    public BooleanField $ecn;
    public StringField $description;
    public StringField $aqm;
    public IntegerField $param_codel_target;
    public IntegerField $param_codel_interval;
    public IntegerField $param_pie_target;
    public IntegerField $param_pie_tupdate;
    public IntegerField $param_pie_alpha;
    public IntegerField $param_pie_beta;
    public IntegerField $param_pie_max_burst;
    public IntegerField $param_pie_max_ecnth;
    public BooleanField $pie_onoff;
    public BooleanField $pie_capdrop;
    public BooleanField $pie_qdelay;
    public BooleanField $pie_pderand;
    public IntegerField $param_red_w_q;
    public IntegerField $param_red_min_th;
    public IntegerField $param_red_max_th;
    public IntegerField $param_red_max_p;
    public IntegerField $param_gred_w_q;
    public IntegerField $param_gred_min_th;
    public IntegerField $param_gred_max_th;
    public IntegerField $param_gred_max_p;
    public IntegerField $weight;
    public FloatField $plr;
    public IntegerField $buckets;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'TrafficShaperLimiter';
        $this->config_path = 'queue';
        $this->subsystem = 'shaper';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [
                new FilterNameValidator(),
                new UniqueFromForeignModelValidator(model: 'TrafficShaperLimiter', field: 'name'),
            ],
            verbose_name: 'Name',
            help_text: 'The unique name for this limiter queue.',
        );
        $this->number = new IntegerField(
            unique: true,
            default: $this->get_next_number(),
            editable: false,
            read_only: true,
            verbose_name: 'Number',
            help_text: 'A unique number auto-assigned to this limiter. This is only used internally by the system and ' .
                'cannot be manually set or changed.',
        );
        $this->enabled = new BooleanField(
            default: false,
            indicates_true: 'on',
            verbose_name: 'Enabled',
            help_text: 'Enables or disables this limiter queue.',
        );
        $this->mask = new StringField(
            default: 'none',
            choices: ['none', 'srcaddress', 'dstaddress'],
            verbose_name: 'Mask',
            help_text: 'If `source` or `destination` slots is chosen a dynamic pipe with the bandwidth, delay, ' .
                'packet loss and queue size given above will be created for each source/destination IP address ' .
                'encountered, respectively. This makes it possible to easily specify bandwidth limits per host ' .
                'or subnet.',
        );
        $this->maskbits = new IntegerField(
            default: 32,
            minimum: 1,
            maximum: 32,
            conditions: ['mask' => ['srcaddress', 'dstaddress']],
            verbose_name: 'Mask Bits',
            help_text: 'The IPv4 mask bits to use when determine the scope of the dynamic pipe for IPv4 traffic.',
        );
        $this->maskbitsv6 = new IntegerField(
            default: 128,
            minimum: 1,
            maximum: 128,
            conditions: ['mask' => ['srcaddress', 'dstaddress']],
            verbose_name: 'IPv6 Mask Bits',
            help_text: 'The IPv6 mask bits to use when determine the scope of the dynamic pipe for IPv4 traffic.',
        );
        $this->qlimit = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            verbose_name: 'Queue Limit',
            help_text: "The length of the limiter's queue which the scheduler and AQM are responsible for. Set to " .
                '`null` to assume default.',
        );
        $this->ecn = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: [
                'aqm' => ['codel', 'pie', 'red', 'gred'],
                'sched' => ['fq_codel', 'fq_pie'],
            ],
            verbose_name: 'ECN',
            help_text: 'Enable or disable ECN. ECN sets a reserved TCP flag when the queue is nearing or exceeding ' .
                'capacity. Not all AQMs or schedulers support this.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'The verbose description for this limiter queue.',
        );
        $this->aqm = new StringField(
            required: true,
            choices: ['droptail', 'codel', 'pie', 'red', 'gred'],
            verbose_name: 'AQM',
            help_text: 'The Active Queue Management (AQM) algorithm to use for this queue. AQM is the intelligent ' .
                'drop of network packets inside the queue, when it becomes full or gets close to becoming full, ' .
                'with the goal of reducing network congestion.',
        );
        $this->param_codel_target = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'codel'],
            verbose_name: 'CoDel Target',
            help_text: 'The value for the CoDel target parameter.',
        );
        $this->param_codel_interval = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'codel'],
            verbose_name: 'CoDel Interval',
            help_text: 'The value for the CoDel interval parameter.',
        );
        $this->param_pie_target = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Target',
            help_text: 'The value for the PIE target parameter.',
        );
        $this->param_pie_tupdate = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Tupdate',
            help_text: 'The value for the PIE tupdate parameter.',
        );
        $this->param_pie_alpha = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Alpha',
            help_text: 'The value for the PIE alpha parameter.',
        );
        $this->param_pie_beta = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Beta',
            help_text: 'The value for the PIE beta parameter.',
        );
        $this->param_pie_max_burst = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Max Burst',
            help_text: 'The value for the PIE max_burst parameter.',
        );
        $this->param_pie_max_ecnth = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Max ECN Threshold',
            help_text: 'The value for the PIE ecnth parameter.',
        );
        $this->pie_onoff = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE On/Off',
            help_text: 'Enable or disable turning PIE on and off depending on queue load.',
        );
        $this->pie_capdrop = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Cap Drop',
            help_text: 'Enable or disable cap drop adjustment.',
        );
        $this->pie_qdelay = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE Queue Delay',
            help_text: 'Set queue delay type to timestamps (true) or departure rate estimation (false).',
        );
        $this->pie_pderand = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['aqm' => 'pie'],
            verbose_name: 'PIE De-randomization',
            help_text: 'Enable or disable drop probability de-randomisation.',
        );
        $this->param_red_w_q = new IntegerField(
            default: 1,
            conditions: ['aqm' => 'red'],
            verbose_name: 'RED Weight',
            help_text: 'The value for the RED w_q parameter.',
        );
        $this->param_red_min_th = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'red'],
            verbose_name: 'RED Min Threshold',
            help_text: 'The value for the RED min_th parameter.',
        );
        $this->param_red_max_th = new IntegerField(
            default: 1,
            conditions: ['aqm' => 'red'],
            verbose_name: 'RED Max Threshold',
            help_text: 'The value for the RED max_th parameter.',
        );
        $this->param_red_max_p = new IntegerField(
            default: 1,
            conditions: ['aqm' => 'red'],
            verbose_name: 'RED Max Probability',
            help_text: 'The value for the RED max_p parameter.',
        );
        $this->param_gred_w_q = new IntegerField(
            default: 1,
            conditions: ['aqm' => 'gred'],
            verbose_name: 'GRED Weight',
            help_text: 'The value for the GRED w_q parameter.',
        );
        $this->param_gred_min_th = new IntegerField(
            default: 0,
            conditions: ['aqm' => 'gred'],
            verbose_name: 'GRED Min Threshold',
            help_text: 'The value for the GRED min_th parameter.',
        );
        $this->param_gred_max_th = new IntegerField(
            default: 1,
            conditions: ['aqm' => 'gred'],
            verbose_name: 'GRED Max Threshold',
            help_text: 'The value for the GRED max_th parameter.',
        );
        $this->param_gred_max_p = new IntegerField(
            default: 1,
            conditions: ['aqm' => 'gred'],
            verbose_name: 'GRED Max Probability',
            help_text: 'The value for the GRED max_p parameter.',
        );
        $this->weight = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            maximum: 100,
            verbose_name: 'Weight',
            help_text: 'The share of the parent limiter this queue gets.',
        );
        $this->plr = new FloatField(
            default: null,
            allow_null: true,
            minimum: 0,
            maximum: 1,
            verbose_name: 'Packet Loss Rate',
            help_text: 'The amount of packet loss (in percentage) added to traffic passing through this limiter queue.',
        );
        $this->buckets = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 16,
            maximum: 65535,
            verbose_name: 'Buckets',
            help_text: "The limiter queue's bucket size (slots).",
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Reloads the filter, and resets applicable RRD graphs
     */
    public function apply(): void {
        (new TrafficShaperApplyDispatcher(async: $this->async))->spawn_process();
    }

    /**
     * Extends the default _delete() method to ensure the queue is removed from dnctl afterward.
     */
    public function _delete(): void {
        # Delete the limiter from config and remove the limiter from dnctl
        parent::_delete();
        new Command('dnctl queue delete ' . $this->number->value);
    }

    /**
     * Obtains the next available DN queue `number`.
     * @return int The next available queue `number` value.
     */
    protected function get_next_number(): int {
        read_dummynet_config(); // Populates the dnqueue list used by dnqueue_find_nextnumber()
        return dnqueue_find_nextnumber();
    }
}
