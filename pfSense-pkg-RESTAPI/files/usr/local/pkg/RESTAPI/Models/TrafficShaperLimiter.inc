<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\TrafficShaperApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FloatField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\FilterNameValidator;

/**
 * Defines a Model for interacting with the traffic shaper limiters.
 */
class TrafficShaperLimiter extends Model {
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
    public StringField $sched;
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
    public IntegerField $param_fq_codel_target;
    public IntegerField $param_fq_codel_interval;
    public IntegerField $param_fq_codel_quantum;
    public IntegerField $param_fq_codel_limit;
    public IntegerField $param_fq_codel_flows;
    public IntegerField $param_fq_pie_target;
    public IntegerField $param_fq_pie_tupdate;
    public IntegerField $param_fq_pie_alpha;
    public IntegerField $param_fq_pie_beta;
    public IntegerField $param_fq_pie_max_burst;
    public IntegerField $param_fq_pie_max_ecnth;
    public IntegerField $param_fq_pie_quantum;
    public IntegerField $param_fq_pie_limit;
    public IntegerField $param_fq_pie_flows;
    public IntegerField $delay;
    public FloatField $plr;
    public IntegerField $buckets;
    public NestedModelField $bandwidth;
    public NestedModelField $queue;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'dnshaper/queue';
        $this->subsystem = 'shaper';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [new FilterNameValidator()],
            verbose_name: 'Name',
            help_text: 'The unique name for this limiter.',
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
            help_text: 'Enables or disables this limiter and its child queues.',
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
            help_text: 'The verbose description for this limiter.',
        );
        $this->aqm = new StringField(
            required: true,
            choices: ['droptail', 'codel', 'pie', 'red', 'gred'],
            verbose_name: 'AQM',
            help_text: 'The Active Queue Management (AQM) algorithm to use for this limiter. AQM is the intelligent ' .
                'drop of network packets inside the limiter, when it becomes full or gets close to becoming full, ' .
                'with the goal of reducing network congestion.',
        );
        $this->sched = new StringField(
            required: true,
            choices: ['wf2q+', 'fifo', 'qfq', 'rr', 'prio', 'fq_codel', 'fq_pie'],
            verbose_name: 'Schedule',
            help_text: 'The scheduler to use for this limiter. The scheduler manages the sequence of network packets ' .
                "in the limiter's queue.",
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
        $this->param_fq_codel_target = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_codel'],
            verbose_name: 'FQ-CoDel Target',
            help_text: 'The value for the FQ CoDel target parameter.',
        );
        $this->param_fq_codel_interval = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_codel'],
            verbose_name: 'FQ-CoDel Interval',
            help_text: 'The value for the FQ CoDel interval parameter.',
        );
        $this->param_fq_codel_quantum = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['sched' => 'fq_codel'],
            verbose_name: 'FQ-CoDel Quantum',
            help_text: 'The value for the FQ CoDel quantum parameter.',
        );
        $this->param_fq_codel_limit = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['sched' => 'fq_codel'],
            verbose_name: 'FQ-CoDel Limit',
            help_text: 'The value for the FQ CoDel limit parameter.',
        );
        $this->param_fq_codel_flows = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['sched' => 'fq_codel'],
            verbose_name: 'FQ-CoDel Flows',
            help_text: 'The value for the FQ CoDel flows parameter.',
        );
        $this->param_fq_pie_target = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Target',
            help_text: 'The value for the FQ PIE target parameter.',
        );
        $this->param_fq_pie_tupdate = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Tupdate',
            help_text: 'The value for the FQ PIE tupdate parameter.',
        );
        $this->param_fq_pie_alpha = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Alpha',
            help_text: 'The value for the FQ PIE alpha parameter.',
        );
        $this->param_fq_pie_beta = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Beta',
            help_text: 'The value for the FQ PIE beta parameter.',
        );
        $this->param_fq_pie_max_burst = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Max Burst',
            help_text: 'The value for the FQ PIE max_burst parameter.',
        );
        $this->param_fq_pie_max_ecnth = new IntegerField(
            default: 0,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Max ECN Threshold',
            help_text: 'The value for the FQ PIE ecnth parameter.',
        );
        $this->param_fq_pie_quantum = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Quantum',
            help_text: 'The value for the FQ PIE quantum parameter.',
        );
        $this->param_fq_pie_limit = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Limit',
            help_text: 'The value for the FQ PIE limit parameter.',
        );
        $this->param_fq_pie_flows = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['sched' => 'fq_pie'],
            verbose_name: 'FQ-PIE Flows',
            help_text: 'The value for the FQ PIE flows parameter.',
        );
        $this->delay = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 0,
            maximum: 10000,
            verbose_name: 'Delay',
            help_text: 'The amount of delay (in milliseconds) added to traffic passing through this limiter.',
        );
        $this->plr = new FloatField(
            default: null,
            allow_null: true,
            minimum: 0,
            maximum: 1,
            verbose_name: 'Packet Loss Rate',
            help_text: 'The amount of packet loss (in percentage) added to traffic passing through the limiter.',
        );
        $this->buckets = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 16,
            maximum: 65535,
            verbose_name: 'Buckets',
            help_text: "The limiter's bucket size (slots).",
        );
        $this->bandwidth = new NestedModelField(
            model_class: 'TrafficShaperLimiterBandwidth',
            default: [],
            verbose_name: 'Bandwidth',
            help_text: 'The bandwidth profiles for this limiter.',
        );
        $this->queue = new NestedModelField(
            model_class: 'TrafficShaperLimiterQueue',
            default: [],
            allow_empty: true,
            verbose_name: 'Queue',
            help_text: 'The child queues for this limiter.',
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
     * Extends the default _delete() method to ensure the limit is removed from dnctl afterward.
     */
    public function _delete(): void {
        # Delete the limiter from config and remove the limiter from dnctl
        parent::_delete();
        new Command('dnctl pipe delete ' . $this->number->value);
    }

    /**
     * Obtains the next available DN pipe `number`.
     * @return int The next available pipe `number` value.
     */
    protected function get_next_number(): int {
        read_dummynet_config(); // Populates the dnpipe list used by dnqueue_find_nextnumber()
        return dnpipe_find_nextnumber();
    }
}
