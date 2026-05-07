<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\FreeRADIUSUser;
use RESTAPI\Responses\ValidationError;

class APIModelsFreeRADIUSUserTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-freeradius3'];

    /**
     * Checks that we can create, read and delete FreeRADIUSUser models.
     */
    public function test_crud(): void {
        # Create a new FreeRADIUSUser with password and ensure it is present in the database
        $user = new FreeRADIUSUser(
            username: 'testuser',
            password: 'testpassword',
            password_encryption: 'Cleartext-Password',
            motp_enable: false,
            description: 'Test User',
        );
        $user->create();
        $raddb = file_get_contents('/usr/local/etc/raddb/users');
        $this->assert_str_contains($raddb, 'testuser" Cleartext-Password := "testpassword"');

        # Ensure we can read the created user from the config
        $read_user = new FreeRADIUSUser(id: $user->id);
        $this->assert_equals($read_user->username->value, 'testuser');
        $this->assert_equals($read_user->password_encryption->value, 'Cleartext-Password');
        $this->assert_equals($read_user->motp_enable->value, false);
        $this->assert_equals($read_user->description->value, 'Test User');

        # Ensure we can update the user
        $user = new FreeRADIUSUser(
            id: $read_user->id,
            username: 'motptestuser',
            motp_enable: true,
            motp_authmethod: 'motp',
            motp_secret: 'abcdef0123456789',
            motp_pin: '1234',
            motp_offset: 30,
        );
        $user->update();
        $raddb = file_get_contents('/usr/local/etc/raddb/users');
        $this->assert_str_does_not_contain($raddb, 'testuser" Cleartext-Password := "testpassword"');
        $this->assert_str_contains($raddb, '"motptestuser" Auth-Type = motp');
        $this->assert_str_contains($raddb, 'MOTP-Init-Secret = abcdef0123456789');
        $this->assert_str_contains($raddb, 'MOTP-PIN = 1234');
        $this->assert_str_contains($raddb, 'MOTP-Offset = 30');

        # Delete the user and ensure it is removed from the database
        $user->delete();
        $raddb = file_get_contents('/usr/local/etc/raddb/users');
        $this->assert_str_does_not_contain($raddb, '"motptestuser" Auth-Type = motp');
    }

    /**
     * Checks that all of the extended (non-MOTP) user attributes serialize into the FreeRADIUS users file.
     */
    public function test_extended_attributes(): void {
        $user = new FreeRADIUSUser(
            username: 'extuser',
            password: 'extpassword',
            password_encryption: 'Cleartext-Password',
            description: 'Extended attributes user',
            framed_ip_address: '192.0.2.10',
            framed_ip_netmask: '255.255.255.0',
            framed_route: '192.0.2.0/24 192.0.2.1 1',
            framed_ipv6_address: '2001:db8:abab::5',
            framed_ipv6_route: '2001:db8:0:16::/64 2001:db8::16:a0:20ff:fe99:a998 1',
            vlan_id: '100',
            wispr_redirection_url: 'http://www.example.com',
            simultaneous_connect: 5,
            expiration: 'Jan 01 2099',
            session_timeout: 3600,
            login_time: 'Wk0855-2305',
            amount_of_time: 60,
            point_of_time: 'Daily',
            max_total_octets: 1024,
            max_total_octets_time_range: 'monthly',
            max_bandwidth_down: 1000,
            max_bandwidth_up: 500,
            acct_interim_interval: 600,
            top_additional_options: ['DEFAULT Auth-Type = System'],
            check_items_additional_options: ['Max-Daily-Session := 36000'],
            reply_items_additional_options: ['Service-Type == Login-User'],
        );
        $user->create();

        # Read it back and validate the values were stored
        $read = new FreeRADIUSUser(id: $user->id);
        $this->assert_equals($read->simultaneous_connect->value, 5);
        $this->assert_equals($read->session_timeout->value, 3600);
        $this->assert_equals($read->expiration->value, 'Jan 01 2099');
        $this->assert_equals($read->vlan_id->value, '100');
        $this->assert_equals($read->point_of_time->value, 'Daily');
        $this->assert_equals($read->max_total_octets_time_range->value, 'monthly');
        $this->assert_equals($read->top_additional_options->value, ['DEFAULT Auth-Type = System']);

        # Verify the rendered raddb users file contains the expected RADIUS attributes
        $raddb = file_get_contents('/usr/local/etc/raddb/users');
        $this->assert_str_contains($raddb, '"extuser"');
        $this->assert_str_contains($raddb, 'Simultaneous-Use := "5"');
        $this->assert_str_contains($raddb, 'Expiration := "Jan 01 2099"');
        $this->assert_str_contains($raddb, 'Login-Time := "Wk0855-2305"');
        $this->assert_str_contains($raddb, 'Max-Daily-Session := 3600'); # 60 minutes * 60 seconds
        $this->assert_str_contains($raddb, 'Framed-IP-Address = 192.0.2.10');
        $this->assert_str_contains($raddb, 'Framed-IP-Netmask = 255.255.255.0');
        $this->assert_str_contains($raddb, 'Framed-Route = "192.0.2.0/24 192.0.2.1 1"');
        $this->assert_str_contains($raddb, 'Framed-IPv6-Address = 2001:db8:abab::5');
        $this->assert_str_contains($raddb, 'Session-Timeout := 3600');
        $this->assert_str_contains($raddb, 'Tunnel-Private-Group-ID = "100"');
        $this->assert_str_contains($raddb, 'WISPr-Redirection-URL := http://www.example.com');
        $this->assert_str_contains($raddb, 'DEFAULT Auth-Type = System');
        $this->assert_str_contains($raddb, 'Max-Daily-Session := 36000');
        $this->assert_str_contains($raddb, 'Service-Type == Login-User');

        # Bandwidth values should be multiplied by 1000 in the rendered output
        $this->assert_str_contains($raddb, '1000000'); # 1000 * 1000 (bandwidth down)
        $this->assert_str_contains($raddb, '500000'); # 500 * 1000 (bandwidth up)
        # Total octets should be multiplied by 1024*1024 in the rendered output
        $this->assert_str_contains($raddb, (string) (1024 * 1024 * 1024));

        $user->delete();
    }

    /**
     * Ensures choice fields reject invalid values.
     */
    public function test_pointoftime_rejects_invalid_choice(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                $user = new FreeRADIUSUser(username: 'badpoint', password: 'pw', point_of_time: 'Hourly');
                $user->validate();
            },
        );
    }

    /**
     * Ensures the bandwidth ceiling is enforced.
     */
    public function test_bandwidth_max_enforced(): void {
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () {
                $user = new FreeRADIUSUser(username: 'bwuser', password: 'pw', max_bandwidth_down: 4294968);
                $user->validate();
            },
        );
    }

    /**
     * Ensures the redirection URL field rejects invalid URLs.
     */
    public function test_redirection_url_validated(): void {
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () {
                $user = new FreeRADIUSUser(username: 'urluser', password: 'pw', wispr_redirection_url: 'not a url');
                $user->validate();
            },
        );
    }

    /**
     * Ensures that the login time regex validator rejects illegal characters.
     */
    public function test_login_time_regex(): void {
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () {
                $user = new FreeRADIUSUser(username: 'lt', password: 'pw', login_time: 'Wk@@@');
                $user->validate();
            },
        );
    }
}
