<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;

class OpenVPNServerRouteStatus extends Model {
    public StringField $common_name;
    public StringField $remote_host;
    public StringField $virtual_addr;
    public StringField $last_time;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'OpenVPNServerStatus';
        $this->internal_callable = 'get_openvpn_route_statuses';
        $this->verbose_name = 'OpenVPN Server Route Status';
        $this->many = true;

        # Set model fields
        $this->common_name = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Common Name',
            help_text: 'The common name of the OpenVPN server connection.',
        );
        $this->remote_host = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Remote Host',
            help_text: 'The remote host of the OpenVPN server connection.',
        );
        $this->virtual_addr = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Virtual Addr',
            help_text: 'The virtual address of the OpenVPN server connection.',
        );
        $this->last_time = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Last Time',
            help_text: 'The last time of the route was used.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the OpenVPN server route statuses in their internal format.
     * @return array The array of OpenVPN server statuses in their internal format.
     */
    protected function get_openvpn_route_statuses(): array {
        return openvpn_get_active_servers()[$this->parent_id]['routes'];
    }
}
