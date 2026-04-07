<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Auth;
use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Validators\RegexValidator;

/**
 * Defines a Model for interacting with the system's cron jobs.
 */
class CronJob extends Model {
    const CRON_PREDEFINED_EVENTS = [
        '@reboot',
        '@yearly',
        '@annually',
        '@monthly',
        '@weekly',
        '@daily',
        '@midnight',
        '@hourly',
        '@every_minute',
        '@every_second',
    ];

    public StringField $minute;
    public StringField $hour;
    public StringField $mday;
    public StringField $month;
    public StringField $wday;
    public StringField $who;
    public StringField $command;

    /**
     * @param mixed|null $id Specify the ID of a CronJob object to obtain. Leave as null to create a new one.
     * @param mixed $data Pass in representation data to populate field values during construction.
     * @param Auth|null $client Pass in a custom Auth object for this Model to reference
     * @param bool $async Apply changes to this Model asynchronously (unused)
     * @param bool $require_pkg Specifies whether the pfSense-pkg-Cron package must be installed to use this Model.
     * All necessary Cron components for this Model are already part of the base pfSense installation. While
     * technically the package is optional, the webConfigurator mandates its installation for managing Cron.
     * Consequently, the API adheres to the same default behavior, requiring the package for Cron management.
     */
    public function __construct(
        mixed $id = null,
        mixed $parent_id = null,
        mixed $data = [],
        bool $require_pkg = true,
        mixed ...$options,
    ) {
        # Set model attributes
        $this->config_path = 'cron/item';
        $this->many = true;
        $this->packages = $require_pkg ? ['pfSense-pkg-Cron'] : [];
        $this->always_apply = true;

        # Set model fields
        $this->minute = new StringField(
            required: true,
            validators: [
                new RegexValidator(
                    pattern: "/^(((\d+,)+\d+|(\d+(\/|-)\d+)|\d+|\*|\*\/\d+|@\d+) ?)$/",
                    allowed_keywords: self::CRON_PREDEFINED_EVENTS,
                    error_msg: 'Value must be a numeric 0-59, a range, a divided value, an @ event or delay, or `*`.',
                ),
            ],
            verbose_name: 'Minute',
            help_text: 'The minute(s) at which the command will be executed or a special @ event string. ' .
                '(0-59, ranges, divided, @ event or delay, *=all). When using a special @ event, such as @reboot, ' .
                'the other time fields must be empty.',
        );
        $this->hour = new StringField(
            required: true,
            conditions: ['!minute' => self::CRON_PREDEFINED_EVENTS],
            validators: [
                new RegexValidator(
                    pattern: "/^(((\d+,)+\d+|(\d+(\/|-)\d+)|\d+|\*|\*\/\d+|@\d+) ?)$/",
                    error_msg: 'Value must be a numeric 0-59, a range, a divided value, or `*`.',
                ),
            ],
            verbose_name: 'Hour',
            help_text: 'The hour(s) at which the command will be executed. (0-23, ranges, or divided, *=all)',
        );
        $this->mday = new StringField(
            required: true,
            conditions: ['!minute' => self::CRON_PREDEFINED_EVENTS],
            validators: [
                new RegexValidator(
                    pattern: "/^(((\d+,)+\d+|(\d+(\/|-)\d+)|\d+|\*|\*\/\d+|@\d+) ?)$/",
                    error_msg: 'Value must be a numeric 0-59, a range, a divided value, or `*`.',
                ),
            ],
            verbose_name: 'Mday',
            help_text: 'The day(s) of the month on which the command will be executed. (1-31, ranges, or divided, *=all).',
        );
        $this->month = new StringField(
            required: true,
            conditions: ['!minute' => self::CRON_PREDEFINED_EVENTS],
            validators: [
                new RegexValidator(
                    pattern: "/^(((\d+,)+\d+|(\d+(\/|-)\d+)|\d+|\*|\*\/\d+|@\d+) ?)$/",
                    error_msg: 'Value must be a numeric 0-59, a range, a divided value, or `*`.',
                ),
            ],
            verbose_name: 'Month',
            help_text: 'The month(s) of the year in which the command will be executed. (1-31, ranges, or divided, *=all).',
        );
        $this->wday = new StringField(
            required: true,
            conditions: ['!minute' => self::CRON_PREDEFINED_EVENTS],
            validators: [
                new RegexValidator(
                    pattern: "/^(((\d+,)+\d+|(\d+(\/|-)\d+)|\d+|\*|\*\/\d+|@\d+) ?)$/",
                    error_msg: 'Value must be a numeric 0-7, a range, a divided value, or `*`.',
                ),
            ],
            verbose_name: 'Wday',
            help_text: 'The day(s) of the week on which the command will be executed. (0-7, 7=Sun or use names, ' .
                'ranges, or divided, *=all).',
        );
        $this->who = new StringField(required: true, verbose_name: 'Who', help_text: 'The OS user to use when cron runs the command.');
        $this->command = new StringField(
            required: true,
            verbose_name: 'Command',
            help_text: 'The command to run. Use full file paths for this command and include an command parameters.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `who` field.
     * @param string $who The incoming value to be validated.
     * @returns string The validated value to be assigned.
     * @throws NotFoundError When $who is not an existing user on the OS.
     */
    public function validate_who(string $who): string {
        # Throw a validation error if the requested user does not exist
        if (posix_getpwnam($who) === false) {
            throw new NotFoundError(
                message: "Field `who` must be an existing OS user. User `$who` does not exist.",
                response_id: 'CRON_JOB_WHO_USER_DOES_NOT_EXIST',
            );
        }

        return $who;
    }

    /**
     * Apply changes to cron.
     */
    public function apply(): void {
        configure_cron();
    }
}
