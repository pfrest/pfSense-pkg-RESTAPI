<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\InterfaceField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a Model for interacting with the DHCP relay configuration.
 */
class DHCPRelay extends Model {
    public BooleanField $enable;
    public InterfaceField $interface;
    public BooleanField $agentoption;
    public ForeignModelField $carpstatusvip;
    public StringField $server;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'dhcrelay';
        $this->many = false;
        $this->always_apply = true;

        # Set model fields
        $this->enable = new BooleanField(
            default: false,
            verbose_name: 'Enable',
            help_text: 'Enables or disables the DHCP relay.',
        );
        $this->interface = new InterfaceField(
            default: [],
            many: true,
            verbose_name: 'Interface',
            help_text: 'The downstream interfaces to listen on for DHCP requests.',
        );
        $this->agentoption = new BooleanField(
            default: false,
            verbose_name: 'Agent Option',
            help_text: 'Enables or disables appending the circuit ID (interface number) and the agent ID to the ' .
                'DHCP request.',
        );
        $this->carpstatusvip = new ForeignModelField(
            model_name: 'VirtualIP',
            model_field: 'uniqid',
            allowed_keywords: ['none'],
            default: 'none',
            verbose_name: 'CARP Status VIP',
            help_text: 'DHCP Relay will be stopped when the chosen VIP is in BACKUP status, and started ' .
                'in MASTER status.',
        );
        $this->server = new StringField(
            required: true,
            many: true,
            many_minimum: 1,
            validators: [new IPAddressValidator(allow_ipv6: false)],
            verbose_name: 'Server',
            help_text: 'The IPv4 addresses of the DHCP server to relay requests to.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Provide extra validation to the 'enable' field.
     * @param bool $enable The value to be validated.
     * @return bool The validated value.
     * @throws ConflictError If the DHCP relay is being enabled but a DHCP Server is running.
     */
    public function validate_enable(bool $enable): bool {
        if ($enable and DHCPServer::query(enable: true)->exists()) {
            throw new ConflictError(
                message: 'DHCP Relay cannot be enabled while DHCP Server is enabled on any interface.',
                response_id: 'DHCP_RELAY_CANNOT_BE_ENABLED_WHILE_DHCP_SERVER_IS_ENABLED',
            );
        }

        return $enable;
    }

    /**
     * Override the default 'to_internal()' method to ensure the carpstatusvip field is prefixed with '_vip'.
     */
    public function to_internal(): array|string {
        $internal_data = parent::to_internal();

        if ($this->carpstatusvip->value !== 'none') {
            $internal_data['carpstatusvip'] = "_vip{$internal_data['carpstatusvip']}";
        }

        return $internal_data;
    }

    /**
     * Add custom logic for when the 'carpstatusvip' field is being set from its internal value. This is used to
     * remove the _vip prefix from the value so the framework can locate it's related VirtualIP model.
     */
    public function from_internal_carpstatusvip(string $value): string {
        if (str_starts_with($value, '_vip')) {
            return substr($value, 4);
        }

        return $value;
    }

    /**
     * Apply changes to the DHCP relay
     */
    public function apply(): void {
        services_dhcrelay_configure();
        filter_configure();
    }
}
