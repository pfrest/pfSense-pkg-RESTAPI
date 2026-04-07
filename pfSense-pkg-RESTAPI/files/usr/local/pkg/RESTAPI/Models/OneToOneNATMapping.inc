<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\FilterAddressField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\SpecialNetworkField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model that represents 1:1 NAT mappings.
 */
class OneToOneNATMapping extends Model {
    public InterfaceField $interface;
    public BooleanField $disabled;
    public BooleanField $nobinat;
    public StringField $natreflection;
    public StringField $ipprotocol;
    public SpecialNetworkField $external;
    public FilterAddressField $source;
    public FilterAddressField $destination;
    public StringField $descr;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'nat/onetoone';
        $this->subsystem = 'natconf';
        $this->verbose_name = '1:1 NAT mapping';
        $this->many = true;

        # Set model fields
        $this->interface = new InterfaceField(
            required: true,
            verbose_name: 'Interface',
            help_text: 'The interface this 1:1 NAT mapping applies to.',
        );
        $this->disabled = new BooleanField(default: false, verbose_name: 'Disabled', help_text: 'Disables this 1:1 NAT mapping.');
        $this->nobinat = new BooleanField(
            default: false,
            verbose_name: 'No Binat',
            help_text: 'Exclude traffic matching this mapping from a later, more general, mapping.',
        );
        $this->natreflection = new StringField(
            default: null,
            choices: ['enable', 'disable'],
            allow_null: true,
            verbose_name: 'NAT Reflection',
            help_text: 'Enables or disables NAT reflection for traffic matching this mapping. Set to `null` to use ' .
                'the system default.',
        );
        $this->ipprotocol = new StringField(
            default: 'inet',
            choices: ['inet', 'inet6'],
            verbose_name: 'IP Protocol',
            help_text: 'The IP version this mapping applies to.',
        );
        $this->external = new SpecialNetworkField(
            required: true,
            allow_ipaddr: true,
            allow_subnet: false,
            allow_alias: false,
            allow_interface: false,
            allow_interface_ip: true,
            allow_interface_groups: false,
            allow_self: false,
            allow_l2tp: false,
            allow_pppoe: false,
            verbose_name: 'External',
            help_text: 'The external IP address or interface for the 1:1 mapping.',
        );
        $this->source = new FilterAddressField(
            required: true,
            allow_ipaddr: true,
            allow_subnet: true,
            allow_alias: false,
            allow_interface: true,
            allow_interface_ip: true,
            allow_interface_groups: false,
            allow_any: true,
            allow_invert: true,
            allow_self: false,
            allow_l2tp: true,
            allow_pppoe: true,
            verbose_name: 'Source',
            help_text: 'The source IP address or subnet that traffic must match to apply this mapping.',
        );
        $this->destination = new FilterAddressField(
            required: true,
            allow_ipaddr: true,
            allow_subnet: true,
            allow_alias: true,
            allow_interface: true,
            allow_interface_ip: true,
            allow_interface_groups: false,
            allow_any: true,
            allow_invert: true,
            allow_self: false,
            allow_l2tp: true,
            allow_pppoe: true,
            verbose_name: 'Destination',
            help_text: 'The destination IP address or subnet that traffic must match to apply this mapping.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this 1:1 NAT mapping',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Adds extra validation to the `external` field.
     * @param string $external The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $external is an IP that does not match the current `ipprotocol` field value.
     */
    public function validate_external(string $external): string {
        # Variables
        $ipproto = $this->ipprotocol->value;
        $is_ipv4 = $this->external->has_label('is_ipaddrv4');
        $is_ipv6 = $this->external->has_label('is_ipaddrv6');

        # Throw an error if this value does not match the current ipprotocol
        if ($ipproto === 'inet' and $is_ipv6 or $ipproto === 'inet6' and $is_ipv4) {
            throw new ValidationError(
                message: 'Field `external` is not an IP address of the IP family assigned in `ipprotocol`.',
                response_id: 'ONE_TO_ONE_NAT_MAPPING_EXTERNAL_IP_DOES_NOT_MATCH_IPPROTOCOL',
            );
        }

        return $external;
    }

    /**
     * Adds extra validation to the `source` field.
     * @param string $source The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $source is an IP that does not match the current `ipprotocol` field value.
     * @throws ValidationError When $source is a subnet that does not match the current `ipprotocol` field value.
     */
    public function validate_source(string $source): string {
        # Variables
        $ipproto = $this->ipprotocol->value;
        $is_ipv4 = $this->source->has_label('is_ipaddrv4');
        $is_ipv6 = $this->source->has_label('is_ipaddrv6');
        $is_subnetv4 = $this->source->has_label('is_subnetv4');
        $is_subnetv6 = $this->source->has_label('is_subnetv6');

        # Throw an error if this value is an IP that does not match the current ipprotocol
        if ($ipproto === 'inet' and $is_ipv6 or $ipproto === 'inet6' and $is_ipv4) {
            throw new ValidationError(
                message: 'Field `source` is not an IP address of the IP family assigned in `ipprotocol`.',
                response_id: 'ONE_TO_ONE_NAT_MAPPING_SOURCE_IP_DOES_NOT_MATCH_IPPROTOCOL',
            );
        }

        # Throw an error if this value is a subnet that does not match the current ipprotocol
        if ($ipproto === 'inet' and $is_subnetv6 or $ipproto === 'inet6' and $is_subnetv4) {
            throw new ValidationError(
                message: 'Field `source` is not an subnet of the IP family assigned in `ipprotocol`.',
                response_id: 'ONE_TO_ONE_NAT_MAPPING_SOURCE_SUBNET_DOES_NOT_MATCH_IPPROTOCOL',
            );
        }

        return $source;
    }

    /**
     * Adds extra validation to the `destination` field.
     * @param string $destination The incoming value to be validated.
     * @return string The validated value to be assigned.
     * @throws ValidationError When $destination is an IP that does not match the current `ipprotocol` field value.
     * @throws ValidationError When $destination is a subnet that does not match the current `ipprotocol` field value.
     */
    public function validate_destination(string $destination): string {
        # Variables
        $ipproto = $this->ipprotocol->value;
        $is_ipv4 = $this->destination->has_label('is_ipaddrv4');
        $is_ipv6 = $this->destination->has_label('is_ipaddrv6');
        $is_subnetv4 = $this->destination->has_label('is_subnetv4');
        $is_subnetv6 = $this->destination->has_label('is_subnetv6');

        # Throw an error if this value is an IP that does not match the current ipprotocol
        if ($ipproto === 'inet' and $is_ipv6 or $ipproto === 'inet6' and $is_ipv4) {
            throw new ValidationError(
                message: 'Field `destination` is not an IP address of the IP family assigned in `ipprotocol`.',
                response_id: 'ONE_TO_ONE_NAT_MAPPING_destination_IP_DOES_NOT_MATCH_IPPROTOCOL',
            );
        }

        # Throw an error if this value is a subnet that does not match the current ipprotocol
        if ($ipproto === 'inet' and $is_subnetv6 or $ipproto === 'inet6' and $is_subnetv4) {
            throw new ValidationError(
                message: 'Field `destination` is not an subnet of the IP family assigned in `ipprotocol`.',
                response_id: 'ONE_TO_ONE_NAT_MAPPING_destination_SUBNET_DOES_NOT_MATCH_IPPROTOCOL',
            );
        }

        return $destination;
    }

    /**
     * Applies changes to 1:1 NAT mappings.
     */
    public function apply(): void {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
