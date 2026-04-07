<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

class OpenVPNServerStatus extends Model {
    public StringField $name;
    public StringField $mode;
    public PortField $port;
    public ForeignModelField $vpnid;
    public StringField $mgmt;
    public NestedModelField $conns;
    public NestedModelField $routes;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_openvpn_server_statuses';
        $this->verbose_name = 'OpenVPN Server Status';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Name',
            help_text: 'The name of the OpenVPN server.',
        );
        $this->port = new PortField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Port',
            help_text: 'The port number of the OpenVPN server.',
        );
        $this->vpnid = new ForeignModelField(
            model_name: 'OpenVPNServer',
            model_field: 'vpnid',
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'VPN ID',
            help_text: 'The VPN ID of the OpenVPN server this status corresponds to.',
        );
        $this->mgmt = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Mgmt',
            help_text: 'The management interface of the OpenVPN server.',
        );
        $this->mode = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Mode',
            help_text: 'The mode of the OpenVPN server.',
        );
        $this->conns = new NestedModelField(
            model_class: 'OpenVPNServerConnectionStatus',
            default: [],
            allow_empty: true,
            read_only: true,
            verbose_name: 'Conns',
            help_text: 'The current connections to the OpenVPN server.',
        );
        $this->routes = new NestedModelField(
            model_class: 'OpenVPNServerRouteStatus',
            default: [],
            allow_empty: true,
            read_only: true,
            verbose_name: 'Routes',
            help_text: 'The current routes of the OpenVPN server.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal OpenVPN server status values. This is this Model's internal callable.
     * @return array The array of OpenVPN server statuses in their internal format.
     */
    protected function get_openvpn_server_statuses(): array {
        return openvpn_get_active_servers();
    }
}
