<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a model that interacts with firewall schedule time ranges.
 */
class FirewallScheduleTimeRange extends Model {
    const SUPPORTED_MINUTES = ['00', '15', '30', '45', '59'];

    public IntegerField $position;
    public IntegerField $month;
    public IntegerField $day;
    public StringField $hour;
    public StringField $rangedescr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->parent_model_class = 'FirewallSchedule';
        $this->config_path = 'timerange';
        $this->many = true;
        $this->always_apply = true;
        $this->many_minimum = 1;

        # Set model fieldsf
        $this->position = new IntegerField(
            choices: [1, 2, 3, 4, 5, 6, 7],
            allow_null: true,
            many: true,
            many_maximum: 7,
            verbose_name: 'Position',
            help_text: 'The day of the week this schedule should be active for. Use `1` for every Monday, `2` for ' .
                'every Tuesday, `3` for every Wednesday, `4` for every Thursday, `5` for every Friday, `6` for every ' .
                'Saturday, or `7` for every Sunday. If this field has a value specified, the `month` and `day` fields ' .
                'will be unavailable.',
        );
        $this->month = new IntegerField(
            required: true,
            choices: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12],
            many: true,
            many_minimum: 1,
            conditions: ['position' => null],
            verbose_name: 'Month',
            help_text: 'The month for each specified `day` value. Each value specified must correspond with a `day` ' .
                'field value and must match the order exactly. For example, a `month` value of `[3, 6]` and a `day` ' .
                'value of `[2, 17]` would evaluate to March 2nd and June 17th respectively.',
        );
        $this->day = new IntegerField(
            required: true,
            many: true,
            many_minimum: 1,
            minimum: 1,
            maximum: 31,
            conditions: ['position' => null],
            verbose_name: 'Day',
            help_text: 'The day for each specified `month` value. Each value specified must correspond with a `month` ' .
                'field value and must match the order exactly. For example, a `month` value of `[3, 6]` and a `day` ' .
                'value of `[2, 17]` would evaluate to March 2nd and June 17th respectively.',
        );
        $this->hour = new StringField(
            required: true,
            maximum_length: 11,
            verbose_name: 'Hour',
            help_text: 'The start time and end time for this time range in 24-hour format (i.e. HH:MM-HH:MM).',
        );
        $this->rangedescr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Range Description',
            help_text: "A description detailing this firewall schedule time range's purpose.",
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation for the entire Model.
     * @throws ValidationError When $this->day's value is not a valid day in the $this->month field's current value.
     */
    public function validate_extra(): void {
        # Add validation between the `month` and `day` field if both fields have values
        if ($this->month->value and $this->day->value) {
            # Throw and error if the `day` and `month` field arrays do not have the same number of items
            if (count($this->month->value) !== count($this->day->value)) {
                throw new ValidationError(
                    message: 'Field `day` and field `month` must have the same number of array items.',
                    response_id: 'FIREWALL_SCHEDULE_TIME_RANGE_MISMATCHED_MONTH_AND_DAY_VALUES',
                );
            }

            # Loop through each `month` value and ensure its corresponding `day` value is a day in that month
            foreach ($this->month->value as $id => $month) {
                # Throw an error if $day is larger than the amount of days in the month
                if (!$this->is_day_in_month(month: $month, day: $this->day->value[$id])) {
                    throw new ValidationError(
                        message: "Field `day` value is not a valid day in the `month` of `$month`.",
                        response_id: 'FIREWALL_SCHEDULE_TIME_RANGE_DAY_VALUE_IS_NOT_IN_MONTH',
                    );
                }
            }
        }
    }

    /**
     * Adds extra validation to the `hour` field.
     * @param string $hour The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws ValidationError When $hour is not a valid 24-hour time range (i.e. 0:00-23:59)
     */
    public function validate_hour(string $hour): string {
        # Separate the $hour values start and end times so they can be validated individually
        $start_time = explode('-', $hour)[0] ?: '';
        $start_time_mins = substr($start_time, offset: -2);
        $end_time = explode('-', $hour)[1] ?: '';
        $end_time_mins = substr($end_time, offset: -2);

        # Throw an error if $hour is not a 24-hour time range (i.e. 0:00-23:59)
        if (!$this->is_valid_24_hour_time($start_time) or !$this->is_valid_24_hour_time($end_time)) {
            throw new ValidationError(
                message: 'Field `hour` must be 24-hour time range (i.e. 0:00-23:59).',
                response_id: 'FIREWALL_SCHEDULE_TIME_RANGE_HOUR_MUST_BE_24_HOUR_TIME_RANGE',
            );
        }

        # Throw an error if either the start time or end time are using an unsupported minute value
        if (
            !in_array($start_time_mins, self::SUPPORTED_MINUTES) or !in_array($end_time_mins, self::SUPPORTED_MINUTES)
        ) {
            $min_options_str = implode(',', self::SUPPORTED_MINUTES);
            throw new ValidationError(
                message: "Field `hour` can only specify one of minute value choices: [$min_options_str].",
                response_id: 'FIREWALL_SCHEDULE_TIME_RANGE_HOUR_WITH_INVALID_MINUTES',
            );
        }

        # Throw an error if the start time is greater than the end time
        if (!$this->is_valid_24_hour_time_range($start_time, $end_time)) {
            throw new ValidationError(
                message: 'Field `hour` value start time portion greater than end time.',
                response_id: 'FIREWALL_SCHEDULE_TIME_RANGE_HOUR_START_TIME_GREATER_THAN_END_TIME',
            );
        }

        return $hour;
    }

    /**
     * Checks if a numeric day value is a valid day within a given month.
     * @param int $month The month of the year (i.e. `3` for March)
     * @param int $day The day of the month to evaluate.
     * @return bool Returns true if $day is a valid day of the $month, otherwise false.
     */
    public static function is_day_in_month(int $month, int $day): bool {
        # Local variables
        $month_days = [
            1 => 31,
            2 => 29,
            3 => 31,
            4 => 30,
            5 => 31,
            6 => 30,
            7 => 31,
            8 => 31,
            9 => 30,
            10 => 31,
            11 => 30,
            12 => 31,
        ];

        # Check that the day is within the number of days in its month and return its value
        return 1 <= $day and $day <= $month_days[$month];
    }

    /**
     * Checks if a provided string is a valid 24-hour time string (i.e. HH:MM)
     * @param $time string incoming time string to validate.
     * @return bool Returns true if $time is a valid
     */
    public static function is_valid_24_hour_time(string $time): bool {
        # Convert H:MM to HH:MM for the regex
        $time = substr($time, 1, 1) === ':' ? '0' . $time : $time;

        # Check if this string is formatted as a 24-hour time
        if (preg_match("/^(?:2[0-3]|[01][0-9]):[0-5][0-9]$/", $time)) {
            return true;
        }

        return false;
    }

    /**
     * Checks if a given $start_time is before an $end_time
     * @param $start_time string The start time string to evaluate.
     * @param $end_time string The end time string to evaluate.
     * @return bool Returns true if the $start_time is before $end_time
     */
    public static function is_valid_24_hour_time_range(string $start_time, string $end_time): bool {
        # Check if the start time is less than the end time
        if (strtotime($start_time) <= strtotime($end_time)) {
            return true;
        }

        return false;
    }
}
