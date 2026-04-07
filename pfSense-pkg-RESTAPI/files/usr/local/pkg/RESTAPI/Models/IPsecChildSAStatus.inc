<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\PortField;
use RESTAPI\Fields\StringField;

/**
 * A Model that provides the status of IPsec child SAs.
 */
class IPsecChildSAStatus extends Model {
    public StringField $name;
    public IntegerField $uniqueid;
    public IntegerField $reqid;
    public StringField $state;
    public StringField $mode;
    public StringField $protocol;
    public BooleanField $encap;
    public StringField $spi_in;
    public StringField $spi_out;
    public StringField $encr_alg;
    public IntegerField $encr_keysize;
    public StringField $integ_alg;
    public StringField $dh_group;
    public IntegerField $bytes_in;
    public IntegerField $bytes_out;
    public IntegerField $packets_in;
    public IntegerField $packets_out;
    public IntegerField $use_in;
    public IntegerField $use_out;
    public IntegerField $rekey_time;
    public IntegerField $life_time;
    public IntegerField $install_time;
    public StringField $local_ts;
    public StringField $remote_ts;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_ipsec_sa_statuses';
        $this->parent_model_class = 'IPsecSAStatus';
        $this->verbose_name = 'IPsec Child SA Status';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            default: '',
            read_only: true,
            verbose_name: 'Name',
            help_text: 'The name of the connection ID for the child SA.',
        );
        $this->uniqueid = new IntegerField(default: 0, read_only: true, verbose_name: 'Unique ID', help_text: 'The unique ID of the child SA.');
        $this->reqid = new IntegerField(default: 0, read_only: true, verbose_name: 'Reqid', help_text: 'The request ID of the child SA.');
        $this->state = new StringField(default: '', read_only: true, verbose_name: 'State', help_text: 'The current state of the child SA.');
        $this->mode = new StringField(default: '', read_only: true, verbose_name: 'Mode', help_text: 'The mode of the child SA.');
        $this->protocol = new StringField(default: '', read_only: true, verbose_name: 'Protocol', help_text: 'The protocol of the child SA.');
        $this->encap = new BooleanField(
            default: false,
            read_only: true,
            indicates_true: 'yes',
            verbose_name: 'Encap',
            help_text: 'Whether encapsulation is used for the child SA.',
        );
        $this->spi_in = new StringField(
            default: '',
            read_only: true,
            internal_name: 'spi-in',
            verbose_name: 'SPI In',
            help_text: 'The incoming SPI of the child SA.',
        );
        $this->spi_out = new StringField(
            default: '',
            read_only: true,
            internal_name: 'spi-out',
            verbose_name: 'SPI Out',
            help_text: 'The outgoing SPI of the child SA.',
        );
        $this->encr_alg = new StringField(
            default: '',
            read_only: true,
            internal_name: 'encr-alg',
            verbose_name: 'Encryption Algorithm',
            help_text: 'The encryption algorithm used by the child SA.',
        );
        $this->encr_keysize = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'encr-keysize',
            verbose_name: 'Encryption Key Size',
            help_text: 'The encryption key size used by the child SA.',
        );
        $this->integ_alg = new StringField(
            default: '',
            read_only: true,
            internal_name: 'integ-alg',
            verbose_name: 'Integrity Algorithm',
            help_text: 'The integrity algorithm used by the child SA.',
        );
        $this->dh_group = new StringField(
            default: '',
            read_only: true,
            internal_name: 'dh-group',
            verbose_name: 'DH Group',
            help_text: 'The Diffie-Hellman group used by the child SA.',
        );
        $this->bytes_in = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'bytes-in',
            verbose_name: 'Bytes In',
            help_text: 'The number of bytes received by the child SA.',
        );
        $this->bytes_out = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'bytes-out',
            verbose_name: 'Bytes Out',
            help_text: 'The number of bytes sent by the child SA.',
        );
        $this->packets_in = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'packets-in',
            verbose_name: 'Packets In',
            help_text: 'The number of packets received by the child SA.',
        );
        $this->packets_out = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'packets-out',
            verbose_name: 'Packets Out',
            help_text: 'The number of packets sent by the child SA.',
        );
        $this->use_in = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'use-in',
            verbose_name: 'Use In',
            help_text: 'The number of times the child SA has been used to receive data.',
        );
        $this->use_out = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'use-out',
            verbose_name: 'Use Out',
            help_text: 'The number of times the child SA has been used to send data.',
        );
        $this->rekey_time = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'rekey-time',
            verbose_name: 'Rekey Time',
            help_text: 'The time at which the child SA will be rekeyed.',
        );
        $this->life_time = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'life-time',
            verbose_name: 'Lifetime',
            help_text: 'The time at which the child SA will expire.',
        );
        $this->install_time = new IntegerField(
            default: 0,
            read_only: true,
            internal_name: 'install-time',
            verbose_name: 'Install Time',
            help_text: 'The time at which the child SA was installed.',
        );
        $this->local_ts = new StringField(
            default: '',
            read_only: true,
            many: true,
            delimiter: null,
            internal_name: 'local-ts',
            verbose_name: 'Local Traffic Selectors',
            help_text: 'The local traffic selector of the child SA.',
        );
        $this->remote_ts = new StringField(
            default: '',
            read_only: true,
            many: true,
            delimiter: null,
            internal_name: 'remote-ts',
            verbose_name: 'Remote Traffic Selectors',
            help_text: 'The remote traffic selector of the child SA.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the status of IPsec child SAs. This is the internal callable that is used to obtain the internal data for
     * this model.
     * @return array The status of IPsec child SAs where each item is an associative array containing the status of an
     * IPsec child SA on the system.
     */
    public function get_ipsec_sa_statuses(): array {
        return IPsecSAStatus::get_ipsec_sa_statuses()[$this->parent_id]['child-sas'];
    }
}
