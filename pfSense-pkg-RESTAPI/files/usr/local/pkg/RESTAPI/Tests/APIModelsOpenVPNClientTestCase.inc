<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\Certificate;
use RESTAPI\Models\CertificateAuthority;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Models\OpenVPNClient;

class APIModelsOpenVPNClientTestCase extends TestCase {
    /**
     * Setup the test environment before starting tests.
     */
    public function setup(): void {
        # Create a CA we can test with
        $ca = new CertificateAuthority(
            descr: 'test_ca',
            crt: file_get_contents('/usr/local/pkg/RESTAPI/Tests/assets/test_x509_certificate.crt'),
            prv: file_get_contents('/usr/local/pkg/RESTAPI/Tests/assets/test_x509_rsa.key'),
        );
        $ca->create();
        $this->ca_refid = $ca->refid->value;

        # Create a new certificate we can test with
        $cert = new Certificate(
            descr: 'test_cert',
            crt: file_get_contents('/usr/local/pkg/RESTAPI/Tests/assets/test_x509_certificate.crt'),
            prv: file_get_contents('/usr/local/pkg/RESTAPI/Tests/assets/test_x509_rsa.key'),
        );
        $cert->create();
        $this->cert_refid = $cert->refid->value;

        # Obtain an OpenVPN key to test with
        $this->tls_key = file_get_contents('/usr/local/pkg/RESTAPI/Tests/assets/test_openvpn_tls.key');

        # Set the base data we can use to quickly create a OpenVPNServer model object
        $this->base_data = [
            'mode' => 'p2p_tls',
            'dev_mode' => 'tun',
            'protocol' => 'UDP4',
            'interface' => 'wan',
            'server_addr' => 'example.com',
            'server_port' => '1194',
            'tls' => $this->tls_key,
            'tls_type' => 'auth',
            'dh_length' => '2048',
            'data_ciphers' => ['AES-256-GCM'],
            'data_ciphers_fallback' => 'AES-256-GCM',
            'digest' => 'SHA256',
            'caref' => $this->ca_refid,
            'certref' => $this->cert_refid,
        ];
    }

    /**
     * Remove the CA and cert used for testing after tests complete.
     */
    public function teardown(): void {
        $ca = CertificateAuthority::query(refid: $this->ca_refid)->first();
        $ca->delete();
        $cert = Certificate::query(refid: $this->cert_refid)->first();
        $cert->delete();
    }

    /**
     * Checks that the sum of all `data_ciphers` chars must be less than 254
     */
    public function test_data_ciphers_total_char_limit(): void {
        $this->assert_throws_response(
            response_id: 'OPENVPN_CLIENT_DATA_CIPHERS_CONTAINS_TOO_MANY_CHARS',
            code: 400,
            callable: function () {
                $rand_string = substr(str_shuffle(str_repeat('abc123', 256)), 0, 256);
                $ovpns = new OpenVPNClient(
                    data_ciphers: ['sum', 'of', 'these', 'items', 'contains', 'too', 'many', 'chars', $rand_string],
                );
                $ovpns->validate_extra();
            },
        );
    }

    /**
     * Checks that the `from_internal_vpnif()` method correctly obtains the VPN interface name.
     */
    public function test_from_internal_vpnif(): void {
        $ovpns = new OpenVPNClient();
        $ovpns->vpnid->value = $ovpns->get_next_vpn_id();
        $this->assert_equals($ovpns->from_internal_vpnif(), $ovpns::INTERFACE_PREFIX . $ovpns->vpnid->value);
    }

