<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;

/**
 * Defines a model that represents a DHCP lease.
 */
class DHCPServerLease extends Model {
    const ISC_LEASES_FILE = '/var/dhcpd/var/db/dhcpd.leases';

    public StringField $ip;
    public StringField $mac;
    public StringField $hostname;
    public InterfaceField $if;
    public StringField $starts;
    public StringField $ends;
    public StringField $active_status;
    public StringField $online_status;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_dhcp_leases';
        $this->many = true;
        $this->always_apply = true;

        # Set model fields
        $this->ip = new StringField(default: null, allow_null: true, verbose_name: 'IP', help_text: 'The IP address of the lease.');
        $this->mac = new StringField(default: null, allow_null: true, verbose_name: 'MAC', help_text: 'The MAC address of the lease.');
        $this->hostname = new StringField(default: null, allow_null: true, verbose_name: 'Hostname', help_text: 'The hostname of the lease.');
        $this->if = new InterfaceField(
            default: null,
            allow_null: true,
            verbose_name: 'Interface Name',
            help_text: 'The interface the lease was registered on.',
        );
        $this->starts = new StringField(default: null, allow_null: true, verbose_name: 'Starts', help_text: 'The start time of the lease.');
        $this->ends = new StringField(default: null, allow_null: true, verbose_name: 'Ends', help_text: 'The end time of the lease.');
        $this->active_status = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'act',
            verbose_name: 'Active Status',
            help_text: 'The active status of the lease.',
        );
        $this->online_status = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'online',
            verbose_name: 'Online Status',
            help_text: 'The online status of the lease.',
        );
        $this->descr = new StringField(default: null, allow_null: true, verbose_name: 'Description', help_text: 'The description of the lease.');

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the DHCP leases from the system internally. This is the internal callable for the Model.
     * @return array An array of recent/current DHCP leases
     */
    protected function get_dhcp_leases(): array {
        return system_get_dhcpleases()['lease'];
    }

    /**
     * Deletes a DHCP lease from the system.
     */
    public function _delete(): void {
        # Check the DHCP server backend used
        $dhcp_backend = new DHCPServerBackend();

        # Delete the DHCP lease via dhcpd
        if ($dhcp_backend->dhcpbackend->value === 'isc') {
            # Stop the DHCP server, remove the lease, and restart the DHCP server
            killbyname('dhcpd');
            $this->isc_remove_lease(file_get_contents(self::ISC_LEASES_FILE), $this->ip->value);
        }
        # Delete the DHCP lease via kea
        elseif ($dhcp_backend->dhcpbackend->value === 'kea') {
            system_del_kea4lease($this->ip->value);
        }
        # Otherwise, the DHCP server backend is not supported
        else {
            throw new ServerError(
                message: "The DHCP server backend '{$dhcp_backend->dhcpbackend->value}' is not known.",
                response_id: 'DHCP_SERVER_LEASE_UNKNOWN_DHCP_SERVER_BACKEND',
            );
        }
    }

    /**
     * Clears all existing DHCP server leases
     */
    public static function delete_all(): ModelSet {
        # Read all existing leases and determine the DHCP server backend
        $leases = self::read_all();
        $dhcp_backend = new DHCPServerBackend();

        # Clear all leases
        if ($dhcp_backend->dhcpbackend->value === 'isc') {
            # Stop the DHCP server, wait a couple seconds, remove the leases, and restart the DHCP server
            killbyname('dhcpd');
            sleep(2);
            unlink_if_exists(self::ISC_LEASES_FILE . '*');
            services_dhcpd_configure();
        } elseif ($dhcp_backend->dhcpbackend->value === 'kea') {
            system_clear_all_kea4leases();
        } else {
            throw new ServerError(
                message: "The DHCP server backend '{$dhcp_backend->dhcpbackend->value}' is not known.",
                response_id: 'DHCP_SERVER_LEASE_UNKNOWN_DHCP_SERVER_BACKEND',
            );
        }

        return $leases;
    }

    /**
     * Reads an existing DHCP lease file and removes the lease with the specified IP address.
     * @param string $leases_file_contents The raw contents of the DHCP leases file.
     * @param string $ip The IP address of the lease to remove.
     * @return string The new contents of the DHCP leases file.
     */
    public static function isc_remove_lease(string $leases_file_contents, string $ip): string {
        # Split the leases file into lines and add variable to store new contents
        $leases_lines = explode("\n", $leases_file_contents);
        $new_leases_contents = [];

        # Iterate over each line in the leases file
        $in_declaration = false;
        foreach ($leases_lines as $line) {
            # Skip the current line if we are in the middle of a lease declaration
            if ($in_declaration) {
                # If the line ends the lease declaration, stop skipping lines
                if (str_starts_with('}', trim($line))) {
                    $in_declaration = false;
                }
                continue;
            }

            # If the line starts the lease declaration we're looking for, start skipping lines
            if (str_starts_with("lease $ip {", trim($line))) {
                $in_declaration = true;
                continue;
            }

            # Otherwise, add the line to the new leases contents
            $new_leases_contents[] = $line;
        }

        return implode("\n", $new_leases_contents);
    }
}
