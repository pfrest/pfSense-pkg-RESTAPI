<?php

namespace RESTAPI\Core;

use RESTAPI\Models\CronJob;
use RESTAPI\Models\RESTAPISettings;
use RESTAPI\Responses\FailedDependencyError;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ServiceUnavailableError;

/**
 * Defines a Dispatcher object which defines functions that are intended to run in the background. The Dispatcher
 * objects allows us to dynamically create a private PHP script that can be called in the background, manages the
 * process spawn queue, and enforces background process timeouts.
 */
class Dispatcher {
    use BaseTraits;

    /**
     * @const DISPATCH_SCRIPT The absolute file path to the dispatch.sh helper script.
     */
    const DISPATCH_SCRIPT = '/usr/local/pkg/RESTAPI/.resources/scripts/dispatch.sh';

    /**
     * @var string $full_name
     * The full name of this Dispatcher. This value is automatically populated using the class
     * FQN, but all slashes are removed. This value is used when reading and writing the Dispatcher's PID file.
     */
    protected string $full_name;
    /**
     * @var string $short_name
     * The Dispatcher class's short name. This value is passed to the dispatch.sh helper
     * script to indicate which Dispatcher class should be run.
     */
    protected string $short_name;
    /**
     * @var string $pid_dir
     * Sets the directory where the Dispatcher PID file will be written. This file name must be
     *  unique for each Dispatcher process spawned.
     */
    protected string $pid_dir;
    /**
     * @var string $pid_file
     * Sets the entire name of the PID file.
     */
    protected string $pid_file;
    /**
     * @var string $pid_file_prefix The PID file name prefix. This value should contain this Dispatcher's $full_name.
     */
    protected string $pid_file_prefix;
    /**
     * @var int $timeout
     * Sets the maximum number of seconds that processes spawned by this Dispatcher can run for.
     * After this time-frame, the process will be terminated. This ensures Dispatchers cannot spawn processes that run
     * endlessly and crash the system.
     */
    public int $timeout = 300;
    /**
     * @var int $max_queue
     * Sets the maximum number of processes this Dispatcher can have queued at one time.
     * Attempts to spawn processes past this limit will result in a ServiceUnavailableError error.
     */
    public int $max_queue = 10;
    /**
     * @var bool $async
     * Indicates whether this Dispatcher object should spawn processes asynchronously (in the
     * background) or synchronously (waits for process to complete). If set to `true`, any `spawn_process()` method calls
     * will spawn the process in the background and immediately respond. If set to `false`, any `spawn_process()` will
     * wait until the process finishes to respond.
     */
    public bool $async = true;
    /**
     * @var string $schedule
     * Specifies the frequency in which this Dispatcher should be run on a schedule. Valid options are `hourly`,
     * `daily`, and `weekly`. Leave blank if this Dispatcher should not automatically run on a schedule.
     */
    public string $schedule = '';
    /**
     * @var string $schedule_command
     * The shell command that will be used to call this Dispatcher on a schedule.
     */
    protected string $schedule_command = '';
    /**
     * @var array $required_packages
     * The pfSense packages required to run this Dispatcher. This must use the full pfSense package name including the
     * pfSense-pkg- prefix.
     */
    protected array $required_packages = [];
    /**
     * @var array $package_includes
     * Assigns PHP files to include that are specific to one or more of the assigned $packages.
     */
    protected array $package_includes = [];

    /**
     * Constructs the Dispatcher object.
     * @param bool $async Set to `false` to prevent this Dispatcher from running the process in the background.
     */
    public function __construct(bool $async = true) {
        # Get the current full and short class names
        $this->full_name = str_replace('\\', '', get_called_class());
        $this->short_name = $this->get_class_shortname();

        # Set the Dispatcher PID file directory and name. This cannot be changed by child classes.
        $this->pid_dir = '/tmp/';
        $this->pid_file_prefix = $this->full_name . '-';
        $this->pid_file = $this->pid_dir . uniqid(prefix: $this->pid_file_prefix) . '.pid';

        # Set the Dispatcher async property
        $this->async = $async;

        # Set the schedule command for this Dispatcher
        $this->schedule_command = "/usr/local/pkg/RESTAPI/.resources/scripts/manage.php notifydispatcher $this->short_name";
    }

