<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\NetworkInterface;
use RESTAPI\Models\WireGuardSettings;
use RESTAPI\Models\WireGuardTunnel;

class APIModelsWireGuardTunnelTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-WireGuard'];

    /**
     * Ensures WireGuard is enabled before testing
     */
    public function setup(): void {
        # Only enable WireGuard if it's not already enabled
        $wg = new WireGuardSettings(async: false);
        if (!$wg->enable->value) {
            $wg->enable->value = true;
            $wg->update(apply: true);
        }
    }

    /**
     * Checks that the WireGuardTunnel object's `publickey` field value is populated after validation.
     */
    public function test_publickey_populated_after_validation(): void {
        # Create a WireGuardTunnel object with a private key to test with
        $tunnel = new WireGuardTunnel(privatekey: 'GNdQw+ujEIVgys4B2dDCXcBpiiQsNd2bAq5hnTp+smg=');
        $tunnel->validate();

        # Ensure the public was generated using the private key during validation
        $this->assert_is_not_empty($tunnel->publickey->value);
        $this->assert_type($tunnel->publickey->value, 'string');
    }

    /**
     * Checks that a tunnel cannot be disabled if it is assigned to an existing pfSense interface.
     */
    public function test_tunnel_cannot_be_disabled_while_assigned_to_interface(): void {
        # Create a WireGuardTunnel object with a private key to test with
        $tunnel = new WireGuardTunnel(privatekey: 'GNdQw+ujEIVgys4B2dDCXcBpiiQsNd2bAq5hnTp+smg=', async: false);
        $tunnel->create(apply: true);

        # Assign a NetworkInterface for this tunnel
        $if = new NetworkInterface(
            if: $tunnel->name->value,
            descr: 'WGTEST',
            enable: true,
            typev4: 'none',
            typev6: 'none',
            async: true,
        );
        $if->create();

        # Try to disable the tunnel and ensure it throws an error
        $this->assert_throws_response(
            response_id: 'WIREGUARD_TUNNEL_CANNOT_BE_DISABLED_WHILE_IN_USE_BY_INTERFACE',
            code: 409,
            callable: function () use ($tunnel) {
                $tunnel->enabled->value = false;
                $tunnel->validate();
            },
        );

        # Delete the interface and the tunnel
        $if->delete(apply: true);
        $tunnel->delete(apply: true);
    }

    /**
     * Checks that the `privatekey` field must be a valid WireGuard private key
     */
    public function test_privatekey_must_be_valid_wg_private_key(): void {
        $this->assert_throws_response(
            response_id: 'WIREGUARD_TUNNEL_INVALID_PRIVATE_KEY',
            code: 400,
            callable: function () {
                $tunnel = new WireGuardTunnel(privatekey: 'not valid private key!');
                $tunnel->validate();
            },
        );
    }

    /**
     * Checks that the `privatekey` value is forced to become a clamped value after validation.
     */
    public function test_privatekey_is_always_clamped_after_validation(): void {
        # Ensure an unclamped value becomes clamped after validation
        $tunnel = new WireGuardTunnel(privatekey: 'Lm0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3ALk=');
        $tunnel->validate();
        $this->assert_equals($tunnel->privatekey->value, 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=');

        # Ensure a preclamped value is accepted as is
        $tunnel = new WireGuardTunnel(privatekey: 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=');
        $tunnel->validate();
        $this->assert_equals($tunnel->privatekey->value, 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=');
    }

    /**
     * Checks that the get_next_tunnel_name() method correctly identifies the next tunnel interface name (i.e. tun_wg0)
     */
    public function test_get_next_tunnel_name(): void {
        # Create a new WireGuardTunnel object and ensure the next name is expected
        $tun_wg0 = new WireGuardTunnel(privatekey: 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=', async: false);
        $tun_wg0->create();
        $this->assert_equals($tun_wg0->name->value, 'tun_wg0');

        # Create another WireGuardTunnel object and ensure the next name is expected
        $tun_wg1 = new WireGuardTunnel();
        $this->assert_equals($tun_wg1->get_next_tunnel_name(), 'tun_wg1');

        # Delete tunnels
        $tun_wg0->delete(apply: true);
    }

    /**
     * Checks that the `addresses` field is removed before an update when the tunnel has an existing interface
     * assignment, since addresses are derived from the assigned interface's configured IPs instead.
     */
    public function test_addresses_removed_before_update_when_interface_is_assigned(): void {
        # Create a WireGuardTunnel to test with
        $tunnel = new WireGuardTunnel(privatekey: 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=', async: false);
        $tunnel->create(apply: true);

        # Assign a NetworkInterface for this tunnel
        $if = new NetworkInterface(
            if: $tunnel->name->value,
            descr: 'WGTEST',
            enable: true,
            typev4: 'none',
            typev6: 'none',
            async: false,
        );
        $if->create();

        # Update the tunnel with addresses while it has an interface assignment
        # pre_validate_update() should scrub the addresses before validation runs
        $tunnel->from_representation(addresses: [['address' => '172.20.99.1', 'mask' => 30]]);
        $tunnel->update(apply: false);

        # Ensure the addresses were removed before the update was processed
        $this->assert_is_empty($tunnel->addresses->value);

        # Delete the interface and the tunnel
        $if->delete(apply: true);
        $tunnel->delete(apply: true);
    }

    /**
     * Checks that the `addresses` field is left intact before an update when the tunnel has no existing
     * interface assignment.
     */
    public function test_addresses_retained_on_update_when_no_interface_assigned(): void {
        # Create a WireGuardTunnel to test with
        $tunnel = new WireGuardTunnel(privatekey: 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=', async: false);
        $tunnel->create(apply: true);

        # Update the tunnel with addresses without an interface assignment
        # pre_validate_update() should leave the addresses intact
        $tunnel->from_representation(addresses: [['address' => '172.20.99.1', 'mask' => 30]]);
        $tunnel->update(apply: true);

        # Ensure the addresses were retained during the update
        $this->assert_is_not_empty($tunnel->addresses->value);

        # Delete the tunnel
        $tunnel->delete(apply: true);
    }

    /**
     * Checks that the tunnel is properly configured on the backend after creating, updating and deleting
     */
    public function test_crud(): void {
        # Create a new tunnel
        $tunnel = new WireGuardTunnel(
            privatekey: 'KG0BA4UyPilHH5qnXCfr6Lw8ynecOPor88tljLy3AHk=',
            listenport: '55000',
            descr: 'test',
            async: false,
        );
        $tunnel->create(apply: true);

        # Ensure the tunnel is present in wg showconf and the public key is present in wg show
        $wg_showconf = new Command('wg showconf ' . $tunnel->name->value);
        $wg_show = new Command('wg show ' . $tunnel->name->value);
        $this->assert_str_contains($wg_showconf->output, 'ListenPort = ' . $tunnel->listenport->value);
        $this->assert_str_contains($wg_showconf->output, 'PrivateKey = ' . $tunnel->privatekey->value);
        $this->assert_str_contains($wg_show->output, 'public key: ' . $tunnel->publickey->value);
        $this->assert_equals($tunnel->descr->value, 'test');

        # Update the tunnel with new values
        $tunnel->from_representation(privatekey: 'GNdQw+ujEIVgys4B2dDCXcBpiiQsNd2bAq5hnTp+smg=', listenport: '51820');
        $tunnel->update(apply: true);

        # Ensure the updated tunnel is present in wg showconf
        $wg_showconf = new Command('wg showconf ' . $tunnel->name->value);
        $wg_show = new Command('wg show ' . $tunnel->name->value);
        $this->assert_str_contains($wg_showconf->output, 'ListenPort = ' . $tunnel->listenport->value);
        $this->assert_str_contains($wg_showconf->output, 'PrivateKey = ' . $tunnel->privatekey->value);
        $this->assert_str_contains($wg_show->output, 'public key: ' . $tunnel->publickey->value);

        # Delete the tunnel and ensure it is no longer present in wg show
        $tunnel->delete(apply: true);
        $wg_show = new Command('wg show ' . $tunnel->name->value);
        $this->assert_equals($wg_show->result_code, 1);
        $this->assert_str_contains($wg_show->output, 'Device not configured');
    }
}
