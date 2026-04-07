<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Dispatchers\BINDApplyDispatcher;
use RESTAPI\Fields\Base64Field;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\NestedModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\HostnameValidator;
use RESTAPI\Validators\IPAddressValidator;

/**
 * Defines a model that interacts with BIND zones.
 */
class BINDZone extends Model {
    public BooleanField $disabled;
    public StringField $name;
    public StringField $description;
    public StringField $type;
    public ForeignModelField $view;
    public BooleanField $reversev4;
    public BooleanField $reversev6;
    public BooleanField $rpz;
    public Base64Field $custom;
    public BooleanField $dnssec;
    public BooleanField $backupkeys;
    public StringField $slaveip;
    public StringField $forwarders;
    public IntegerField $ttl;
    public StringField $baseip;
    public StringField $nameserver;
    public StringField $mail;
    public IntegerField $serial;
    public StringField $refresh;
    public StringField $retry;
    public StringField $expire;
    public StringField $minimum;
    public BooleanField $enable_updatepolicy;
    public StringField $updatepolicy;
    public ForeignModelField $allowupdate;
    public ForeignModelField $allowtransfer;
    public ForeignModelField $allowquery;
    public BooleanField $regdhcpstatic;
    public Base64Field $customzonerecords;
    public NestedModelField $records;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/bindzone/config';
        $this->packages = ['pfSense-pkg-bind'];
        $this->package_includes = ['bind.inc'];
        $this->many = true;
        $this->always_apply = true;
        $this->verbose_name = 'BIND Zone';

