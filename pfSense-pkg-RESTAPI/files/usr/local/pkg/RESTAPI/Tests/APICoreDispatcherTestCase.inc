<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Dispatcher;
use RESTAPI\Core\TestCase;
use RESTAPI\Dispatchers\WireGuardApplyDispatcher;
use RESTAPI\Models\Package;
use RESTAPI\Models\RESTAPISettings;

class APICoreDispatcherTestCase extends TestCase {
    /**
     * Checks that the `max_concurrency` property limits the number of active Dispatcher process that can
     * be spawn.
     */
    public function test_max_concurrency() {
        # Create a test dispatcher and spawn the maximum number of processes
        $dispatcher = new Dispatcher();
        $dispatcher->max_queue = 2;
        $dispatcher->spawn_process();
        $dispatcher->spawn_process();

        # Ensure an error is thrown trying to spawn another
        $this->assert_throws_response(
            response_id: 'DISPATCHER_TOO_MANY_QUEUED_PROCESSES',
            code: 503,
            callable: function () use ($dispatcher) {
                $dispatcher->spawn_process();
            },
        );

        # Kill the running processes
        $dispatcher->kill_running_processes();
    }

    /**
     * Checks that running processes are correctly identified by `get_running_processes()`
     */
    public function test_get_running_processes() {
        # Create a test dispatcher
        $dispatcher = new Dispatcher();

        # Spawn a few processes
        $pid1 = $dispatcher->spawn_process();
        $pid2 = $dispatcher->spawn_process();

        # Ensure both spawned PIDs are in the returned array
        $this->assert_equals($dispatcher->get_running_processes(), [$pid1, $pid2]);

        # Kill the running processes
        $dispatcher->kill_running_processes();
    }

    /**
     * Checks that the PID file is created after process spawns.
     */
    public function test_pid_file_exists_after_spawn() {
        # Create a test dispatcher
        $dispatcher = new Dispatcher();
        $dispatcher->timeout = 2;
        $pid_file = $dispatcher->get_pid_file();

        # Spawn the process and ensure the PID file exists and it's contents match the PID
        $pid = $dispatcher->spawn_process();
        $this->assert_is_true(is_file($pid_file));
        $this->assert_equals((int) file_get_contents($pid_file), $pid);

        # Kill the processes
        $dispatcher->kill_running_processes();
    }

    /**
     * Checks that the PID and PID file are killed when `kill_running_processes()` is called
     */
    public function test_kill_running_processes() {
        # Create a test dispatcher
        $dispatcher = new Dispatcher();
        $dispatcher->timeout = 2;
        $pid_file = $dispatcher->get_pid_file();

        # Spawn the process and ensure the PID file exists and it's contents match the PID
        $pid = $dispatcher->spawn_process();

        # Kill the processes and ensure the PID and PID file are no longer present
        $dispatcher->kill_running_processes();
        $this->assert_is_false(is_file($pid_file));
        $this->assert_is_empty($dispatcher->get_running_processes());
    }

    /**
     * Checks that Dispatcher processes is correctly timed out after the specified amount of time.
     */
    public function test_process_timeout() {
        # Create a test dispatcher
        $dispatcher = new Dispatcher();
        $dispatcher->timeout = 2;
        $pid_file = $dispatcher->get_pid_file();

        # Spawn a new process and ensure it is running
        $dispatcher->spawn_process();
        $this->assert_is_not_empty($dispatcher->get_running_processes());

        # Wait for the Dispatcher to timeout and check that the process is no longer running
        sleep($dispatcher->timeout + 1);
        $this->assert_is_empty($dispatcher->get_running_processes());

        # Ensure the PID file for the process is no longer present
        $this->assert_is_false(is_file($pid_file));
    }

    /**
     * Checks that a Dispatcher cannot be assigned an unsupported `schedule` value
     */
    public function test_schedule_options(): void {
        $this->assert_throws_response(
            response_id: 'DISPATCHER_SCHEDULE_UNSUPPORTED',
            code: 500,
            callable: function () {
                $dispatcher = new Dispatcher();
                $dispatcher->schedule = 'invalid_option';
                $dispatcher->setup_schedule();
            },
        );
    }

    /**
     * Checks that the Dispatcher's `setup_schedule()` method creates a CronJob for the Dispatcher using the `schedule`
     * property.
     */
    public function test_setup_schedule(): void {
        # Create a Dispatcher to test with but do not assign it a schedule yet
        $dispatcher = new Dispatcher();

        # Ensure the setup_schedule() returns no when no schedule is set
        $this->assert_equals($dispatcher->setup_schedule(), null);

        # Assign the dispatcher a schedule and ensure setup_schedule() returns a CronJob object with the correct schedule
        $dispatcher->schedule = '1 2 3 4 5';
        $dispatcher_cron_job = $dispatcher->setup_schedule();
        $cron_job_cmd = '/usr/local/pkg/RESTAPI/.resources/scripts/manage.php notifydispatcher Dispatcher';
        $this->assert_equals($dispatcher_cron_job->minute->value, '1');
        $this->assert_equals($dispatcher_cron_job->hour->value, '2');
        $this->assert_equals($dispatcher_cron_job->mday->value, '3');
        $this->assert_equals($dispatcher_cron_job->month->value, '4');
        $this->assert_equals($dispatcher_cron_job->wday->value, '5');
        $this->assert_equals($dispatcher_cron_job->command->value, $cron_job_cmd);

        # Delete the CronJob
        $dispatcher_cron_job->delete();
    }