    /**
     * Checks that a OpenVPNClient object cannot be disabled or deleted while it's in use.
     */
    public function test_cannot_disable_or_delete_while_in_use(): void {
        # Create a OpenVPNClient to test with
        $ovpnc = new OpenVPNClient(data: $this->base_data);
        $ovpnc->create();

        # Create a NetworkInterface that uses this OpenVPN client interface
        $if = new NetworkInterface(
            if: 'ovpnc' . $ovpnc->vpnid->value,
            descr: 'OVPNTEST',
            typev4: 'none',
            typev6: 'none',
        );
        $if->create();

        # Ensure an error is thrown if we try to disable or delete the OpenVPNClient while its assigned to an interface
        $this->assert_throws_response(
            response_id: 'OPENVPN_CLIENT_CANNOT_BE_DISABLED_WHILE_IN_USE',
            code: 409,
            callable: function () use ($ovpnc) {
                $ovpnc->disable->value = true;
                $ovpnc->validate();
            },
        );
        $this->assert_throws_response(
            response_id: 'OPENVPN_CLIENT_CANNOT_BE_DELETED_WHILE_IN_USE',
            code: 409,
            callable: function () use ($ovpnc) {
                $ovpnc->delete();
            },
        );

        # Delete the interface assignment
        $if->delete();

        # Ensure the OpenVPNClient can be disabled and deleted while not assigned
        $this->assert_does_not_throw(
            callable: function () use ($ovpnc) {
                $ovpnc->disable->value = true;
                $ovpnc->validate();
                $ovpnc->delete();
            },
        );
    }

    /**
     * Checks that an IPv4 OpenVPNClient cannot be created on an interface without a IPv4 address.
     */
    public function test_ipv4_client_must_use_ipv4_interface(): void {
        # Create a new interface without an IPv4 address
        $if = new NetworkInterface(if: $this->env['PFREST_OPT1_IF'], descr: 'OVPNTEST', typev4: 'none', typev6: 'none');
        $if->create();

        # Ensure an IPv4 only OpenVPNClient cannot be created on the interface without an IPv4 address
        $this->assert_throws_response(
            response_id: 'OPENVPN_CLIENT_INTERFACE_HAS_NO_IPV4_ASSIGNMENT',
            code: 400,
            callable: function () use ($if) {
                $ovpns = new OpenVPNClient(data: $this->base_data, protocol: 'UDP4', interface: $if->id);
                $ovpns->validate();
            },
        );

        # Delete the interface
        $if->delete();
    }

    /**
     * Checks that an IPv6 OpenVPNClient cannot be created on an interface without a IPv6 address.
     */
    public function test_ipv6_client_must_use_ipv6_interface(): void {
        # Create a new interface without an IPv6 address
        $if = new NetworkInterface(if: $this->env['PFREST_OPT1_IF'], descr: 'OVPNTEST', typev4: 'none', typev6: 'none');
        $if->create();

        # Ensure an IPv4 only OpenVPNClient cannot be created on the interface without an IPv4 address
        $this->assert_throws_response(
            response_id: 'OPENVPN_CLIENT_INTERFACE_HAS_NO_IPV6_ASSIGNMENT',
            code: 400,
            callable: function () use ($if) {
                $ovpns = new OpenVPNClient(data: $this->base_data, protocol: 'UDP6', interface: $if->id);
                $ovpns->validate();
            },
        );

        # Delete the interface
        $if->delete();
    }

    /**
     * Checks that the `tls` field must contain a valid OpenVPN TLS key
     */
    public function test_validate_tls(): void {
        $this->assert_throws_response(
            response_id: 'OPENVPN_CLIENT_TLS_INVALID_KEY',
            code: 400,
            callable: function () {
                $ovpns = new OpenVPNClient(data: $this->base_data);
                $ovpns->tls->value = 'not an openvpn tls key!';
                $ovpns->validate();
            },
        );
    }

    /**
     * Checks that the `tunnel_network` and `tunnel_networkv6` values are converted to network
     * address CIDRs during validation
     */
    public function test_tunnel_networks_convert_to_network_address_cidr(): void {
        $ovpns = new OpenVPNClient(
            data: $this->base_data,
            tunnel_network: '10.1.2.3/24',
            tunnel_networkv6: '1234::5/64',
        );
        $ovpns->validate();
        $this->assert_equals($ovpns->tunnel_network->value, '10.1.2.0/24');
        $this->assert_equals($ovpns->tunnel_networkv6->value, '1234::/64');
    }

    /**
     * Checks that the get_next_vpn_id() method correctly obtains the next available OpenVPN ID
     */
    public function test_get_next_vpn_id(): void {
        $ovpns = new OpenVPNClient();
        $this->assert_equals($ovpns->get_next_vpn_id(), 1);
    }

    /**
     * Checks that the get_available_cipher_choices() correctly obtains the available cipher choices on this system.
     */
    public function test_get_available_cipher_choices(): void {
        $ovpns = new OpenVPNClient();
        $this->assert_is_not_empty($ovpns->get_available_cipher_choices());
    }