        # Set model fields
        $this->disabled = new BooleanField(default: false, indicates_true: 'on', verbose_name: 'Disabled', help_text: 'Disable this BIND zone.');
        $this->name = new StringField(required: true, unique: true, verbose_name: 'Name', help_text: 'The name of this BIND zone.');
        $this->description = new StringField(
            default: '',
            allow_empty: true,
            verbose_name: 'Description',
            help_text: 'A description for this BIND zone.',
        );
        $this->type = new StringField(
            default: 'master',
            choices: ['master', 'slave', 'forward', 'redirect'],
            verbose_name: 'Type',
            help_text: 'The type of this BIND zone.',
        );
        $this->view = new ForeignModelField(
            model_name: 'BINDView',
            model_field: 'name',
            default: [],
            allow_empty: true,
            many: true,
            verbose_name: 'View',
            help_text: 'The views this BIND zone belongs to.',
        );
        $this->reversev4 = new BooleanField(
            default: false,
            indicates_true: 'on',
            internal_name: 'reverso',
            conditions: ['type' => ['master', 'slave']],
            verbose_name: 'Reverse IPv4',
            help_text: 'Enable reverse DNS for this BIND zone.',
        );
        $this->reversev6 = new BooleanField(
            default: false,
            indicates_true: 'on',
            internal_name: 'reversv6o',
            conditions: ['type' => ['master', 'slave']],
            verbose_name: 'Reverse IPv6',
            help_text: 'Enable reverse IPv6 DNS for this BIND zone.',
        );
        $this->rpz = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['type' => ['master', 'slave']],
            verbose_name: 'RPZ',
            help_text: 'Enable this zone as part of a response policy.',
        );
        $this->custom = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'Custom',
            help_text: 'Custom BIND options for this BIND zone.',
        );
        $this->dnssec = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['type' => ['master', 'slave']],
            verbose_name: 'DNSSEC',
            help_text: 'Enable DNSSEC for this BIND zone.',
        );
        $this->backupkeys = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['dnssec' => true],
            verbose_name: 'Backup Keys',
            help_text: 'Enable backing up DNSSEC keys in the XML configuration for this BIND zone.',
        );
        $this->slaveip = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['type' => 'slave'],
            validators: [new IPAddressValidator()],
            verbose_name: 'Secondary IP',
            help_text: 'The IP address of the slave server for this BIND zone.',
        );
        $this->forwarders = new StringField(
            required: true,
            many: true,
            delimiter: ';',
            conditions: ['type' => 'forward'],
            validators: [new IPAddressValidator()],
            verbose_name: 'Forwarders',
            help_text: 'The forwarders for this BIND zone.',
        );
        $this->ttl = new IntegerField(
            default: null,
            allow_null: true,
            internal_name: 'tll',
            conditions: ['type' => 'master'],
            verbose_name: 'TTL',
            help_text: 'The default TTL interval (in seconds) for records within this BIND zone without a specific TTL.',
        );
        $this->baseip = new StringField(
            required: true,
            internal_name: 'ipns',
            conditions: ['type' => 'master'],
            validators: [new IPAddressValidator()],
            verbose_name: 'Base IP',
            help_text: 'The IP address of the base domain for this zone. This sets an A record for the base domain.',
        );
        $this->nameserver = new StringField(
            required: true,
            conditions: ['type' => ['master', 'redirect']],
            validators: [new IPAddressValidator(allow_fqdn: true)],
            verbose_name: 'Name Server',
            help_text: 'The SOA nameserver for this zone.',
        );
        $this->mail = new StringField(
            required: true,
            conditions: ['type' => ['master', 'redirect']],
            validators: [new HostnameValidator(allow_fqdn: true)],
            verbose_name: 'Mail',
            help_text: 'The SOA email address (RNAME) for this zone. This must be in an FQDN format.',
        );
        $this->serial = new IntegerField(
            required: true,
            unique: true,
            conditions: ['type' => ['master', 'redirect']],
            verbose_name: 'Serial',
            help_text: 'The SOA serial number for this zone.',
        );
        $this->refresh = new StringField(
            default: null,
            allow_null: true,
            conditions: ['type' => ['master', 'redirect']],
            verbose_name: 'Refresh',
            help_text: 'The SOA refresh interval for this zone. TTL-style time-unit suffixes are ' .
                'supported (e.g. 1h, 1d, 1w), otherwise time in seconds is assumed.',
        );
        $this->retry = new StringField(
            default: null,
            allow_null: true,
            conditions: ['type' => ['master', 'redirect']],
            verbose_name: 'Retry',
            help_text: 'The SOA retry interval for this zone. TTL-style time-unit suffixes are ' .
                'supported (e.g. 1h, 1d, 1w), otherwise time in seconds is assumed.',
        );
        $this->expire = new StringField(
            default: null,
            allow_null: true,
            conditions: ['type' => ['master', 'redirect']],
            verbose_name: 'Expire',
            help_text: 'The SOA expiry interval for this zone. TTL-style time-unit suffixes are ' .
                'supported (e.g. 1h, 1d, 1w), otherwise time in seconds is assumed.',
        );
        $this->minimum = new StringField(
            default: null,
            allow_null: true,
            conditions: ['type' => ['master', 'redirect']],
            verbose_name: 'Minimum',
            help_text: 'The SOA minimum TTL interval (in seconds) for this zone. This is also referred to as the ' .
                'negative TTL. TTL-style time-unit suffixes are supported (e.g. 1h, 1d, 1w), otherwise time in ' .
                'seconds is assumed.',
        );
        $this->enable_updatepolicy = new BooleanField(
            default: false,
            indicates_true: 'on',
            conditions: ['type' => 'master'],
            verbose_name: 'Enable Update Policy',
            help_text: 'Enable a specific dynamic update policy for this BIND zone.',
        );
        $this->updatepolicy = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['type' => 'master', 'enable_updatepolicy' => true],
            verbose_name: 'Update Policy',
            help_text: 'The update policy for this BIND zone.',
        );
        $this->allowupdate = new ForeignModelField(
            model_name: 'BINDAccessList',
            model_field: 'name',
            default: [],
            allow_empty: true,
            many: true,
            conditions: ['type' => 'master', 'enable_updatepolicy' => false],
            verbose_name: 'Allow Update',
            help_text: "The access lists that are allowed to submit dynamic updates for 'master' " .
                'zones (e.g. dynamic DNS).',
        );
        $this->allowtransfer = new ForeignModelField(
            model_name: 'BINDAccessList',
            model_field: 'name',
            default: [],
            allow_empty: true,
            many: true,
            conditions: ['type' => 'master'],
            verbose_name: 'Allow Transfer',
            help_text: 'The access lists that are allowed to transfer this BIND zone.',
        );
        $this->allowquery = new ForeignModelField(
            model_name: 'BINDAccessList',
            model_field: 'name',
            default: [],
            allow_empty: true,
            many: true,
            verbose_name: 'Allow Query',
            help_text: 'The access lists that are allowed to query this BIND zone.',
        );
        $this->regdhcpstatic = new BooleanField(
            default: false,
            indicates_true: 'on',
            verbose_name: 'Register DHCP Static Mappings',
            help_text: 'Register DHCP static mappings as records in this BIND zone.',
        );
        $this->customzonerecords = new Base64Field(
            default: '',
            allow_empty: true,
            verbose_name: 'Custom Zone Records',
            help_text: 'Custom records for this BIND zone.',
        );
        $this->records = new NestedModelField(
            model_class: 'BINDZoneRecord',
            default: [],
            allow_empty: true,
            verbose_name: 'Records',
            help_text: 'The records for this BIND zone.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Applies changes to the BIND zones.
     */
    public function apply(): void {
        (new BINDApplyDispatcher(async: $this->async))->spawn_process();
    }
}
