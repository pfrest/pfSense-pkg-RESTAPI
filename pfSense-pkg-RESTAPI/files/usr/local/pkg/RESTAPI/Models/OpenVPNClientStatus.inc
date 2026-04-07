<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

class OpenVPNClientStatus extends Model {
    public StringField $name;
    public PortField $port;
    public ForeignModelField $vpnid;
    public StringField $mgmt;
    public StringField $status;
    public StringField $state;
    public StringField $state_detail;
    public StringField $connect_time;
    public StringField $virtual_addr;
    public StringField $virtual_addr6;
    public StringField $remote_host;
    public PortField $remote_port;
    public StringField $local_host;
    public PortField $local_port;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_openvpn_client_statuses';
        $this->verbose_name = 'OpenVPN Client Status';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Name',
            help_text: 'The name of the OpenVPN client.',
        );
        $this->port = new PortField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Port',
            help_text: 'The port number of the OpenVPN client.',
        );
        $this->vpnid = new ForeignModelField(
            model_name: 'OpenVPNClient',
            model_field: 'vpnid',
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'VPN ID',
            help_text: 'The VPN ID of the OpenVPN client this status corresponds to.',
        );
        $this->mgmt = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Mgmt',
            help_text: 'The management interface of the OpenVPN client.',
        );
        $this->status = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Status',
            help_text: 'The current status of the OpenVPN client.',
        );
        $this->state = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'State',
            help_text: 'The current state of the OpenVPN client.',
        );
        $this->state_detail = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'State Detail',
            help_text: 'The details for the current state of the OpenVPN client.',
        );
        $this->connect_time = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Connect Time',
            help_text: 'The connection time of the OpenVPN client.',
        );
        $this->remote_host = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Remote Host',
            help_text: 'The remote host of the OpenVPN client.',
        );
        $this->remote_port = new PortField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Remote Port',
            help_text: 'The remote port of the OpenVPN client.',
        );
        $this->local_host = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Local Host',
            help_text: 'The local host of the OpenVPN client.',
        );
        $this->local_port = new PortField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Local Port',
            help_text: 'The local port of the OpenVPN client.',
        );
        $this->virtual_addr = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Virtual Addr',
            help_text: 'The virtual address of the OpenVPN client.',
        );
        $this->virtual_addr6 = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Virtual Addr6',
            help_text: 'The virtual address 6 of the OpenVPN client.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal OpenVPN client status values. This is this Model's internal callable.
     * @return array The array of OpenVPN client statuses in their internal format.
     */
    protected function get_openvpn_client_statuses(): array {
        return openvpn_get_active_clients();
    }
}