    /**
     * Checks that the get_available_digest_choices() correctly obtains the available `digest` choices on this system.
     */
    public function get_available_digest_choices(): void {
        $ovpns = new OpenVPNClient();
        $this->assert_is_not_empty($ovpns->get_available_digest_choices());
    }

    /**
     * Checks that OpenVPN clients are correctly created, updated and deleted.
     */
    public function test_crud(): void {
        # Create a OpenVPN client
        $ovpnc = new OpenVPNClient(data: $this->base_data);
        $ovpnc->create();

        # Ensure the OpenVPN is correctly configured on the backend
        $ovpn_config = file_get_contents("/var/etc/openvpn/client{$ovpnc->vpnid->value}/config.ovpn");
        $ifconfig = new Command("ifconfig ovpnc{$ovpnc->vpnid->value}");
        $this->assert_equals($ifconfig->result_code, 0);
        $this->assert_str_contains($ovpn_config, "dev-type {$ovpnc->dev_mode->value}");
        $lc_proto = strtolower($ovpnc->protocol->value);
        $this->assert_str_contains($ovpn_config, "proto $lc_proto");
        $if_ip = $ovpnc->interface->get_related_model()->get_current_ipv4();
        $this->assert_str_contains($ovpn_config, "local $if_ip");
        $this->assert_str_contains($ovpn_config, "lport {$ovpnc->local_port->value}");
        $this->assert_str_contains($ovpn_config, "auth {$ovpnc->digest->value}");
        $ciphers = implode(':', $ovpnc->data_ciphers->value);
        $this->assert_str_contains($ovpn_config, "data-ciphers $ciphers");
        $this->assert_str_contains($ovpn_config, "data-ciphers-fallback {$ovpnc->data_ciphers_fallback->value}");
        $this->assert_str_contains($ovpn_config, "allow-compression {$ovpnc->allow_compression->value}");

        # Update the client with new values and ensure it is correctly reconfigured on the backend
        $ovpnc->from_representation(
            dev_mode: 'tap',
            protocol: 'TCP4',
            interface: 'lan',
            local_port: '1195',
            digest: 'SHA512',
            data_ciphers: ['AES-256-GCM', 'AES-128-GCM'],
            data_ciphers_fallback: 'CHACHA20-POLY1305',
            tunnel_network: '10.1.2.0/24',
            allow_compression: 'yes',
        );
        $ovpnc->update();
        $ovpn_config = file_get_contents("/var/etc/openvpn/client{$ovpnc->vpnid->value}/config.ovpn");
        $this->assert_str_contains($ovpn_config, "dev-type {$ovpnc->dev_mode->value}");
        $lc_proto = strtolower($ovpnc->protocol->value);
        $this->assert_str_contains($ovpn_config, "proto $lc_proto");
        $if_ip = $ovpnc->interface->get_related_model()->get_current_ipv4();
        $this->assert_str_contains($ovpn_config, "local $if_ip");
        $this->assert_str_contains($ovpn_config, "lport {$ovpnc->local_port->value}");
        $this->assert_str_contains($ovpn_config, "auth {$ovpnc->digest->value}");
        $ciphers = implode(':', $ovpnc->data_ciphers->value) . ":{$ovpnc->data_ciphers_fallback->value}";
        $this->assert_str_contains($ovpn_config, "data-ciphers $ciphers");
        $this->assert_str_contains($ovpn_config, "data-ciphers-fallback {$ovpnc->data_ciphers_fallback->value}");
        $this->assert_str_contains($ovpn_config, "allow-compression {$ovpnc->allow_compression->value}");

        # Delete the OpenVPNClient and ensure it is no longer found on the backend
        $ovpnc->delete();
        $ifconfig = new Command("ifconfig ovpnc{$ovpnc->vpnid->value}");
        $this->assert_equals($ifconfig->result_code, 1);
    }

    /**
     * Checks that the `tls` field is hidden from representations by default.
     */
    public function test_tls_is_sensitive(): void {
        $ovpnc = new OpenVPNClient(tls: $this->tls_key);
        $this->assert_is_false(array_key_exists('tls', $ovpnc->to_representation()));
    }
}
