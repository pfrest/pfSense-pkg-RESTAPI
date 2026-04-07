<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;

class OpenVPNServerConnectionStatus extends Model {
    public StringField $common_name;
    public StringField $remote_host;
    public StringField $virtual_addr;
    public StringField $virtual_addr6;
    public IntegerField $bytes_recv;
    public IntegerField $bytes_sent;
    public StringField $connect_time;
    public IntegerField $connect_time_unix;
    public StringField $user_name;
    public IntegerField $client_id;
    public IntegerField $peer_id;
    public StringField $cipher;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->parent_model_class = 'OpenVPNServerStatus';
        $this->internal_callable = 'get_openvpn_conn_statuses';
        $this->verbose_name = 'OpenVPN Server Connection Status';
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
        $this->virtual_addr6 = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Virtual Addr6',
            help_text: 'The virtual IPv6 address of the OpenVPN server connection.',
        );
        $this->bytes_recv = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Bytes Recv',
            help_text: 'The number of bytes received by the OpenVPN server connection.',
        );
        $this->bytes_sent = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Bytes Sent',
            help_text: 'The number of bytes sent by the OpenVPN server connection.',
        );
        $this->connect_time = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Connect Time',
            help_text: 'The connection time of the OpenVPN server connection.',
        );
        $this->connect_time_unix = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Connect Time Unix',
            help_text: 'The connection time of the OpenVPN server connection in Unix time.',
        );
        $this->user_name = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'User Name',
            help_text: 'The user name of the OpenVPN server connection.',
        );
        $this->client_id = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Client ID',
            help_text: 'The client ID of the OpenVPN server connection.',
        );
        $this->peer_id = new IntegerField(
            default: null,
            allow_null: true,
            read_only: true,
            verbose_name: 'Peer ID',
            help_text: 'The peer ID of the OpenVPN server connection.',
        );
        $this->cipher = new StringField(
            default: null,
            allow_empty: true,
            allow_null: true,
            read_only: true,
            verbose_name: 'Cipher',
            help_text: 'The cipher of the OpenVPN server connection.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the internal OpenVPN server connection status values. This is this Model's internal callable.
     * @return array The array of OpenVPN server statuses in their internal format.
     */
    protected function get_openvpn_conn_statuses(): array {
        return openvpn_get_active_servers()[$this->parent_id]['conns'];
    }

    /**
     * Kills this OpenVPN server connection.
     */
    public function _delete(): void {
        # Attempt to kill the OpenVPN server connection
        $result = openvpn_kill_client(
            port: $this->parent_model->mgmt->value,
            remipp: $this->remote_host->value,
            client_id: $this->client_id->value,
        );

        # Throw an error if the connection failed to be killed
        if ($result !== 0) {
            throw new ServerError(
                message: 'OpenVPN Server Connection failed to be killed. Check the OpenVPN logs for more info.',
                response_id: 'OPENVPN_SERVER_CONNECTION_STATUS_CLIENT_FAILED_TO_BE_KILLED',
            );
        }
    }
}
