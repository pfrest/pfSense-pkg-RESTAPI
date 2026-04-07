<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a model for interacting with the BIND sync remote servers (peers)
 */
class BINDSyncRemoteHost extends Model {
    public BooleanField $syncdestinenable;
    public StringField $syncprotocol;
    public StringField $ipaddress;
    public PortField $syncport;
    public StringField $username;
    public StringField $password;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/bindsync/config/0/row';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'BIND Sync Remote Host';

        # Set model fields
        $this->syncdestinenable = new BooleanField(
            default: false,
            indicates_true: 'ON',
            verbose_name: 'Sync Destination Enable',
            help_text: 'Enable this remote host for syncing.',
        );
        $this->syncprotocol = new StringField(
            required: true,
            choices: ['http', 'https'],
            verbose_name: 'Sync Protocol',
            help_text: 'The protocol to use for syncing.',
        );
        $this->ipaddress = new StringField(
            required: true,
            validators: [new IPAddressValidator(allow_fqdn: true)],
            verbose_name: 'IP Address',
            help_text: 'The IP address/hostname of the remote host.',
        );
        $this->syncport = new PortField(required: true, verbose_name: 'Sync Port', help_text: 'The remote host port to use for syncing.');
        $this->username = new StringField(
            required: true,
            verbose_name: 'Username',
            help_text: 'The username to use to authenticate when syncing.',
        );
        $this->password = new StringField(
            required: true,
            sensitive: true,
            verbose_name: 'Password',
            help_text: 'The password to use to authenticate when syncing.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the BIND sync settings.
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
