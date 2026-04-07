<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * A Model that provides the status of IPsec tunnels.
 */
class IPsecSAStatus extends Model {
    public StringField $con_id;
    public IntegerField $uniqueid;
    public IntegerField $version;
    public StringField $state;
    public StringField $local_host;
    public PortField $local_port;
    public StringField $local_id;
    public StringField $remote_host;
    public PortField $remote_port;
    public StringField $remote_id;
    public StringField $initiator_spi;
    public StringField $responder_spi;
    public BooleanField $nat_remote;
    public BooleanField $nat_any;
    public StringField $encr_alg;
    public IntegerField $encr_keysize;
    public StringField $integ_alg;
    public StringField $prf_alg;
    public StringField $dh_group;
    public IntegerField $established;
    public IntegerField $rekey_time;
    public NestedModelField $child_sas;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_ipsec_sa_statuses';
        $this->verbose_name = 'IPsec SA Status';
        $this->many = true;

        # Set model fields
        $this->con_id = new StringField(
            default: '',
            read_only: true,
            internal_name: 'con-id',
            verbose_name: 'Con ID',
            help_text: 'The connection ID of the tunnel.',
        );
        $this->uniqueid = new IntegerField(default: 0, read_only: true, verbose_name: 'Unique ID', help_text: 'The unique ID of the tunnel.');
        $this->version = new IntegerField(
            default: 0,
            read_only: true,
            verbose_name: 'Version',
            help_text: 'The IKE version used by the tunnel.',
        );
        $this->state = new StringField(default: '', read_only: true, verbose_name: 'State', help_text: 'The current state of the tunnel.');
        $this->local_host = new StringField(
            default: '',
            read_only: true,
            internal_name: 'local-host',
            verbose_name: 'Local Host',
            help_text: 'The local host for the tunnel.',
        );
        $this->local_port = new PortField(
            default: '',
            read_only: true,
            internal_name: 'local-port',
            verbose_name: 'Local Port',
            help_text: 'The local port for the tunnel.',
        );
        $this->local_id = new StringField(
            default: '',
            read_only: true,
            internal_name: 'local-id',
            verbose_name: 'Local ID',
            help_text: 'The local ID for the tunnel.',
        );
        $this->remote_host = new StringField(
            default: '',
            read_only: true,
            internal_name: 'remote-host',
            verbose_name: 'Remote Host',
            help_text: 'The remote host for the tunnel.',
        );
        $this->remote_port = new PortField(
            default: '',
            read_only: true,
            internal_name: 'remote-port',
            verbose_name: 'Remote Port',
            help_text: 'The remote port for the tunnel.',
        );
        $this->remote_id = new StringField(
            default: '',
            read_only: true,
            internal_name: 'remote-id',
            verbose_name: 'Remote ID',
            help_text: 'The remote ID for the tunnel.',
        );
        $this->initiator_spi = new StringField(
            default: '',
            read_only: true,
            internal_name: 'initiator-spi',
            verbose_name: 'Initiator SPI',
            help_text: 'The initiator SPI for the tunnel.',
        );
        $this->responder_spi = new StringField(
            default: '',
            read_only: true,
            internal_name: 'responder-spi',
            verbose_name: 'Responder SPI',
            help_text: 'The responder SPI for the tunnel.',
        );
        $this->nat_remote = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: 'yes',
            internal_name: 'nat-remote',
            verbose_name: 'NAT Remote',
            help_text: 'Whether the remote host is behind NAT.',
        );
        $this->nat_any = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: 'yes',
            internal_name: 'nat-any',
            verbose_name: 'NAT Any',
            help_text: 'Whether the remote host is behind any NAT.',
        );
        $this->encr_alg = new StringField(
            default: '',
            read_only: true,
            internal_name: 'encr-alg',
            verbose_name: 'Encryption Algorithm',
            help_text: 'The encryption algorithm used by the tunnel.',
        );
        $this->encr_keysize = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'encr-keysize',
            verbose_name: 'Encryption Key Size',
            help_text: 'The encryption key size used by the tunnel.',
        );
        $this->integ_alg = new StringField(
            default: '',
            read_only: true,
            internal_name: 'integ-alg',
            verbose_name: 'Integrity Algorithm',
            help_text: 'The integrity algorithm used by the tunnel.',
        );
        $this->prf_alg = new StringField(
            default: '',
            read_only: true,
            internal_name: 'prf-alg',
            verbose_name: 'PRF Algorithm',
            help_text: 'The pseudo-random function algorithm used by the tunnel.',
        );
        $this->dh_group = new StringField(
            default: '',
            read_only: true,
            internal_name: 'dh-group',
            verbose_name: 'DH Group',
            help_text: 'The Diffie-Hellman group used by the tunnel.',
        );
        $this->established = new IntegerField(
            default: 0,
            read_only: true,
            verbose_name: 'Established',
            help_text: 'The amount of time the tunnel has been established.',
        );
        $this->rekey_time = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'rekey-time',
            verbose_name: 'Rekey Time',
            help_text: 'The amount of time until the tunnel rekeys.',
        );
        $this->child_sas = new NestedModelField(
            model_class: 'IPsecChildSAStatus',
            read_only: true,
            internal_name: 'child-sas',
            verbose_name: 'Child Sas',
            help_text: 'The child SAs of the tunnel.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the status of IPsec SAs. This is the internal callable that is used to obtain the internal data for
     * this model.
     * @return array The status of IPsec tunnels where each item is an associative array containing the status of an
     * IPsec tunnel on the system.
     */
    public static function get_ipsec_sa_statuses(): array {
        # Variables
        $ipsec_statuses = [];
        $ipsec_statuses_raw = ipsec_list_sa();

        # Loop through each tunnel status and reform the data
        foreach ($ipsec_statuses_raw as $id => $ipsec_status) {
            # Variables
            $ipsec_statuses[] = [];

            # Loop through each status item
            foreach ($ipsec_status as $key => $value) {
                # For the 'child-sas' key, we need to convert the associative array to a list of associative arrays
                if ($key === 'child-sas') {
                    $ipsec_statuses[$id][$key] = array_values($value);
                    continue;
                }

                # Otherwise just set the value
                $ipsec_statuses[$id][$key] = $value;
            }
        }

        return $ipsec_statuses;
    }
}
