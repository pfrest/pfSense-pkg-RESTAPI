<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\UnprocessableContentError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\FilterNameValidator;
use function RESTAPI\Core\Tools\bandwidth_to_bits;

/**
 * Defines a Model for interacting with the traffic shaper queues.
 */
class TrafficShaperQueue extends Model {
    public InterfaceField $interface;
    public BooleanField $enabled;
    public StringField $name;
    public IntegerField $priority;
    public IntegerField $qlimit;
    public StringField $description;
    public BooleanField $default;
    public BooleanField $red;
    public BooleanField $rio;
    public BooleanField $ecn;
    public BooleanField $codel;
    public StringField $bandwidthtype;
    public IntegerField $bandwidth;
    public IntegerField $buckets;
    public IntegerField $hogs;
    public BooleanField $borrow;
    public BooleanField $upperlimit;
    public StringField $upperlimit_m1;
    public IntegerField $upperlimit_d;
    public StringField $upperlimit_m2;
    public BooleanField $realtime;
    public StringField $realtime_m1;
    public IntegerField $realtime_d;
    public StringField $realtime_m2;
    public BooleanField $linkshare;
    public StringField $linkshare_m1;
    public IntegerField $linkshare_d;
    public StringField $linkshare_m2;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->parent_model_class = 'TrafficShaper';
        $this->config_path = 'queue';
        $this->many = true;
        $this->subsystem = 'shaper';

