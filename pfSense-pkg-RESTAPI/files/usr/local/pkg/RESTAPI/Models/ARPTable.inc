<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interacting with the system's ARP table.
 */
class ARPTable extends Model {
    public StringField $hostname;
    public StringField $ip_address;
    public StringField $mac_address;
    public StringField $interface;
    public StringField $type;
    public BooleanField $permanent;
    public StringField $dnsresolve;
    public StringField $expires;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->many = true;
        $this->internal_callable = 'get_arp_table';
        $this->verbose_name = 'ARP Table Entry';
        $this->verbose_name_plural = 'ARP Table Entries';

        # Define model Fields
        $this->ip_address = new StringField(
            read_only: true,
            internal_name: 'ip-address',
            verbose_name: 'IP Address',
            help_text: 'The IP address associated with the ARP entry.',
        );
        $this->mac_address = new StringField(
            read_only: true,
            internal_name: 'mac-address',
            verbose_name: 'MAC Address',
            help_text: 'The MAC address associated with the ARP entry.',
        );
        $this->hostname = new StringField(
            allow_empty: true,
            read_only: true,
            verbose_name: 'Hostname',
            help_text: 'The hostname associated with the ARP entry.',
        );
        $this->dnsresolve = new StringField(
            read_only: true,
            verbose_name: 'DNS Resolve',
            help_text: 'The full DNS name associated with this ARP entry.',
        );
        $this->interface = new StringField(
            read_only: true,
            verbose_name: 'Interface',
            help_text: 'The interface where the ARP entry was registered.',
        );
        $this->type = new StringField(read_only: true, verbose_name: 'Type', help_text: 'The type of connection this host utilizes.');
        $this->permanent = new BooleanField(
            read_only: true,
            indicates_true: true,
            indicates_false: false,
            verbose_name: 'Permanent',
            help_text: 'Indicates whether the ARP entry is permanent in the ARP table.',
        );
        $this->expires = new StringField(read_only: true, verbose_name: 'Expires', help_text: 'The expiration time for this ARP entry.');

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Fetches the ARP table contents. This method is the internal callable used to read the internal ARP table entries.
     */
    function get_arp_table(): array {
        return prepare_ARP_table();
    }

    /**
     * Deletes this Model object from the ARP table.
     */
    public function _delete() {
        $arp_cmd = new Command('/usr/sbin/arp -d ' . escapeshellarg($this->ip_address->value));
    }
}
