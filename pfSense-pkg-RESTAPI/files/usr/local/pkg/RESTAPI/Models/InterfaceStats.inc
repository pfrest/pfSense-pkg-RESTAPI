<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model that represents interface statistics and detailed interface information.
 */
class InterfaceStats extends Model {
    public StringField $name;
    public StringField $descr;
    public StringField $hwif;
    public StringField $macaddr;
    public StringField $mtu;
    public BooleanField $enable;
    public StringField $status;
    public StringField $ipaddr;
    public StringField $subnet;
    public StringField $linklocal;
    public StringField $ipaddrv6;
    public StringField $subnetv6;
    public IntegerField $inerrs;
    public IntegerField $outerrs;
    public IntegerField $collisions;
    public IntegerField $inbytes;
    public IntegerField $inbytespass;
    public IntegerField $outbytes;
    public IntegerField $outbytespass;
    public IntegerField $inpkts;
    public IntegerField $inpktspass;
    public IntegerField $outpkts;
    public IntegerField $outpktspass;
    public StringField $dhcplink;
    public StringField $media;
    public StringField $gateway;
    public StringField $gatewayv6;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_interface_stats';
        $this->many = true;

        # Set model fields
        $this->name = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Name',
            help_text: 'The name of the interface.',
        );
        $this->descr = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Description',
            help_text: 'The description of the interface.',
        );
        $this->hwif = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Hardware Interface',
            help_text: 'The hardware interface name of the interface.',
        );
        $this->macaddr = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'MAC Address',
            help_text: 'The MAC address of the interface.',
        );
        $this->mtu = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'MTU',
            help_text: 'The MTU of the interface.',
        );
        $this->enable = new BooleanField(default: false, verbose_name: 'Enable', help_text: 'Whether the interface is enabled.');
        $this->status = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Status',
            help_text: 'The status of the interface.',
        );
        $this->ipaddr = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'IP Address',
            help_text: 'The IPv4 address of the interface.',
        );
        $this->subnet = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Subnet',
            help_text: 'The IPv4 subnet of the interface.',
        );
        $this->linklocal = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Link-Local Address',
            help_text: 'The IPv6 link-local address of the interface.',
        );
        $this->ipaddrv6 = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'IPv6 Address',
            help_text: 'The IPv6 address of the interface.',
        );
        $this->subnetv6 = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'IPv6 Subnet',
            help_text: 'The IPv6 subnet of the interface.',
        );
        $this->inerrs = new IntegerField(default: 0, verbose_name: 'Inbound Errors', help_text: 'The number of inbound errors on the interface.');
        $this->outerrs = new IntegerField(default: 0, verbose_name: 'Outbound Errors', help_text: 'The number of outbound errors on the interface.');
        $this->collisions = new IntegerField(default: 0, verbose_name: 'Collisions', help_text: 'The number of collisions on the interface.');
        $this->inbytes = new IntegerField(default: 0, verbose_name: 'Inbound Bytes', help_text: 'The number of inbound bytes on the interface.');
        $this->inbytespass = new IntegerField(
            default: 0,
            verbose_name: 'Inbound Bytes (Pass)',
            help_text: 'The number of inbound bytes passed on the interface.',
        );
        $this->outbytes = new IntegerField(default: 0, verbose_name: 'Outbound Bytes', help_text: 'The number of outbound bytes on the interface.');
        $this->outbytespass = new IntegerField(
            default: 0,
            verbose_name: 'Outbound Bytes (Pass)',
            help_text: 'The number of outbound bytes passed on the interface.',
        );
        $this->inpkts = new IntegerField(default: 0, verbose_name: 'Inbound Packets', help_text: 'The number of inbound packets on the interface.');
        $this->inpktspass = new IntegerField(
            default: 0,
            verbose_name: 'Inbound Packets (Pass)',
            help_text: 'The number of inbound packets passed on the interface.',
        );
        $this->outpkts = new IntegerField(default: 0, verbose_name: 'Outbound Packets', help_text: 'The number of outbound packets on the interface.');
        $this->outpktspass = new IntegerField(
            default: 0,
            verbose_name: 'Outbound Packets (Pass)',
            help_text: 'The number of outbound packets passed on the interface.',
        );
        $this->dhcplink = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'DHCP Link',
            help_text: 'The DHCP link status of the interface.',
        );
        $this->media = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Media',
            help_text: 'The speed/duplex of the interface.',
        );
        $this->gateway = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Gateway',
            help_text: 'The IPv4 gateway of the interface.',
        );
        $this->gatewayv6 = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'IPv6 Gateway',
            help_text: 'The IPv6 gateway of the interface.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Retrieves interface statistics and detailed interface information. This method is used as the internal callable
     * for the InterfaceStats model.
     * @return array An array of interface statistics and detailed interface information.
     */
    protected function get_interface_stats(): array {
        $if_descrs = get_configured_interface_with_descr(true);
        $if_info = [];

        foreach ($if_descrs as $if_descr => $if_name) {
            $if = ['name' => $if_descr, 'descr' => $if_name];
            $if = $if + get_interface_info($if_descr);
            $if_info[] = $if;
        }
        return $if_info;
    }
}
