<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for interfaces that are available to be assigned on this system.
 */
class AvailableInterface extends Model {
    public StringField $if;
    public StringField $mac;
    public StringField $dmesg;
    public StringField $in_use_by;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_all_avail_interfaces';
        $this->many = true;

        # Set model fields
        $this->if = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Interface Name',
            help_text: 'The name of the interface.',
        );
        $this->mac = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'MAC',
            help_text: 'The MAC address of the interface.',
        );
        $this->dmesg = new StringField(
            default: '',
            allow_empty: true,
            allow_null: true,
            verbose_name: 'Dmesg',
            help_text: 'The description of the interface.',
        );
        $this->in_use_by = new StringField(
            default: null,
            allow_null: true,
            verbose_name: 'In Use By',
            help_text: 'The pfSense interface ID that is using this interface.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains all available interfaces on the system
     * @return array Returns all available interfaces on the system
     */
    public static function get_all_avail_interfaces(): array {
        // Local variables
        global $ipsec_descrs;
        $base_ifs = get_interface_list();

        # Include wireless clone interfaces
        if (
            config_get_path('wireless/clone') &&
            is_array(config_get_path('wireless/clone')) &&
            count(config_get_path('wireless/clone'))
        ) {
            foreach (config_get_path('wireless/clone', []) as $clone) {
                $base_ifs[$clone['cloneif']] = $clone;
                $base_ifs[$clone['cloneif']]['iswlclone'] = true;
            }
        }

        # Include VLAN interfaces
        if (
            config_get_path('vlans/vlan') &&
            is_array(config_get_path('vlans/vlan')) &&
            count(config_get_path('vlans/vlan'))
        ) {
            foreach (config_get_path('vlans/vlan', []) as $vlan) {
                $base_ifs[$vlan['vlanif']] = $vlan;
                $base_ifs[$vlan['vlanif']]['isvlan'] = true;
            }
        }

        # Include Bridge interfaces
        if (
            config_get_path('bridges/bridged') &&
            is_array(config_get_path('bridges/bridged')) &&
            count(config_get_path('bridges/bridged'))
        ) {
            foreach (config_get_path('bridges/bridged', []) as $bridge) {
                $base_ifs[$bridge['bridgeif']] = $bridge;
                $base_ifs[$bridge['bridgeif']]['isbridge'] = true;
            }
        }

        # Include GIF interfaces
        if (
            config_get_path('gifs/gif') &&
            is_array(config_get_path('gifs/gif')) &&
            count(config_get_path('gifs/gif'))
        ) {
            foreach (config_get_path('gifs/gif', []) as $gif) {
                $base_ifs[$gif['gifif']] = $gif;
                $base_ifs[$gif['gifif']]['isgif'] = true;
            }
        }

        # Include GRE interfaces
        if (
            config_get_path('gres/gre') &&
            is_array(config_get_path('gres/gre')) &&
            count(config_get_path('gres/gre'))
        ) {
            foreach (config_get_path('gres/gre', []) as $gre) {
                $base_ifs[$gre['greif']] = $gre;
                $base_ifs[$gre['greif']]['isgre'] = true;
            }
        }

        # Include LAGG interfaces
        if (
            config_get_path('laggs/lagg') &&
            is_array(config_get_path('laggs/lagg')) &&
            count(config_get_path('laggs/lagg'))
        ) {
            foreach (config_get_path('laggs/lagg', []) as $lagg) {
                $base_ifs[$lagg['laggif']] = $lagg;
                $base_ifs[$lagg['laggif']]['islagg'] = true;
                # LAGG members cannot be assigned
                $lagifs = explode(',', $lagg['members']);
                foreach ($lagifs as $lagif) {
                    if (isset($base_ifs[$lagif])) {
                        unset($base_ifs[$lagif]);
                    }
                }
            }
        }

        # Include QinQ interfaces
        if (
            config_get_path('qinqs/qinqentry') &&
            is_array(config_get_path('qinqs/qinqentry')) &&
            count(config_get_path('qinqs/qinqentry'))
        ) {
            foreach (config_get_path('qinqs/qinqentry', []) as $qinq) {
                $base_ifs["{$qinq['vlanif']}"]['descr'] = "VLAN {$qinq['tag']} on {$qinq['if']}";
                $base_ifs["{$qinq['vlanif']}"]['isqinq'] = true;
                # QinQ members
                $qinqifs = explode(' ', $qinq['members']);
                foreach ($qinqifs as $qinqif) {
                    $base_ifs["{$qinq['vlanif']}.{$qinqif}"][
                        'descr'
                    ] = "QinQ {$qinqif} on VLAN {$qinq['tag']} on {$qinq['if']}";
                    $base_ifs["{$qinq['vlanif']}.{$qinqif}"]['isqinq'] = true;
                }
            }
        }

        # Include PPP interfaces
        if (
            config_get_path('ppps/ppp') &&
            is_array(config_get_path('ppps/ppp')) &&
            count(config_get_path('ppps/ppp'))
        ) {
            foreach (config_get_path('ppps/ppp', []) as $pppid => $ppp) {
                $portname = $ppp['if'];
                $base_ifs[$portname] = $ppp;
                $base_ifs[$portname]['isppp'] = true;
                $ports_base = basename($ppp['ports']);
                if (isset($ppp['descr'])) {
                    $base_ifs[$portname]['descr'] = strtoupper($ppp['if']) . "({$ports_base}) - {$ppp['descr']}";
                } elseif (isset($ppp['username'])) {
                    $base_ifs[$portname]['descr'] = strtoupper($ppp['if']) . "({$ports_base}) - {$ppp['username']}";
                } else {
                    $base_ifs[$portname]['descr'] = strtoupper($ppp['if']) . "({$ports_base})";
                }
            }
        }

        # Include OpenVPN interfaces
        $ovpn_descrs = [];
        if (config_get_path('openvpn') && is_array(config_get_path('openvpn'))) {
            if (config_get_path('openvpn/openvpn-server') && is_array(config_get_path('openvpn/openvpn-server'))) {
                foreach (config_get_path('openvpn/openvpn-server', []) as $s) {
                    $portname = "ovpns{$s['vpnid']}";
                    $base_ifs[$portname] = $s;
                    $ovpn_descrs[$s['vpnid']] = $s['description'];
                }
            }
            if (config_get_path('openvpn/openvpn-client') && is_array(config_get_path('openvpn/openvpn-client'))) {
                foreach (config_get_path('openvpn/openvpn-client', []) as $c) {
                    $portname = "ovpnc{$c['vpnid']}";
                    $base_ifs[$portname] = $c;
                    $ovpn_descrs[$c['vpnid']] = $c['description'];
                }
            }
        }

        # Include IPsec interfaces
        $ipsec_descrs = interface_ipsec_vti_list_all();
        foreach ($ipsec_descrs as $ifname => $ifdescr) {
            $base_ifs[$ifname] = ['descr' => $ifdescr];
        }

        # Loop through our array and check if interface is in use
        foreach ($base_ifs as $pid => $conf) {
            # Add the physical interface ID to the configuration
            $base_ifs[$pid]['if'] = $pid;

            # Check if there is a configuration for this interface
            $pf_id = NetworkInterface::query(if: $pid);

            # Check if the pfSense interface ID was found
            if ($pf_id->exists()) {
                $base_ifs[$pid]['in_use_by'] = $pf_id->first()->id;
            }
        }

        return $base_ifs;
    }
}
