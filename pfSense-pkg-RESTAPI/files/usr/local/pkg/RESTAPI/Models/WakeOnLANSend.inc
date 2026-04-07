<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;
use RESTAPI\Validators\MACAddressValidator;

/**
 * Defines a Model for sending wake-on-LAN packets to hosts on the network.
 */
class WakeOnLANSend extends Model {
    public InterfaceField $interface;
    public StringField $mac_addr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->many = false;
        $this->internal_callable = 'void_internal_callable';
        $this->verbose_name = 'Wake-on-LAN Packet';

        # Set model fields
        $this->interface = new InterfaceField(
            required: true,
            verbose_name: 'Interface',
            help_text: 'The interface the host to be woken up is connected to.',
        );
        $this->mac_addr = new StringField(
            required: true,
            validators: [new MACAddressValidator()],
            verbose_name: 'MAC Addr',
            help_text: 'The MAC address of the host to be awoken.',
        );
        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation for the `interface` field.
     * @param string $interface The incoming value to be validated.
     * @returns string The validated value to be set.
     * @throws ValidationError When $interface does not have a valid broadcast address
     */
    public function validate_interface(string $interface): string {
        # Throw an error if the $interface does not have a valid IPv4 broadcast address.
        if (!$this->interface->get_related_model()->get_broadcast_ipv4()) {
            throw new ValidationError(
                message: 'Field `interface` must have a valid IPv4 broadcast address.',
                response_id: 'WAKE_ON_LAN_SEND_INTERFACE_MUST_HAVE_BROADCAST_ADDRESS',
            );
        }

        return $interface;
    }

    /**
     * Send a wake-on-LAN packet from the `interface` broadcast IP to the specified `mac_addr`.
     */
    public function _create(): void {
        # Send a WOL packet to the requested MAC address from the requested interface
        $if_broadcast_ip = $this->interface->get_related_model()->get_broadcast_ipv4();
        $mac_addr = $this->mac_addr->value;
        $wol = new Command("/usr/local/bin/wol -i $if_broadcast_ip $mac_addr");

        # Throw an error if the WOL command failed
        if ($wol->result_code !== 0) {
            throw new ServerError(
                message: "Wake-on-LAN command failed: {$wol->output}",
                response_id: 'WAKE_ON_LAN_SEND_COMMAND_FAILED',
            );
        }
    }

    /**
     * Returns an empty array to act as the internal callable for this Model. This Model does not support reads so
     * this method essentially reads nothing.
     * @returns array An empty array to act as a blank internal callable.
     */
    public function void_internal_callable(): array {
        return [];
    }
}