    /**
     * Checks that Dispatchers with $required_packages must have the listed packages installed before use.
     */
    public function check_required_packages(): void {
        # Ensure an error is thrown if a Dispatcher is constructed without all required packages installed
        $this->assert_throws_response(
            response_id: 'DISPATCHER_MISSING_REQUIRED_PACKAGE',
            code: 424,
            callable: function () {
                # Use the WireGuardApplyDispatcher to test this as it requires pfSense-pkg-WireGuard.
                new WireGuardApplyDispatcher();
            },
        );

        # Install the required package and ensure the error is no longer thrown when constructed
        $package = new Package(name: 'pfSense-pkg-WireGuard');
        $package->create();
        $this->assert_does_not_throw(
            callable: function () {
                # Use the WireGuardApplyDispatcher to test this as it requires pfSense-pkg-WireGuard.
                $dispatcher = new WireGuardApplyDispatcher();
                $dispatcher->spawn_process();
            },
        );

        # Remove the installed package
        $package->delete();
    }

    /**
     * Checks that a Dispatcher requiring a package that is not installed throws DISPATCHER_MISSING_REQUIRED_PACKAGE
     * when the `allow_development_packages` setting is disabled (default).
     */
    public function test_required_package_missing_throws_by_default(): void {
        # Ensure the allow_development_packages setting is disabled
        $settings = new RESTAPISettings();
        $settings->from_internal();
        $original = $settings->allow_development_packages->value;

        if ($original) {
            $settings->allow_development_packages->value = false;
            $settings->update(apply: false);
        }

        # A Dispatcher requiring a package that is not installed should fail
        $this->assert_throws_response(
            response_id: 'DISPATCHER_MISSING_REQUIRED_PACKAGE',
            code: 424,
            callable: function () {
                $dispatcher = new RequiredPackagesTestDispatcher();
                $dispatcher->required_packages = ['pfSense-pkg-some_package_we_dont_have'];
                $dispatcher->process();
            },
        );
    }

    /**
     * Checks that when `allow_development_packages` is enabled, a Dispatcher whose required package is not installed
     * and whose -devel counterpart is also not installed still throws DISPATCHER_MISSING_REQUIRED_PACKAGE. Since we
     * cannot install arbitrary packages in tests, we verify the inverse: a package whose -devel name also doesn't
     * exist still fails even with the setting enabled.
     */
    public function test_devel_variant_still_fails_when_neither_installed(): void {
        # Enable the allow_development_packages setting
        $settings = new RESTAPISettings();
        $settings->from_internal();
        $settings->allow_development_packages->value = true;
        $settings->update(apply: false);

        # Even with the setting on, if neither the base package nor the -devel variant is installed, it should fail
        $this->assert_throws_response(
            response_id: 'DISPATCHER_MISSING_REQUIRED_PACKAGE',
            code: 424,
            callable: function () {
                $dispatcher = new RequiredPackagesTestDispatcher();
                $dispatcher->required_packages = ['pfSense-pkg-some_package_we_dont_have'];
                $dispatcher->process();
            },
        );

        # Restore the setting to its original value
        $settings->allow_development_packages->value = false;
        $settings->update(apply: false);
    }

    /**
     * Checks that when `allow_development_packages` is enabled, a Dispatcher whose required base package is installed
     * (using pfSense-pkg-RESTAPI as a proxy for the base package) still passes the required package check.
     */
    public function test_base_package_accepted_when_devel_enabled(): void {
        # Enable the allow_development_packages setting
        $settings = new RESTAPISettings();
        $settings->from_internal();
        $settings->allow_development_packages->value = true;
        $settings->update(apply: false);

        # The base package (pfSense-pkg-RESTAPI) is installed; the package check should still pass
        $this->assert_does_not_throw(
            callable: function () {
                $dispatcher = new RequiredPackagesTestDispatcher();
                $dispatcher->required_packages = ['pfSense-pkg-RESTAPI'];
                $dispatcher->process();
            },
        );

        # Restore the setting to its original value
        $settings->allow_development_packages->value = false;
        $settings->update(apply: false);
    }
}

/**
 * Defines a test-only Dispatcher used to exercise the $required_packages check without spawning a real background
 * process. The $required_packages property is exposed publicly so individual tests can assign the package(s) to
 * check, and _process() is intentionally a no-op so calling process() only runs the required package validation.
 */
class RequiredPackagesTestDispatcher extends Dispatcher {
    public array $required_packages = [];

    /**
     * Overrides the default _process() with a no-op so tests can validate the package check in isolation.
     * @param mixed ...$arguments Unused arguments accepted to match the parent signature.
     */
    protected function _process(mixed ...$arguments): void {}
}
