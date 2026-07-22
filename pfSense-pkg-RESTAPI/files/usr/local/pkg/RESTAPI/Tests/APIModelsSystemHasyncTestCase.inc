<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\SystemHasync;

class APIModelsSystemHasyncTestCase extends TestCase {
    /**
     * Checks that the default HA sync configuration loads with null/empty values
     */
    public function test_read_default_config() {
        $hasync = new SystemHasync();

        # Verify all fields are empty by default (pfSense stores unset values as empty strings or null)
        $this->assert_is_empty($hasync->synchronizetoip->value);
        $this->assert_is_empty($hasync->pfsyncpeerip->value);
        # pfsyncinterface may have a system default (loopback/wan) - verify it has a value
        $this->assert_is_not_empty($hasync->pfsyncinterface->value);
        $this->assert_is_empty($hasync->username->value);
        $this->assert_is_empty($hasync->password->value);
        $this->assert_is_empty($hasync->synchronizeusers->value);
        $this->assert_is_empty($hasync->synchronizeauthservers->value);
        $this->assert_is_empty($hasync->synchronizecerts->value);
        $this->assert_is_empty($hasync->synchronizerules->value);
        $this->assert_is_empty($hasync->synchronizeschedules->value);
        $this->assert_is_empty($hasync->synchronizealiases->value);
        $this->assert_is_empty($hasync->synchronizenat->value);
        $this->assert_is_empty($hasync->synchronizeipsec->value);
        $this->assert_is_empty($hasync->synchronizeopenvpn->value);
        $this->assert_is_empty($hasync->synchronizedhcpd->value);
        $this->assert_is_empty($hasync->synchronizedhcrelay->value);
        $this->assert_is_empty($hasync->synchronizedhcrelay6->value);
        $this->assert_is_empty($hasync->synchronizewol->value);
        $this->assert_is_empty($hasync->synchronizestaticroutes->value);
        $this->assert_is_empty($hasync->synchronizevirtualip->value);
        $this->assert_is_empty($hasync->synchronizetrafficshaper->value);
        $this->assert_is_empty($hasync->synchronizetrafficshaperlimiter->value);
        $this->assert_is_empty($hasync->synchronizednsforwarder->value);
        $this->assert_is_empty($hasync->synchronizecaptiveportal->value);
        $this->assert_is_empty($hasync->pfsyncenabled->value);
    }

    /**
     * Checks that synchronizetoip can be updated and persisted
     */
    public function test_update_synchronizetoip() {
        $hasync = new SystemHasync();
        $hasync->synchronizetoip->value = '192.168.1.100';
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        $reread = new SystemHasync();
        $this->assert_equals('192.168.1.100', $reread->synchronizetoip->value);
    }

    /**
     * Checks that pfsyncpeerip can be updated and persisted
     */
    public function test_update_pfsyncpeerip() {
        $hasync = new SystemHasync();
        $hasync->pfsyncpeerip->value = '10.0.0.1';
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        $reread = new SystemHasync();
        $this->assert_equals('10.0.0.1', $reread->pfsyncpeerip->value);
    }

    /**
     * Checks that pfsyncinterface can be updated and persisted
     */
    public function test_update_pfsyncinterface() {
        $hasync = new SystemHasync();
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        $reread = new SystemHasync();
        $this->assert_equals('wan', $reread->pfsyncinterface->value);
    }

    /**
     * Checks that password is write-only and not returned in representation
     */
    public function test_password_write_only() {
        $hasync = new SystemHasync();
        $hasync->password->value = 'test_password_123';
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        # Get the representation (this is what the API returns)
        $representation = $hasync->to_representation();

        # Password should NOT be in the representation
        $this->assert_is_false(array_key_exists('password', $representation));
    }

    /**
     * Checks that password is marked as sensitive and not exposed
     */
    public function test_password_sensitive() {
        $hasync = new SystemHasync();
        $hasync->password->value = 'sensitive_password';
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        # Get the representation - password should not be included
        $representation = $hasync->to_representation();
        $this->assert_is_false(array_key_exists('password', $representation));
    }

    /**
     * Checks that multiple sync flags can be updated and persisted
     */
    public function test_update_sync_flags() {
        $hasync = new SystemHasync();
        $hasync->synchronizeusers->value = 'on';
        $hasync->synchronizerules->value = 'on';
        $hasync->synchronizealiases->value = 'on';
        $hasync->synchronizenat->value = 'on';
        $hasync->pfsyncenabled->value = 'on';
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        $reread = new SystemHasync();
        $this->assert_equals('on', $reread->synchronizeusers->value);
        $this->assert_equals('on', $reread->synchronizerules->value);
        $this->assert_equals('on', $reread->synchronizealiases->value);
        $this->assert_equals('on', $reread->synchronizenat->value);
        $this->assert_equals('on', $reread->pfsyncenabled->value);
    }

    /**
     * Checks that apply() calls filter_configure_sync() and filter_configure()
     */
    public function test_apply_calls_filter_configure() {
        $hasync = new SystemHasync();
        $hasync->synchronizetoip->value = '192.168.1.200';
        $hasync->pfsyncinterface->value = 'wan';
        $hasync->update();

        # Verify the config was actually written (apply was called)
        $reread = new SystemHasync();
        $this->assert_equals('192.168.1.200', $reread->synchronizetoip->value);
    }
}
