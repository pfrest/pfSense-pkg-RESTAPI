<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\DHCPServer;
use RESTAPI\Models\InterfaceVLAN;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Models\VirtualIP;

class APIModelsNetworkInterfaceTestCase extends TestCase {
    /**
     * Checks to ensure the `from_internal_descr()` correctly populates empty interface
     * `descr` field values.
     */
    public function test_from_internal_descr() {
        # Create a NetworkInterface model to test with
        $test_model = new NetworkInterface();
        $test_model->get_next_id();

        # Ensure empty internal `descr` values are populate as the ID in all caps
        $this->assert_equals($test_model->from_internal_descr(null), strtoupper($test_model->id));

        # Ensure non-empty internal `descr` values retain their original value
        $this->assert_equals($test_model->from_internal_descr('test'), 'test');
    }

    /**
     * Checks that the `descr' field value must be 22 characters or less when the `if` field is set
     * to an OpenVPN interface
     */
    public function test_descr_max_length_with_openvpn_if() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_DESCR_OPENVPN_MAX_LENGTH_EXCEEDED',
            code: 400,
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'ovpn1';

                # Run the `validate_descr()` method to trigger the thrown response
                $test_model->validate_descr('this descr is too long for an openvpn interface');
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'ovpn1';

                # Ensure the validation succeeds when value is an acceptable length
                $test_model->validate_descr('lengthisok');
            },
        );
    }

    /**
     * Checks that the `descr' field value must be 22 characters or less when the `if` field is set
     * to an IPsec interface
     */
    public function test_descr_max_length_with_ipsec_if() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_DESCR_IPSEC_MAX_LENGTH_EXCEEDED',
            code: 400,
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'ipsec1';

                # Run the `validate_descr()` method to trigger the thrown response
                $test_model->validate_descr('this descr is too long for an ipsec interface');
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'ipsec1';

                # Ensure the validation succeeds when value is an acceptable length
                $test_model->validate_descr('lengthisok');
            },
        );
    }

    /**
     * Checks that the `descr' field value must be 22 characters or less when the `if` field is set
     * to a GIF interface
     */
    public function test_descr_max_length_with_gif_if() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_DESCR_GIF_MAX_LENGTH_EXCEEDED',
            code: 400,
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'gif1';

                # Run the `validate_descr()` method to trigger the thrown response
                $test_model->validate_descr('this descr is too long for a gif interface');
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'gif1';

                # Ensure the validation succeeds when value is an acceptable length
                $test_model->validate_descr('lengthisok');
            },
        );
    }

    /**
     * Checks that the `descr' field value must be 22 characters or less when the `if` field is set
     * to a GRE interface
     */
    public function test_descr_max_length_with_gre_if() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_DESCR_GRE_MAX_LENGTH_EXCEEDED',
            code: 400,
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'gre1';

                # Run the `validate_descr()` method to trigger the thrown response
                $test_model->validate_descr('this descr is too long for a gre interface');
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                # Create a NetworkInterface model to test with
                $test_model = new NetworkInterface();
                $test_model->if->value = 'gre1';

                # Ensure the validation succeeds when value is an acceptable length
                $test_model->validate_descr('length is ok');
            },
        );
    }

    /**
     * Checks that a VLAN interface cannot set its MTU higher than its parent interface's MTU.
     */
    public function test_vlan_interface_cannot_have_higher_mtu_than_parent() {
        # Create a new interface VLAN
        $vlan = new InterfaceVLAN(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'tag' => 2,
            ],
        );
        $vlan->create();

        # Configure the MTU on the VLANs parent interface
        $parent_if = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'enable' => true,
                'descr' => 'test_parent',
                'mtu' => 1450,
                'typev4' => 'none',
                'typev6' => 'none',
            ],
            async: false,
        );
        $parent_if->create(apply: true);

        # Ensure a validation error is thrown when trying to validate a VLAN interface with a higher MTU than its parent
        $this->assert_throws_response(
            response_id: 'INTERFACE_WITH_MTU_HIGHER_THAN_PARENT_MTU',
            code: 400,
            callable: function () {
                # Try to configure the VLAN interface with an MTU higher than the parent's
                $vlan_if = new NetworkInterface(
                    data: [
                        'if' => "{$this->env['PFREST_OPT1_IF']}.2",
                        'descr' => 'test_vlan_if',
                        'mtu' => 1500,
                        'typev4' => 'none',
                        'typev6' => 'none',
                    ],
                    async: false,
                );
                $vlan_if->validate();
            },
        );

        # Delete model objects used for testing
        $parent_if->delete();
        $vlan->delete();
    }

    /**
     * Checks that the `validate_media()` method properly validates available interface media types.
     */
    public function test_validate_media() {
        # Ensure an error is thrown if media is not a supported value
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_WITH_UNSUPPORTED_MEDIA_TYPE',
            code: 400,
            callable: function () {
                (new NetworkInterface())->validate_media('not a valid media type');
            },
        );

        # Ensure valid media types are accepted
        $this->assert_does_not_throw(
            callable: function () {
                (new NetworkInterface())->validate_media('autoselect');
            },
        );
    }

    /**
     * Checks that the `validate_mediaopt()` method properly validates available interface media options.
     */
    public function test_validate_mediaopt() {
        # Ensure an error is thrown if mediaopt is not a supported value
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_WITH_UNSUPPORTED_MEDIA_OPTION',
            code: 400,
            callable: function () {
                $if = new NetworkInterface();
                $if->media->value = '1000baseT';
                $if->validate_mediaopt('not a valid media option');
            },
        );

        # Ensure valid media options are accepted
        $this->assert_does_not_throw(
            callable: function () {
                $if = new NetworkInterface();
                $if->media->value = '1000baseT';
                $if->validate_mediaopt('full-duplex');
            },
        );
    }

    /**
     * Checks that the `validate_blockbogons()` correctly determines if bogons can be blocked on the current system.
     */
    public function test_validate_blockbogons() {
        # Ensure no error is thrown when trying to block bogons with a sufficient table entries size limit
        $this->assert_does_not_throw(
            callable: function () {
                (new NetworkInterface())->validate_blockbogons(true);
            },
        );

        # Ensure an error is thrown when trying to block bogons with an insufficient table entries size limit
        $original_maximumtableentries = Model::get_config(path: 'system/maximumtableentries');
        Model::set_config(path: 'system/maximumtableentries', value: g_get('minimumtableentries_bogonsv6') - 1);
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_BLOCKBOGONS_ENABLED_WITH_INSUFFICIENT_TABLE_ENTRY_LIMIT',
            code: 400,
            callable: function () {
                (new NetworkInterface())->validate_blockbogons(true);
            },
        );

        # Ensure no error is thrown with an insufficient table entries size limit, but IPv6 disabled
        Model::del_config('system/ipv6allow');
        $this->assert_does_not_throw(
            callable: function () {
                (new NetworkInterface())->validate_blockbogons(true);
            },
        );

        # Reset the original maximumtableentries value
        Model::set_config(path: 'system/maximumtableentries', value: $original_maximumtableentries);
        Model::set_config(path: 'system/ipv6allow', value: '');
    }

    /**
     * Checks that the `from_internal_typev4()` method correctly determines the IPv4 type of the configured interface.
     * `typev4` is not a stored field so this method is responsible for populating the field dynamically.
     */
    public function test_from_internal_typev4() {
        # Create a model to test with
        $test_model = new NetworkInterface();

        # Ensure the value is retained if it is not `null`. This emulates the behavior when a client sets the
        # `typev4` field via API call.
        $this->assert_equals($test_model->from_internal_typev4('test'), 'test');

        # Ensure the value is automatically populated if it is null. This emulates the behavior when the client has not
        # set a `typev4` in their call but the value can be determined by the `ipaddr` field.
        $test_model->ipaddr->value = '1.2.3.4';
        $this->assert_equals($test_model->from_internal_typev4(null), 'static');
        $test_model->ipaddr->value = 'dhcp';
        $this->assert_equals($test_model->from_internal_typev4(null), 'dhcp');
        $test_model->ipaddr->value = null;
        $this->assert_equals($test_model->from_internal_typev4(null), 'none');
    }

    /**
     * Checks that the interface `typev4` cannot be changed from `static` if the DHCP server is enabled on the
     * interface.
     */
    public function test_no_non_static_typev4_when_dhcpd_enabled() {
        $this->assert_throws_response(
            'NETWORK_INTERFACE_CANNOT_CHANGE_TYPEV4_WHILE_DHCP_SERVER_ENABLED',
            code: 400,
            callable: function () {
                # Ensure the DHCP server is enabled on the lan interface, then try to change the interface to DHCP
                Model::set_config(path: 'dhcpd/lan/enable', value: '');
                $test_model = new NetworkInterface(id: 'lan');
                $test_model->validate_typev4('dhcp');
            },
        );

        # Reset the dhcpd enable value
        Model::del_config(path: 'dhcpd/lan/enable');
    }

    /**
     * Checks that the interface `typev4` cannot be changed to `none` if it has virtual IPs.
     */
    public function test_no_none_typev4_when_vips_exist() {
        $this->assert_throws_response(
            'NETWORK_INTERFACE_NO_TYPEV4_NONE_WITH_EXISTING_VIRTUAL_IPS',
            code: 400,
            callable: function () {
                # Disable the DHCP server for this lan interface so it does not throw an error
                $dhcp_server = new DHCPServer(id: 'lan', async: false);
                $dhcp_server_original_enable = $dhcp_server->enable->value;
                $dhcp_server->enable->value = false;
                $dhcp_server->update();

                # Add a virtual IP for the LAN interface
                $test_vip = new VirtualIP(
                    data: [
                        'mode' => 'ipalias',
                        'interface' => 'lan',
                        'subnet' => '192.168.1.5',
                        'subnet_bits' => 24,
                    ],
                );
                $test_vip->create();

                # Ensure the `lan` interface's `typev4` value cannot change to `none` now.
                $test_model = new NetworkInterface(id: 'lan');
                $test_model->validate_typev4('none');

                # Remove the test vip and restore the DHCP server to it's original value
                $test_vip->delete();
                $dhcp_server->enable->value = $dhcp_server_original_enable;
                $dhcp_server->update();
            },
        );

        # Remove the virtual IP
        Model::del_config(path: 'virtualip/vip/0');
    }

    /**
     * Checks that the `validate_ipaddr()` method forces the `ipaddr` to match the `typev4` value when `typev4` is not
     * `static`
     */
    public function test_ipaddr_forced_value_for_non_static_typev4() {
        # Create a NetworkInterface to use for testing
        $test_model = new NetworkInterface();

        # Ensure all non `static` and non `none` typev4 values force `ipaddr` to match `typev4`
        # Note: the `ipaddr` field is only available when `typev4` is not `none` so we don't need to test it
        foreach ($test_model->typev4->choices as $typev4_choice) {
            if (!in_array($typev4_choice, ['static', 'none'])) {
                # Ensure the value returned by `validate_ipaddr` matches the `typev4` regardless of input
                $test_model->typev4->value = $typev4_choice;
                $this->assert_equals($test_model->validate_ipaddr('some other value'), $typev4_choice);
            }
        }
    }

    /**
     * Checks that the `validate_ipaddr()` method forces the `ipaddr` to be an IPv4 address if `typev4` is `static`.
     */
    public function test_ipaddr_must_be_ipv4_when_static() {
        $this->assert_throws_response(
            response_id: 'INTERFACE_IPADDR_NOT_IPV4_WHEN_STATIC',
            code: 400,
            callable: function () {
                # Create a NetworkInterface to use for testing
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';

                # Ensure an error is thrown if we try to set the `ipaddr` to anything but an IPv4
                $test_model->validate_ipaddr('dhcp');
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                # Create a NetworkInterface to use for testing
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';

                # Ensure no error is thrown when we set the `ipaddr` to an IPv4 address
                $test_model->validate_ipaddr('1.2.3.4');
            },
        );
    }

    /**
     * Checks that `validate_ipaddr()` ensures an interface's `ipaddr` and `subnet` cannot overlap with another
     * interface, virtual IP, etc.
     */
    public function test_ipaddr_and_subnet_cannot_overlap_other_objects() {
        $this->assert_throws_response(
            response_id: 'INTERFACE_IPADDR_ALREADY_IN_USE',
            code: 400,
            callable: function () {
                # Create a NetworkInterface to use for testing
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';
                $test_model->subnet->value = 24;

                # Ensure an error is thrown if we try to set `ipaddr` to a value that overlaps the default LAN subnet
                $test_model->validate_ipaddr('192.168.1.2');
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                # Create a NetworkInterface to use for testing
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';
                $test_model->subnet->value = 24;

                # Ensure an error is not thrown when the `ipaddr` is not in use anywhere else
                $test_model->validate_ipaddr('192.168.100.1');
            },
        );
    }

    /**
     * Checks that `validate_ipaddr()` prevents the `ipaddr` from being set as the network address.
     */
    public function test_ipaddr_cannot_be_network_address() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_IPADDR_CANNOT_USE_NETWORK_ADDRESS',
            code: 400,
            callable: function () {
                # Create a NetworkInterface to use for testing
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';
                $test_model->subnet->value = 24;

                # Ensure an error is thrown if we try to set `ipaddr` to a value that overlaps the default LAN subnet
                $test_model->validate_ipaddr('192.168.10.0');
            },
        );
    }

    /**
     * Checks that `validate_ipaddr()` prevents the `ipaddr` from being set as the broadcast address.
     */
    public function test_ipaddr_cannot_be_broadcast_address() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_IPADDR_CANNOT_USE_BROADCAST_ADDRESS',
            code: 400,
            callable: function () {
                # Create a NetworkInterface to use for testing
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';
                $test_model->subnet->value = 24;

                # Ensure an error is thrown if we try to set `ipaddr` to a value that overlaps the default LAN subnet
                $test_model->validate_ipaddr('192.168.10.255');
            },
        );
    }

    /**
     * Checks that the `validate_ipaddr()` method correctly prevents the `ipaddr` and `subnet` from being set to values
     * that conflict with an existing static route.
     */
    public function test_ipaddr_cannot_conflict_with_static_route() {
        # Add a static route we can use to create a conflict
        # TODO: replace this with a StaticRoute model call when developed
        $static_route = [
            'network' => '1.2.3.0/24',
            'gateway' => 'WAN_DHCP',
        ];
        Model::set_config(path: 'staticroutes/route/0', value: $static_route);

        # Ensure an error is thrown when trying to create an interface with the same network as the static route
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_IPADDR_CONFLICTS_WITH_STATIC_ROUTE',
            code: 400,
            callable: function () {
                # Create a new NetworkInterface object to test with
                $test_model = new NetworkInterface();
                $test_model->typev4->value = 'static';
                $test_model->subnet->value = 24;
                $test_model->validate_ipaddr('1.2.3.1');
            },
        );

        # Remote the static route
        Model::del_config(path: 'staticroutes/route/0');
    }

    /**
     * Checks that the `validate_subnet()` method prevents an interface's subnet from being reduced to a /31 or /32
     * while the DHCP server is enabled on the interface.
     */
    public function test_subnet_is_less_than_31_while_dhcp_server_enabled() {
        # Ensure the DHCP server is enabled on the `lan` interface
        Model::set_config(path: 'dhcpd/lan/enable', value: '');

        # Ensure an error is thrown when we try to reduce the `lan` interface's subnet to 31
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_SUBNET_MAXIMUM_EXCEEDED_WHILE_DHCP_SERVER_ENABLED',
            code: 400,
            callable: function () {
                # Create a new NetworkInterface object to test with
                $test_model = new NetworkInterface(id: 'lan');
                $test_model->validate_subnet(31);
            },
        );

        # Ensure an error is thrown when we try to reduce the `lan` interface's subnet to 32
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_SUBNET_MAXIMUM_EXCEEDED_WHILE_DHCP_SERVER_ENABLED',
            code: 400,
            callable: function () {
                # Create a new NetworkInterface object to test with
                $test_model = new NetworkInterface(id: 'lan');
                $test_model->validate_subnet(32);
            },
        );

        # Ensure no error is thrown when we reduce the `lan` interface's subnet to 30
        $this->assert_does_not_throw(
            callable: function () {
                # Create a new NetworkInterface object to test with
                $test_model = new NetworkInterface(id: 'lan');
                $test_model->validate_subnet(30);
            },
        );

        # Disable the DHCP server
        Model::del_config(path: 'dhcpd/lan/enable');
    }

    /**
     * Checks that the `from_internal_typev6()` method correctly determines the IPv6 type of the configured interface.
     * `typev6` is not a stored field so this method is responsible for populating the field dynamically.
     */
    public function test_from_internal_typev6() {
        # Create a model to test with
        $test_model = new NetworkInterface();

        # Ensure the value is retained if it is not `null`. This emulates the behavior when a client sets the
        # `typev4` field via API call.
        $this->assert_equals($test_model->from_internal_typev6('staticv6'), 'staticv6');

        # Ensure the value is automatically populated if it is null. This emulates the behavior when the client has not
        # set a `typev6` in their call but the value can be determined by the `ipaddrv6` field.
        $test_model->ipaddrv6->value = '1234::1';
        $this->assert_equals($test_model->from_internal_typev6(null), 'staticv6');
        $test_model->ipaddrv6->value = 'dhcp6';
        $this->assert_equals($test_model->from_internal_typev6(null), 'dhcp6');
        $test_model->ipaddrv6->value = 'slaac';
        $this->assert_equals($test_model->from_internal_typev6(null), 'slaac');
        $test_model->ipaddrv6->value = '6rd';
        $this->assert_equals($test_model->from_internal_typev6(null), '6rd');
        $test_model->ipaddrv6->value = '6to4';
        $this->assert_equals($test_model->from_internal_typev6(null), '6to4');
        $test_model->ipaddrv6->value = 'track6';
        $this->assert_equals($test_model->from_internal_typev6(null), 'track6');
        $test_model->ipaddrv6->value = null;
        $this->assert_equals($test_model->from_internal_typev6(null), 'none');
    }

    # TODO: need test to ensure typev6 cannot change from staticv6 or track6 when the DHCPv6 server is enabled

    # TODO: need test to ensure typev6 cannot change from staticv6 or track6 when DHCPv6 RA is enabled

    /**
     * Checks that the `validate_typev6()` method prevents more than one interface from using the `6to4` typev6
     */
    public function test_only_one_interface_with_6to4_typev6() {
        # Create a new 6to4 typev6 interface
        $if_6to4 = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'descr' => 'test_6to4',
                'typev4' => 'none',
                'typev6' => '6to4',
                'ipaddrv6' => '6to4',
            ],
        );
        $if_6to4->create();

        # Ensure an error is thrown if we try to create another 6to4 interface
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_TYPEV6_6TO4_ALREADY_IN_USE',
            code: 400,
            callable: function () {
                $test_model = new NetworkInterface();
                $test_model->validate_typev6('6to4');
            },
        );

        # Remove the created 6to4 interface
        $if_6to4->delete();
    }

    /**
     * Checks that the interface `typev6` cannot be changed to `none` if it has virtual IPs.
     */
    public function test_no_none_typev6_when_vips_exist() {
        # Create a static IPv6 interface to test with
        $test_model = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'descr' => 'testv6',
                'typev4' => 'none',
                'typev6' => 'staticv6',
                'ipaddrv6' => '1234::1',
                'subnetv6' => 64,
            ],
        );
        $test_model->create();

        $this->assert_throws_response(
            'NETWORK_INTERFACE_NO_TYPEV6_NONE_WITH_EXISTING_VIRTUAL_IPS',
            code: 400,
            callable: function () use ($test_model) {
                # Add a virtual IP for the newly created interface
                # TODO: replace this with a VirtualIP model call when the VirtualIP model is developed
                $test_vip = [
                    'mode' => 'ipalias',
                    'interface' => $test_model->id,
                    'subnet' => '1234::2',
                    'subnet_bits' => '64',
                    'type' => 'single',
                    'uniqid' => uniqid(),
                    'descr' => 'test',
                ];
                Model::set_config(path: 'virtualip/vip/0', value: $test_vip);

                # Ensure an error is thrown when we try to change the `typev6` to `none`
                $test_model->validate_typev6('none');
            },
        );

        # Remove the virtual IP and NetworkInterface object we added for this test
        Model::del_config(path: 'virtualip/vip/0');
        $test_model->delete();
    }

    /**
     * Checks that the `get_next_id()` method correctly obtains the next available pfSense interface ID.
     */
    public function test_get_next_id() {
        $test_model = new NetworkInterface();
        $this->assert_equals($test_model->get_next_id(), 'opt1');
    }

    /**
     * Checks that the `get_supported_media()` method correctly identifies an interface's supported speed and duplex
     * types and options.
     */
    public function test_get_supported_media() {
        # Create a model object to test with
        $test_model = new NetworkInterface();
        $test_model->if->value = $this->env['PFREST_WAN_IF'];

        # Run ifconfig to gather interface details
        $ifconfig_cmd = new Command(
            command: "/sbin/ifconfig -m {$test_model->if->value}",
            pipe: 'grep "media "',
            escape_pipe: false,
        );
        $ifconfig_output = explode(PHP_EOL, $ifconfig_cmd->output);

        # Loop through each line in the ifconfig output and ensure it is found in the `get_supported_media()` output
        foreach ($ifconfig_output as $media_line) {
            # Remove excess whitespace
            $media_line = trim($media_line);

            # Extract the `media` type and `mediaopt` option from the media line
            $media_line_items = explode(' ', $media_line);
            $media = $media_line_items[1];
            $mediaopt = count($media_line_items) > 2 ? $media_line_items[3] : '';

            # Ensure the media and mediaopt values are correctly found in get_supported_media()
            $this->assert_is_true(array_key_exists($media, $test_model->get_supported_media()));
            $this->assert_is_true(in_array($mediaopt, $test_model->get_supported_media()[$media]));
        }
    }

    /**
     * Checks that the `serialize_changes()` method correctly serializes pending interface changes and writes the
     * pending changes to /tmp/.interfaces.apply to be applied later
     */
    public function test_serialize_changes() {
        # Create a NetworkInterface object to test with
        $test_model = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'enable' => true,
                'descr' => 'testif',
                'typev4' => 'static',
                'ipaddr' => '1.2.3.4',
                'subnet' => 24,
                'typev6' => 'staticv6',
                'ipaddrv6' => '1234::1',
                'subnetv6' => 64,
            ],
            async: false,
        );
        $test_model->validate();

        # Apply the pending interfaces file and ensure it doesn't already exist afterward
        $test_model->apply_create();
        $this->assert_is_false(file_exists('/tmp/.interfaces.apply'));

        # Serialize the changes to this interface so it populates a new .interfaces.apply file
        $test_model->serialize_changes();

        # Ensure the file now exists
        $this->assert_is_true(file_exists('/tmp/.interfaces.apply'));

        # Ensure the file contents can be unserialized to the expected values
        $pending_ifs = unserialize(file_get_contents('/tmp/.interfaces.apply'));
        $this->assert_equals($pending_ifs, [
            $test_model->id => [
                'ifcfg' => $test_model->to_internal() + ['realif' => $test_model->if->value],
            ],
        ]);

        # Discard the pending interface changes
        unlink('/tmp/.interfaces.apply');
    }

    /**
     * Checks that NetworkInterface objects cannot be deleted if the ID is `wan`
     */
    public function test_cannot_delete_wan_interface() {
        $this->assert_throws_response(
            response_id: 'NETWORK_INTERFACE_WAN_CANNOT_BE_DELETED',
            code: 400,
            callable: function () {
                $wan_if = new NetworkInterface(id: 'wan');
                $wan_if->delete();
            },
        );
    }

    # TODO: Need test to ensure interface can't be deleted while its in an if group

    # TODO: Need test to ensure interface can't be dleted while its in an if bridge

    # TODO Need test to ensure interface can't be deleted while its in an if gre

    # TODO: Need test to ensure interface can't be deleted while its in an if gif

    # TODO: Need test to ensure interface can't be deleted while it has a traffic shaper queue

    /**
     * Checks that the `get_all_avail_interfaces()` method correctly gathers available interfaces.
     */
    public function test_get_all_avail_interfaces() {
        # Create a model to test with
        $test_model = new NetworkInterface();

        # Ensure the wan interface is found
        $this->assert_is_true(array_key_exists($this->env['PFREST_WAN_IF'], $test_model->get_all_avail_interfaces()));

        # Ensure the lan interface is found
        $this->assert_is_true(array_key_exists($this->env['PFREST_LAN_IF'], $test_model->get_all_avail_interfaces()));
    }

    /**
     * Checks that static NetworkInterfaces are accurately configured on the backend whenever they are created, updated,
     * and/or deleted.
     */
    public function test_static_interfaces_match_ifconfig() {
        # Create a new static NetworkInterface
        $static_if = new NetworkInterface(
            data: [
                'if' => $this->env['PFREST_OPT1_IF'],
                'descr' => 'teststatic',
                'enable' => true,
                'spoofmac' => '00:00:00:00:00:00',
                'mtu' => 1501,
                'typev4' => 'static',
                'ipaddr' => '192.168.100.1',
                'subnet' => 24,
                'typev6' => 'staticv6',
                'ipaddrv6' => '1234::1',
                'subnetv6' => 64,
            ],
            async: false,
        );
        $static_if->create(apply: true);

        # Run ifconfig to check the configured interface's values
        $ifconfig_output = (new Command("ifconfig {$this->env['PFREST_OPT1_IF']}"))->output;

        # Ensure the interface's mtu is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'mtu 1501');
        # Ensure the interface's description is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'description: teststatic');
        # Ensure the interface's spoofmac is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'ether 00:00:00:00:00:00');
        # Ensure the static IPv4 address is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'inet 192.168.100.1 netmask 0xffffff00 broadcast 192.168.100.255');
        # Ensure the static IPv6 address is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'inet6 1234::1 prefixlen 64');

        # Update the static interface configuration and ensure the updated values are correctly shown by ifconfig
        $static_if->from_representation([
            'descr' => 'teststaticupdated',
            'spoofmac' => '11:11:11:11:11:11',
            'mtu' => 1500,
            'typev4' => 'static',
            'ipaddr' => '192.168.200.1',
            'subnet' => 25,
            'typev6' => 'staticv6',
            'ipaddrv6' => '4321::1',
            'subnetv6' => 96,
        ]);
        $static_if->update(apply: true);

        # Run ifconfig to check the configured interface's new values
        $ifconfig_output = (new Command("ifconfig {$this->env['PFREST_OPT1_IF']}"))->output;

        # Ensure the interface's new mtu is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'mtu 1500');
        # Ensure the interface's new description is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'description: teststaticupdated');
        # Ensure the interface's new spoofmac is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'ether 11:11:11:11:11:11');
        # Ensure the new static IPv4 address is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'inet 192.168.200.1 netmask 0xffffff80 broadcast 192.168.200.127');
        # Ensure the new static IPv6 address is shown by ifconfig
        $this->assert_str_contains($ifconfig_output, 'inet6 4321::1 prefixlen 96');

        # Delete the static interface and ensure it is no longer present in ifconfig
        $static_if->delete();

        # Run ifconfig to check the configured interface's deleted values
        $ifconfig_output = (new Command("ifconfig {$this->env['PFREST_OPT1_IF']}"))->output;

        # Ensure the new static IPv4 address is not shown by ifconfig
        $this->assert_str_does_not_contain(
            $ifconfig_output,
            'inet 192.168.200.1 netmask 0xffffff80 broadcast 192.168.200.127',
        );
        # Ensure the new static IPv6 address isnot  shown by ifconfig
        $this->assert_str_does_not_contain($ifconfig_output, 'inet6 4321::1 prefixlen 96');
    }

    /**
     * Checks that the `is_ipv4_in_cidr()` method correctly determines if a given IP address is within an interface's
     * configured IPv4 CIDR
     */
    public function test_is_ipv4_in_cidr(): void {
        $if = new NetworkInterface(id: 'lan');
        $this->assert_is_true($if->is_ipv4_in_cidr('192.168.1.5'));
        $this->assert_is_false($if->is_ipv4_in_cidr('192.168.2.5'));
    }

    # TODO: Needs Tests to ensure other interface types are correctly configured on the backend

    /**
     * Checks that a NEW descr value violating filter-name rules is rejected
     * (FilterNameValidator now runs in validate_descr for changed values).
     */
    public function test_descr_new_value_rejects_reserved_name(): void {
        $this->assert_throws_response(
            response_id: 'FILTER_NAME_VALIDATOR_RESERVED_NAME_USED',
            code: 400,
            callable: function () {
                $test_model = new NetworkInterface();
                $test_model->if->value = 'igc0';
                $test_model->validate_descr('LAN');
            },
        );
    }

    /**
     * Checks that validate_descr accepts reserved legacy values when the
     * descr is UNCHANGED from the initial object (regression: pfSense
     * revalidates every field on PATCH; validating an unchanged value
     * locked whole interfaces whose descr predates the rules).
     */
    public function test_descr_unchanged_legacy_value_not_revalidated(): void {
        $this->assert_does_not_throw(
            callable: function () {
                $test_model = new NetworkInterface();
                $test_model->if->value = 'igc0';

                # Simulate a loaded object whose initial descr is a legacy
                # reserved token (e.g. created before the validator existed)
                $test_model->initial_object = $test_model->copy();
                $test_model->initial_object->descr->value = 'LAN';
                $test_model->descr->value = 'LAN';

                # Unchanged descr -> no filter-name validation
                $test_model->validate_descr('LAN');
            },
        );
    }

    /**
     * Checks that dhcp6_ia_pd_len accepts the GUI choices ('none' and the
     * 16..0 IA-PD lengths) and rejects out-of-set values.
     */
    public function test_dhcp6_ia_pd_len_choices(): void {
        $this->assert_does_not_throw(
            callable: function () {
                $test_model = new NetworkInterface();
                $test_model->typev6->value = 'dhcp6';
                $test_model->dhcp6_ia_pd_len->value = '0';
                $test_model->dhcp6_ia_pd_len->validate();
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                $test_model = new NetworkInterface();
                $test_model->typev6->value = 'dhcp6';
                $test_model->dhcp6_ia_pd_len->value = 'none';
                $test_model->dhcp6_ia_pd_len->validate();
            },
        );

        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                $test_model = new NetworkInterface();
                $test_model->typev6->value = 'dhcp6';
                $test_model->dhcp6_ia_pd_len->value = '999';
                $test_model->dhcp6_ia_pd_len->validate();
            },
        );
    }
}