    /**
     * Checks if the required pfSense packages for this Dispatcher are installed and their required dependencies are
     * imported.
     * @throws FailedDependencyError When the Dispatcher requires a pfSense package that is not installed.
     * @throws FailedDependencyError When a package requires a PHP include file that could not be found.
     */
    private function check_required_packages(): void {
        # Check if the user has opted in to using development (-devel) package variants
        $pkg_config = RESTAPISettings::get_pkg_config();
        $allow_development_packages = ($pkg_config['allow_development_packages'] ?? '') === 'enabled';

        # Loop through each required package and ensure it is present on the system.
        foreach ($this->required_packages as $pkg) {
            # When the development packages setting is enabled, also accept the -devel variant of the package
            $devel_pkg = $pkg . '-devel';
            $pkg_installed = is_pkg_installed($pkg) || ($allow_development_packages && is_pkg_installed($devel_pkg));

            # Return an error if neither the package nor its -devel variant (when allowed) is installed
            if (!$pkg_installed) {
                throw new FailedDependencyError(
                    message: "The requested action requires the '$pkg' package but it is not installed.",
                    response_id: 'DISPATCHER_MISSING_REQUIRED_PACKAGE',
                );
            }
        }

        # Loop through each package dependency and add it.
        foreach ($this->package_includes as $include) {
            if (!include_once $include) {
                throw new FailedDependencyError(
                    message: "Dispatcher requires include file $include but it could not be found in the PHP path.",
                    response_id: 'DISPATCHER_WITH_FAILED_INCLUDE',
                );
            }
        }
    }

    /**
     * Reads the PID file path that will be used for the next spawned process.
     */
    public function get_pid_file(): string {
        return $this->pid_file;
    }

    /**
     * Obtains the PIDs of any active processes spawned by this Dispatcher.
     * @returns array An array of PIDs of processes spawned by this Dispatcher.
     */
    public function get_running_processes(): array {
        # Variables
        $pids = [];

        # Loop through each existing proc file and checks its PID
        foreach (glob($this->pid_dir . $this->pid_file_prefix . '*.pid') as $pid_file) {
            $pid = (int) file_get_contents($pid_file);

            # If this PID is actively running, add it to the return array
            if (posix_getpgid($pid)) {
                $pids[] = $pid;
            }
            # Otherwise, remove the PID file
            else {
                unlink($pid_file);
            }
        }

        return $pids;
    }

    /**
     * Kills any running Dispatcher processes.
     */
    public function kill_running_processes(): void {
        # Terminate this Dispatcher's running processes
        foreach ($this->get_running_processes() as $pid) {
            posix_kill($pid, SIGKILL);
        }

        # Remove any leftover PID files
        foreach (glob($this->pid_dir . $this->pid_file_prefix . '*.pid') as $pid_file) {
            unlink($pid_file);
        }
    }

