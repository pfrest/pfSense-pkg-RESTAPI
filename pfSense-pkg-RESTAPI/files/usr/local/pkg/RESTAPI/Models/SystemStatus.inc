<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FloatField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the system's current status.
 */
class SystemStatus extends Model {
    public StringField $platform;
    public StringField $serial;
    public StringField $netgate_id;
    public StringField $uptime;
    public StringField $bios_vendor;
    public StringField $bios_version;
    public StringField $bios_date;
    public BooleanField $kernel_pti;
    public StringField $mds_mitigation;
    public FloatField $temp_c;
    public FloatField $temp_f;
    public StringField $cpu_model;
    public FloatField $cpu_load_avg;
    public IntegerField $cpu_count;
    public FloatField $cpu_usage;
    public FloatField $mbuf_usage;
    public FloatField $mem_usage;
    public FloatField $swap_usage;
    public FloatField $disk_usage;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_system_status_internal';

        # Set model fields
        $this->platform = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Platform',
            help_text: "The verbose name of this system's platform.",
        );
        $this->serial = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Serial',
            help_text: "The system's unique serial number/ID.",
        );
        $this->netgate_id = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Netgate ID',
            help_text: 'The unique ID issued for this pfSense instance by Netgate.',
        );
        $this->uptime = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Uptime',
            help_text: 'The amount of time this system has been up since the last reboot.',
        );
        $this->bios_vendor = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'BIOS Vendor',
            help_text: 'The name of the BIOS vendor.',
        );
        $this->bios_version = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'BIOS Version',
            help_text: 'The BIOS version installed on this system.',
        );
        $this->bios_date = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'BIOS Date',
            help_text: 'The build date of the BIOS version.',
        );
        $this->kernel_pti = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: '1',
            indicates_false: '0',
            verbose_name: 'Kernel Pti',
            help_text: 'Indicates whether kernel PTI is enabled or not.',
        );
        $this->mds_mitigation = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'MDS Mitigation',
            help_text: 'Indicates whether MDS mitigation is enabled or not.',
        );
        $this->temp_c = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Temp C',
            help_text: 'The current system temperature in celsius.',
        );
        $this->temp_f = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Temp F',
            help_text: 'The current system temperature in fahrenheit.',
        );
        $this->cpu_model = new StringField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'CPU Model',
            help_text: 'The model of CPU installed in this system and other CPU info.',
        );
        $this->cpu_load_avg = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            many: true,
            delimiter: ' ',
            verbose_name: 'CPU Load Avg',
            help_text: 'The CPU load averages. The first value represents the load average for the last minute, the ' .
                'second value represents the load average for the last 5 minutes, and the third value represents the ' .
                'load average for the last 15 minutes.',
        );
        $this->cpu_count = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'CPU Count',
            help_text: 'The total number of CPUs cores available on this system.',
        );
        $this->cpu_usage = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'CPU Usage',
            help_text: 'The current CPU usage as a whole percentage. Note: This is an approximate usage based on the ' .
                'last minute load average and total number of CPU cores. This may not represent exact CPU usage.',
        );
        $this->mem_usage = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Mem Usage',
            help_text: 'The current memory usage as a whole percentage.',
        );
        $this->mbuf_usage = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'MBUF Usage',
            help_text: 'The current MBUF usage as a whole percentage.',
        );
        $this->swap_usage = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Swap Usage',
            help_text: 'The current swap usage as a whole percentage.',
        );
        $this->disk_usage = new FloatField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Disk Usage',
            help_text: 'The current disk usage as a whole percentage.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal values for the systems status. This method is the internally callable and is used to
     * populate the Field values for this Model.
     * @return array An array of internal system status values.
     */
    protected function get_system_status_internal(): array {
        return [
            'platform' => system_identify_specific_platform()['descr'],
            'serial' => system_get_serial(),
            'netgate_id' => system_get_uniqueid(),
            'uptime' => get_uptime(),
            'bios_vendor' => $this->get_bios_info('vendor'),
            'bios_version' => $this->get_bios_info('version'),
            'bios_date' => $this->get_bios_info('reldate'),
            'cpu_model' => get_single_sysctl('hw.model'),
            'kernel_pti' => get_single_sysctl('vm.pmap.pti'),
            'mds_mitigation' => get_single_sysctl('hw.mds_disable_state'),
            'temp_c' => $this->get_system_temp(),
            'temp_f' => $this->get_system_temp(as_fahrenheit: true),
            'cpu_load_avg' => $this->get_cpu_load_average(),
            'cpu_count' => get_single_sysctl('hw.ncpu'),
            'cpu_usage' => $this->get_cpu_usage(),
            'mbuf_usage' => $this->get_mbuf_usage(),
            'mem_usage' => mem_usage(),
            'swap_usage' => swap_usage(),
            'disk_usage' => $this->get_disk_usage(),
        ];
    }

    /**
     * Obtains the BIOS vendor internally.
     * @param string $bios_field The kenv smbios.bios.* field name to obtain
     * @return string The BIOS vendor's name
     */
    private function get_bios_info(string $bios_field): string {
        # Run the kenv command to obtain the requested BIOS information
        $kenv = new Command(command: "/bin/kenv -q smbios.bios.$bios_field 2>/dev/null", trim_whitespace: true);
        return $kenv->result_code === 0 ? $kenv->output : '';
    }

    /**
     * Obtains the system temperate in either Fahrenheit or Celsius.
     * @param bool $as_fahrenheit Obtain the system temperature in Fahrenheit instead of Celsius
     * @return float|null The current system temperature as a float OR null if the temperature could not be read.
     */
    private function get_system_temp(bool $as_fahrenheit = false): ?float {
        # Obtain the current system temperature in Celsius
        $temp = get_temp();

        # When the system cannot read temperature, $temp may be empty or non-numeric. If so, simply return null.
        if (!is_numeric($temp)) {
            return null;
        }

        # Now that we know $temp is numeric, convert it to a float
        $temp = (float) $temp;

        # Convert the value to Fahrenheit if requested, otherwise leave as is for Celsius
        return $as_fahrenheit ? $temp * 1.8 + 32 : $temp;
    }

    /**
     * Obtains the CPU load average. This method is responsible for obtaining the raw load average string from pfSense
     * and ensuring it is formatted correctly before the cpu_load_average field splits it into an array.
     * @return string|null The CPU load average as a string or null if it could not be obtained.
     */
    private function get_cpu_load_average(): ?string {
        # Obtain the load average using pfSense's get_load_average() function
        $load_avg = get_load_average();

        # The system language may affect the load average's delimiter (missing a comma in some languages)
        # ensure values are only ever separated by a single space (no commas). #716
        if (str_contains($load_avg, ', ')) {
            $load_avg = str_replace(', ', ' ', $load_avg);
        }

        return $load_avg ?? null;
    }

    /**
     * Determines approximate CPU usage using the last minute load average.
     * @return float|null The CPU usage as a whole percentage.
     */
    private function get_cpu_usage(): ?float {
        # Determine the last minute load average and total number of cpu cores
        $load_avg = explode(', ', get_load_average());
        $load_avg_last_min = (float) $load_avg[0];
        $cpu_count = (int) get_single_sysctl('hw.ncpu');

        # Determine the CPU usage by dividing the last minute load avg by the cpu count and multiplying by 100
        $cpu_usage = ($load_avg_last_min / $cpu_count) * 100;

        # If the usage estimate is above 100%, ensure it is capped at 100
        return $cpu_usage <= 100 ? $cpu_usage : 100;
    }

    /**
     * Obtains the current MBUF usage.
     * @return float|null The current MBUF usage as a whole percentage.
     */
    private function get_mbuf_usage(): ?float {
        # get_mbuf() sets values by reference so ensure we have actual variables set before calling
        $mbufs_text = null;
        $mbuf_usage = null;

        # Determine the current MBUF usage and return it.
        get_mbuf($mbufs_text, $mbuf_usage);

        return isset($mbuf_usage) ? (float) $mbuf_usage : null;
    }

    /**
     * Obtains the current disk usage for the root mountpoint.
     * @return float|null The current disk usage as a whole percentage.
     */
    private function get_disk_usage(): ?float {
        foreach (get_mounted_filesystems() as $fs) {
            if ($fs['mountpoint'] === '/') {
                return (float) $fs['percent_used'];
            }
        }

        return null;
    }
}
