<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\FirewallApplyDispatcher;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents the outbound NAT mode.
 */
class OutboundNATMode extends Model {
    public StringField $mode;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'nat/outbound';

        # Set model fields
        $this->mode = new StringField(
            required: true,
            choices: ['automatic', 'hybrid', 'advanced', 'disabled'],
            verbose_name: 'Mode',
            help_text: 'The outbound NAT mode to assign this system. Set to `automatic` to have this system ' .
                'automatically generate NAT rules this firewall, `hybrid` to automatically generate NAT rules AND ' .
                'allow manual outbound NAT mappings to be assigned, `manual` to prevent the system from automatically ' .
                'generating NAT rules and only allow manual outbound NAT mappings, or `disabled` to disable outbound ' .
                'NAT on this system entirely.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the outbound NAT mode.
     */
    public function apply() {
        (new FirewallApplyDispatcher(async: $this->async))->spawn_process();
    }
}
