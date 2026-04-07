<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Core\ModelSet;
use RESTAPI\Dispatchers\FirewallStateTableResetDispatcher;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interacting with the firewall states.
 */
class FirewallState extends Model {
    public StringField $interface;
    public StringField $protocol;
    public StringField $direction;
    public StringField $source;
    public StringField $destination;
    public StringField $state;
    public StringField $age;
    public StringField $expires_in;
    public IntegerField $packets_total;
    public IntegerField $packets_in;
    public IntegerField $packets_out;
    public IntegerField $bytes_total;
    public IntegerField $bytes_in;
    public IntegerField $bytes_out;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_pf_states';
        $this->many = true;

        # Set model fields
        # TODO: Change this to an InterfaceField once InterfaceFields can store values as real interface IDs
        $this->interface = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'if',
            verbose_name: 'Interface',
            help_text: 'The interface that initially received the traffic which registered the state.',
        );
        $this->protocol = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'proto',
            verbose_name: 'Protocol',
            help_text: 'The protocol listed in the state.',
        );
        $this->direction = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'Direction',
            help_text: 'The direction of traffic listed in the state.',
        );
        $this->source = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'src',
            verbose_name: 'Source',
            help_text: 'The source address listed in the state. Note: Depending on the `protocol`, this value may ' .
                'contain the source port as well.',
        );
        $this->destination = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'dst',
            verbose_name: 'Destination',
            help_text: 'The destination address listed in the state. Note: Depending on the `protocol`, this value may ' .
                'contain the destination port as well.',
        );
        $this->state = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'State',
            help_text: 'The current status of the firewall state.',
        );
        $this->age = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'Age',
            help_text: 'The age of the firewall state in HH:MM:SS format.',
        );
        $this->expires_in = new StringField(
            default: null,
            allow_null: true,
            internal_name: 'expires in',
            verbose_name: 'Expires In',
            help_text: 'The amount of time remaining until the state expires in HH:MM:SS format.',
        );
        $this->packets_total = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'packets total',
            verbose_name: 'Packets Total',
            help_text: 'The total number of packets observed by the state.',
        );
        $this->packets_in = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'packets in',
            verbose_name: 'Packets In',
            help_text: 'The total number of inbound packets observed by the state.',
        );
        $this->packets_out = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'packets out',
            verbose_name: 'Packets Out',
            help_text: 'The total number of outbound packets observed by the state.',
        );
        $this->bytes_total = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'bytes total',
            verbose_name: 'Bytes Total',
            help_text: 'The total number of traffic (in bytes) observed by the state.',
        );
        $this->bytes_in = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'bytes in',
            verbose_name: 'Bytes In',
            help_text: 'The total number of inbound traffic (in bytes) observed by the state.',
        );
        $this->bytes_out = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'bytes out',
            verbose_name: 'Bytes Out',
            help_text: 'The total number of outbound traffic (in bytes) observed by the state.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains all current firewall states from pf
     * @return array The array of states currently registered in pf
     */
    protected function get_pf_states(): array {
        return pfSense_get_pf_states();
    }

    /**
     * Resets the entire state table.
     * @note This action is performed in the background and on a delay to prevent the API from hanging or timing out.
     */
    public static function delete_all(): ModelSet|Model {
        # Initiate the state table reset in the background
        $dispatcher = new FirewallStateTableResetDispatcher(async: true);
        $dispatcher->spawn_process();
        return new ModelSet();
    }

    /**
     * Deletes this FirewallState from pf. This method should not be called directly, use the `delete()` method instead.
     */
    protected function _delete(): void {
        # Obtain the IP from the source and destination. This is required by pfSense_kill_states()
        $source = $this->source->value;
        $source_ip = str_contains($source, ':') ? explode(':', $source)[0] : $source;
        $destination = $this->destination->value;
        $destination_ip = str_contains($destination, ':') ? explode(':', $destination)[0] : $destination;

        # Kill states with this source and destination IP combos
        pfSense_kill_states($source_ip, $destination_ip);
    }
}