    /**
     * Spawns a new process for this Dispatcher. If this Dispatcher has the $async property enabled, this will spawn
     * the _process() method in the background. Otherwise, this behaves similar to the process() method by calling the
     * _process() method synchronously, but this method enforces the $max_concurrency property unlike process().
     * @param mixed ...$arguments Optional arguments to pass to the `_process()` method.
     * @return int The PID of the spawned process.
     * @throws ServiceUnavailableError When the maximum number of processes allowed by $max_concurrency is exceeded.
     */
    public function spawn_process(mixed ...$arguments): int {
        # Ensure required packages are present and required files are included
        $this->check_required_packages();

        # Convert the arguments to JSON so they can be passed to the spawned process
        $arguments_json = json_encode($arguments);
        $arguments_json = escapeshellarg($arguments_json);

        # Before we spawn a new process, ensure we don't have too many concurrent processes running now
        if ($this->max_queue and count($this->get_running_processes()) >= $this->max_queue) {
            throw new ServiceUnavailableError(
                message: "Dispatcher allows for a maximum of `$this->max_queue` processes to be queued at " .
                    "once, try again after `$this->timeout` seconds.",
                response_id: 'DISPATCHER_TOO_MANY_QUEUED_PROCESSES',
                retry_after: $this->timeout,
            );
        }

        # Spawn the process in the background if this Dispatcher is asynchronous
        if ($this->async) {
            $spawn_process = new Command(
                command: 'nohup sh ' .
                    self::DISPATCH_SCRIPT .
                    " $this->short_name $this->pid_file $this->timeout $arguments_json",
                redirect: ">/dev/null & echo $!",
            );
            $pid = is_numeric($spawn_process->output) ? (int) $spawn_process->output : null;

            # Ensure the spawn process output is a numeric PID and that the PID is actively running
            if (!is_null($pid) and posix_getpgid($pid)) {
                # Write this process's PID file, set the PID file name for the next process and return the PID.
                file_put_contents($this->pid_file, $pid);
                $this->pid_file = $this->pid_dir . uniqid(prefix: $this->pid_file_prefix) . '.pid';
                return $pid;
            }

            # Throw an error if we failed to spawn the Dispatcher process
            throw new ServerError(
                message: "Dispatcher `$this->full_name` failed to spawn new process. Received: $spawn_process->output",
                response_id: 'DISPATCHER_FAILED_TO_SPAWN_PROCESS',
            );
        }

        # Otherwise, call the `process()` directly and wait for it to complete
        # Since we are running this process synchronously, use this PHP fork's PID
        $pid = getmypid();
        file_put_contents($this->pid_file, $pid);

        # Call the process directly and remove the PID file after its done
        $this->process(...$arguments);
        unlink($this->pid_file);

        # Set the PID file name for the next process and return the PID.
        $this->pid_file = $this->pid_dir . uniqid(prefix: $this->pid_file_prefix) . '.pid';
        return $pid;
    }

    /**
     * Runs the _process() directly. This method is essentially a wrapper for _process() that checks for required
     * packages before starting. In almost every situation you should call spwan_process() instead of this method.
     */
    public function process(mixed ...$arguments): void {
        # Ensure required packages are present and required files are included
        $this->check_required_packages();

        # Call the protected _process() method to start the process synchronously.
        $this->_process(...$arguments);
    }

    /**
     * Defines what should be done when the Dispatch process is spawned. In most cases, this will restart some service
     * or perform computations that may take a long time. It is up to the child Dispatcher class to decide what is
     * done here.
     */
    protected function _process(mixed ...$arguments): void {
        # Emulate a long running process for testing purposes
        sleep(30);
    }

    /**
     * Configures this Dispatcher to run on a schedule if the `schedule` property is set.
     * @return CronJob|null Returns the CronJob created for this Dispatcher if a `schedule` is defined. Returns `null`
     * if no schedule was defined.
     */
    public function setup_schedule(): CronJob|null {
        # Only proceed if a schedule was requested
        if ($this->schedule) {
            # Ensure the requested schedule is supported
            if (count(explode(' ', $this->schedule)) !== 5) {
                throw new ServerError(
                    message: "Dispatcher schedule `$this->schedule` is not a supported schedule frequency.",
                    response_id: 'DISPATCHER_SCHEDULE_UNSUPPORTED',
                );
            }

            # Remove any existing scheduled CronJob for this Dispatcher
            $this->remove_schedule();

            # Create the cron job for this dispatcher
            $cron_expr = explode(' ', $this->schedule);
            $cron_job = new CronJob(
                require_pkg: false,
                minute: $cron_expr[0],
                hour: $cron_expr[1],
                mday: $cron_expr[2],
                month: $cron_expr[3],
                wday: $cron_expr[4],
                who: 'root',
                command: $this->schedule_command,
            );
            $cron_job->create();

            # Spawn this dispatcher process to ensure applicable values are populated before the next schedule runs
            $this->spawn_process();

            # Return the created CronJob
            return $cron_job;
        }

        return null;
    }

    /**
     * Removes the scheduled CronJob for this Dispatcher if it exists.
     */
    public function remove_schedule(): void {
        # Check if a cron job already exists for this dispatcher
        $dispatcher_cron_job_q = CronJob::query(command: $this->schedule_command);

        # Delete the cron job for this dispatcher if it exists, so we can recreate it with current values
        if ($dispatcher_cron_job_q->exists()) {
            $existing_cron_job = $dispatcher_cron_job_q->first();
            $existing_cron_job->packages = []; // Don't require the pfSense-pkg-Cron package to delete
            $existing_cron_job->delete();
        }
    }
}