        # Set model fields
        $this->interface = new InterfaceField(
            default: null,
            read_only: true,
            editable: false,
            verbose_name: 'Interface',
            help_text: 'The parent interface this traffic shaper queue a child of. This value is automatically ' .
                "determined by the queue's parent and cannot be manually set or changed.",
        );
        $this->enabled = new BooleanField(
            default: false,
            indicates_true: 'on',
            verbose_name: 'Enabled',
            help_text: 'Enables or disables the traffic shaper queue.',
        );
        $this->name = new StringField(
            required: true,
            unique: true,
            maximum_length: 15,
            validators: [new FilterNameValidator()],
            verbose_name: 'Name',
            help_text: 'The name to assign this traffic shaper queue.',
        );
        $this->priority = new IntegerField(
            unique: true,
            default: 1,
            minimum: 0,
            maximum: 15,
            conditions: ['parent__scheduler' => ['FAIRQ', 'CBQ', 'PRIQ']],
            verbose_name: 'Priority',
            help_text: 'The priority level for this traffic shaper queue.',
        );
        $this->qlimit = new IntegerField(
            required: true,
            minimum: 1,
            verbose_name: 'Queue Limit',
            help_text: 'The number of packets that can be held in a queue waiting to be transmitted by the shaper.',
        );
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this traffic shaper queue.',
        );
        $this->default = new BooleanField(
            default: false,
            indicates_true: 'default',
            verbose_name: 'Default',
            help_text: 'Mark this traffic shaper queue as the default queue.',
        );
        $this->red = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'RED',
            help_text: "Use the 'Random Early Detection' scheduler option for this traffic shaper queue.",
        );
        $this->rio = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'RIO',
            help_text: "Use the 'Random Early Detection In and Out' scheduler option for this traffic shaper queue.",
        );
        $this->ecn = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'ECN',
            help_text: "Use the 'Explicit Congestion Notification' scheduler option for this traffic shaper queue.",
        );
        $this->codel = new BooleanField(
            default: false,
            indicates_true: 'yes',
            verbose_name: 'CoDel',
            help_text: "Use the 'Codel Active Queue' scheduler option for this traffic shaper queue.",
        );
        $this->bandwidth = new IntegerField(
            required: true,
            minimum: 1,
            conditions: ['parent__scheduler' => ['FAIRQ', 'CBQ', 'HFSC']],
            verbose_name: 'Bandwidth',
            help_text: 'The total bandwidth amount allowed by this traffic shaper.',
        );
        $this->bandwidthtype = new StringField(
            default: 'Mb',
            choices: ['%', 'b', 'Kb', 'Mb', 'Gb'],
            conditions: ['parent__scheduler' => ['FAIRQ', 'CBQ', 'HFSC']],
            verbose_name: 'Bandwidth Type',
            help_text: "The scale type of the `bandwidth` field's value.",
        );
        $this->buckets = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['parent__scheduler' => 'FAIRQ'],
            verbose_name: 'Buckets',
        );
        $this->hogs = new IntegerField(
            default: null,
            allow_null: true,
            conditions: ['parent__scheduler' => 'FAIRQ'],
            verbose_name: 'Hogs',
            help_text: 'The bandwidth limit per host.',
        );
        $this->borrow = new BooleanField(
            default: false,
            indicates_true: 'yes',
            conditions: ['parent__scheduler' => 'CBQ'],
            verbose_name: 'Borrow',
            help_text: 'Allow this queue to borrow from other queues when available.',
        );
        $this->upperlimit = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['parent__scheduler' => 'HFSC'],
            verbose_name: 'Upper Limit',
            help_text: 'Allow setting the maximum bandwidth allowed for the queue. Will force hard bandwidth limiting.',
        );
        $this->upperlimit_m1 = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'upperlimit1',
            conditions: ['upperlimit' => true],
            verbose_name: 'Upper Limit M1',
            help_text: 'The burst-able bandwidth limit for this traffic shaper queue.',
        );
        $this->upperlimit_d = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            internal_name: 'upperlimit2',
            conditions: ['upperlimit' => true],
            help_text: 'The duration (in milliseconds) that the burst-able bandwidth limit (`upperlimit_m1` is in effect.',
        );
        $this->upperlimit_m2 = new StringField(
            required: true,
            internal_name: 'upperlimit3',
            conditions: ['upperlimit' => true],
            verbose_name: 'Upper Limit M2',
            help_text: 'The normal bandwidth limit for this traffic shaper queue. If `upperlimit_m1` is not defined, ' .
                'this limit will always be in effect. If `upperlimit_m1` is defined, this limit will take effect ' .
                'after the `upperlimit_d` duration has expired.',
        );
        $this->realtime = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['parent__scheduler' => 'HFSC'],
            verbose_name: 'Real Time',
            help_text: 'Allow setting the guaranteed bandwidth minimum allotted to the queue.',
        );
        $this->realtime_m1 = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'realtime1',
            conditions: ['realtime' => true],
            verbose_name: 'Real Time M1',
            help_text: 'The guaranteed minimum bandwidth limit for this traffic shaper queue during real time.',
        );
        $this->realtime_d = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            internal_name: 'realtime2',
            conditions: ['realtime' => true],
            verbose_name: 'Real Time D',
            help_text: 'The duration (in milliseconds) that the guaranteed bandwidth limit (`realtime_m1`) is in effect.',
        );
        $this->realtime_m2 = new StringField(
            required: true,
            internal_name: 'realtime3',
            conditions: ['realtime' => true],
            verbose_name: 'Real Time M2',
            help_text: 'The maximum bandwidth this traffic shaper queue is allowed to use. Note: This value should ' .
                "not exceed 30% of parent queue's maximum bandwidth.",
        );
        $this->linkshare = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['parent__scheduler' => 'HFSC'],
            verbose_name: 'Link Share',
            help_text: 'Allow sharing bandwidth from this queue for other queues as long as the real time values ' .
                'have been satisfied.',
        );
        $this->linkshare_m1 = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'linkshare1',
            conditions: ['linkshare' => true],
            verbose_name: 'Link Share M1',
            help_text: 'The initial bandwidth limit for this traffic shaper queue when link sharing.',
        );
        $this->linkshare_d = new IntegerField(
            default: null,
            allow_null: true,
            minimum: 1,
            internal_name: 'linkshare2',
            conditions: ['linkshare' => true],
            verbose_name: 'Link Share D',
            help_text: 'The duration (in milliseconds) that the initial bandwidth limit (`linkshare_m1`) is in effect.',
        );
        $this->linkshare_m2 = new StringField(
            required: true,
            internal_name: 'linkshare3',
            conditions: ['linkshare' => true],
            verbose_name: 'Link Share M2',
            help_text: 'The maximum bandwidth this traffic shaper queue is allowed to use. Note: This behaves exactly ' .
                'the same as the `bandwidth` field. If this field is set, it will override whatever value is ' .
                'current assigned to the `bandwidth` field.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation for this entire Model.
     * @throws ValidationError When the parent TrafficShaper's `scheduler` is set to CODELQ. This TrafficShapers cannot
     * support child queues.
     */
    public function validate_extra(): void {
        # Do not allow TrafficShaperQueues to be created for CODELQ TrafficShapers
        if ($this->parent_model->scheduler->value === 'CODELQ') {
            throw new UnprocessableContentError(
                message: "Parent traffic shaper with ID `$this->parent_id` does not support child queues because it " .
                    'uses `scheduler` type `CODELQ`',
                response_id: 'TRAFFIC_SHAPER_QUEUE_NOT_SUPPORTED_BY_PARENT_SCHEDULER',
            );
        }

        # Force the `interface` field value to always match the parent
        # TODO: Change this to use the default_callable for this field when developed
        $this->interface->value = $this->parent_model->interface->to_internal();
    }

    /**
     * Adds extra validation to the `bandwidthtype` field.
     * @param string $bandwidthtype The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When the parent TrafficShaper has its type set to `%` but this object specifies an
     * exact amount.
     */
    public function validate_bandwidthtype(string $bandwidthtype): string {
        # Throw an error if the parent's `bandwidthtype` is `%` but this object's is not
        # Note: This is not enforced in the pfSense webConfigurator but probably should be. The algorithm used to check
        # if a queue is over committed on bandwidth becomes inaccurate when the parent uses % but child queues use
        # a set amount.
        if ($bandwidthtype !== '%' and $this->parent_model->bandwidthtype->value === '%') {
            throw new UnprocessableContentError(
                message: "Field `bandwidthtype` must be set `%` when the parent's `bandwidthtype` is set to `%`.",
                response_id: 'TRAFFIC_SHAPER_QUEUE_BANDWIDTHTYPE_MUST_MATCH_PARENT_WITH_PERCENT',
            );
        }

        return $bandwidthtype;
    }

    /**
     * Adds extra validation to the `bandwidth` field.
     * @param int $bandwidth The incoming value to be validated.
     * @returns int The validated value to be assigned.
     * @throws ValidationError When the sum of all queue bandwidth assigned to this queue's parent exceeds the
     * maximum bandwidth allowed by the parent.
     */
    public function validated_bandwidth(int $bandwidth): int {
        # Do not allow the bandwidth allowed by this queue to push the sum of all bandwidth for the interface over
        if ($this->is_bandwidth_exceeded()) {
            $our_bw = $bandwidth . $this->bandwidthtype->value;
            $parent_bw = $this->parent_model->bandwidth->value . $this->parent_model->bandwidthtype->value;
            throw new UnprocessableContentError(
                message: "Cannot allot $our_bw to this queue because sum of all queues for this parent exceeds " .
                    "the parent's maximum bandwidth limit of $parent_bw.",
                response_id: 'TRAFFIC_SHAPER_EXCEEDS_PARENTS_BANDWIDTH_LIMITS',
            );
        }

        return $bandwidth;
    }

    /**
     * Adds extra validation to the `priority` field.
     * @param integer $priority The incoming value to be validated.
     * @returns integer The validated value to be assigned.
     * @throws ValidationError When the parent `scheduler` is not `PRIQ` but the $priority is greater than 7.
     */
    public function validate_priority(int $priority): int {
        # Variables
        $parent_scheduler = $this->parent_model->scheduler->value;

        # Throw an error if the $priority is greater than 7, but the parent `scheduler` is not `PRIQ`
        # Note: the `priority` field's `maximum` property will still force a value for 15 or less.
        if ($parent_scheduler !== 'PRIQ' and $priority > 7) {
            throw new ValidationError(
                message: "Field `priority` must be 7 or less while the parent's `scheduler` is `$parent_scheduler`.",
                response_id: 'TRAFFIC_SHAPER_QUEUE_PRIORITY_MAXIMUM_EXCEEDED_FOR_PARENT_SCHEDULER',
            );
        }

        return $priority;
    }

    /**
     * Adds extra validation to the `upperlimit_m1` field.
     * @param string $upperlimit_m1 The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When $upperlimit_m1 is not a bandwidth formatted string (i.e. 25Mb)
     * @throws ValidationError When $upperlimit_m1 is defined, but no `upperlimit_d` field value is defined.
     */
    public function validate_upperlimit_m1(string $upperlimit_m1): string {
        # Throw an error if this value is not a bandwidth formatted string
        if (!$this->is_bandwidth_string($upperlimit_m1)) {
            throw new ValidationError(
                message: 'Field `upperlimit_m1` must be in a valid bandwidth format.',
                response_id: 'TRAFFIC_SHAPER_QUEUE_UPPERLIMIT_M1_INVALID_BANDWIDTH_FORMAT',
            );
        }

        # Throw an error if `upperlimit_d` was not defined. These two fields are mutually inclusive.
        if (!$this->upperlimit_d->value) {
            throw new ValidationError(
                message: 'Field `upperlimit_m1` cannot be defined without `upperlimit_d`.',
                response_id: 'TRAFFIC_SHAPER_QUEUE_UPPERLIMIT_M1_REQUIRES_UPPERLIMIT_D',
            );
        }

        return $upperlimit_m1;
    }

    /**
     * Adds extra validation to the `realtime_m1` field.
     * @param string $realtime_m1 The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When $realtime_m1 is not a bandwidth formatted string (i.e. 25Mb)
     * @throws ValidationError When $realtime_m1 is defined, but no `realtime_d` field value is defined.
     */
    public function validate_realtime_m1(string $realtime_m1): string {
        # Throw an error if this value is not a bandwidth formatted string
        if (!$this->is_bandwidth_string($realtime_m1)) {
            throw new ValidationError(
                message: 'Field `realtime_m1` must be in a valid bandwidth format.',
                response_id: 'TRAFFIC_SHAPER_QUEUE_REALTIME_M1_INVALID_BANDWIDTH_FORMAT',
            );
        }

        # Throw an error if `realtime_d` was not defined. These two fields are mutually inclusive.
        if (!$this->realtime_d->value) {
            throw new ValidationError(
                message: 'Field `realtime_m1` cannot be defined without `realtime_d`.',
                response_id: 'TRAFFIC_SHAPER_QUEUE_REALTIME_M1_REQUIRES_REALTIME_D',
            );
        }

        return $realtime_m1;
    }

    /**
     * Adds extra validation to the `linkshare_m1` field.
     * @param string $linkshare_m1 The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When $linkshare_m1 is not a bandwidth formatted string (i.e. 25Mb)
     * @throws ValidationError When $linkshare_m1 is defined, but no `linkshare_d` field value is defined.
     */
    public function validate_linkshare_m1(string $linkshare_m1): string {
        # Throw an error if this value is not a bandwidth formatted string
        if (!$this->is_bandwidth_string($linkshare_m1)) {
            throw new ValidationError(
                message: 'Field `linkshare_m1` must be in a valid bandwidth format.',
                response_id: 'TRAFFIC_SHAPER_QUEUE_LINKSHARE_M1_INVALID_BANDWIDTH_FORMAT',
            );
        }

        # Throw an error if `linkshare_d` was not defined. These two fields are mutually inclusive.
        if (!$this->linkshare_d->value) {
            throw new ValidationError(
                message: 'Field `linkshare_m1` cannot be defined without `linkshare_d`.',
                response_id: 'TRAFFIC_SHAPER_QUEUE_LINKSHARE_M1_REQUIRES_LINKSHARE_D',
            );
        }

        return $linkshare_m1;
    }

    /**
     * Reloads the firewall filter and restarts queue graphing.
     */
    public function apply(): void {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
        system('rm -f /var/db/rrd/*queuedrops.rrd');
        system('rm -f /var/db/rrd/*queues.rrd');
        enable_rrd_graphing();
    }

    /**
     * Checks if a given value is a bandwidth formatted string (i.e. 20Mb, 1Gb)
     * @param string $value The string to validate as a bandwidth string
     * @returns bool Returns true if $bandwidth is a bandwidth formatted string, otherwise false.
     */
    public static function is_bandwidth_string(string $value): bool {
        # Loop through each bandwidth type
        foreach (['Kb', 'Mb', 'Gb', 'b'] as $type) {
            if (str_ends_with($value, $type)) {
                # Remove the bandwidth type portion from the string
                $num_value = str_replace($type, '', $value);

                # If our expected numeric value is numeric, return a list of parsed data
                if (is_numeric($num_value)) {
                    return true;
                }
            }
        }
        return false;
    }

    /**
     * Obtains the total bandwidth for a given TrafficShaperQueue in bits. If the queue is percentage based, and the
     * parent TrafficShaper is percentage based, this will return the 1-100 percentage value.
     * @param TrafficShaperQueue $queue The TrafficShaperQueue object to obtain the bandwidth total for.
     * @return int The total number of bandwidth in bits allowed by this queue, or the 1-100 percentage value if both
     * $queue and its parent TrafficShaper are percentage based.
     */
    public static function get_bandwidth_bits(TrafficShaperQueue $queue): int {
        # Variables
        $queue->get_parent_model();
        $parent_is_percent = $queue->parent_model->bandwidthtype->value === '%';
        $parent_bandwidth_max = $queue->parent_model->get_bandwidth_bits();

        # When both parent and child are percentage-based, just return the bandwidth as it's already a percentage
        if ($parent_is_percent and $queue->bandwidthtype->value === '%') {
            return $queue->bandwidth->value;
        }

        # When the parent is not percentage-based but this queue is, convert the percentage to real bits.
        if (!$parent_is_percent and $queue->bandwidthtype->value === '%') {
            return intval(($queue->bandwidth->value / 100) * $parent_bandwidth_max);
        }

        # Otherwise, just convert the bandwidth scale to bits.
        return bandwidth_to_bits($queue->bandwidth->value, $queue->bandwidthtype->value);
    }

    /**
     * Checks if the sum of all child queue bandwidth exceeds the parent's bandwidth limit.
     * @return bool Returns true if the total bandwidth exceeds the parent's bandwidth limit. Otherwise, false.
     */
    public function is_bandwidth_exceeded(): bool {
        # Variables
        $parent_bandwidth_max = $this->parent_model->get_bandwidth_bits();
        $queues = TrafficShaperQueue::query(parent_id: $this->parent_id);
        $bandwidth_total = 0;

        # Loop through each child queue and ensure the sum of each does not exceed the parent's maximum.
        foreach ($queues->model_objects as $queue) {
            # Skip this object in case its being updated, we'll add this objects values after the loop
            if ($queue->id !== $this->id) {
                $bandwidth_total += $this->get_bandwidth_bits($queue);
            }
        }

        # Add our current bandwidth too
        $bandwidth_total += $this->get_bandwidth_bits($this);

        return $bandwidth_total > $parent_bandwidth_max;
    }
}
