<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;
use RESTAPI\Core\TestCaseRetry;
use RESTAPI\Models\FirewallAdvancedSettings;
use RESTAPI\Models\FirewallAlias;

class APIModelsFirewallAdvancedSettingsTestCase extends TestCase {
    /**
     * Checks that the aliasesresolveinterval field correctly configures the filterdns service.
     */
    #[TestCaseRetry(retries: 3, delay: 5)]
    public function test_aliasesresolveinterval(): void {
        # We need an FQDN alias first to enable the filterdns service
        $alias = new FirewallAlias(name: 'test', type: 'host', address: ['example.com'], async: true);
        $alias->create(apply: true);

        # Update the aliasesresolveinterval value and check that the filterdns service uses the correct interval
        $firewall_advanced_settings = new FirewallAdvancedSettings(aliasesresolveinterval: 500, async: true);
        $firewall_advanced_settings->update(apply: true);
        sleep(3);
        $filterdns_ps = new Command('ps aux');
        $this->assert_str_contains($filterdns_ps->output, '/usr/local/sbin/filterdns -p /var/run/filterdns.pid -i 500');

        $firewall_advanced_settings = new FirewallAdvancedSettings(aliasesresolveinterval: 222, async: true);
        $firewall_advanced_settings->update(apply: true);
        sleep(3);
        $filterdns_ps = new Command('ps aux');
        $this->assert_str_contains($filterdns_ps->output, '/usr/local/sbin/filterdns -p /var/run/filterdns.pid -i 222');

        # Clean up the alias
        $alias->delete(apply: true);
    }

    /**
     * Checks that the checkaliasesurlcert field can be updated.
     */
    public function test_checkaliasesurlcert(): void {
        $firewall_advanced_settings = new FirewallAdvancedSettings(checkaliasesurlcert: false, async: false);
        $firewall_advanced_settings->update(apply: true);

        $firewall_advanced_settings = new FirewallAdvancedSettings(checkaliasesurlcert: true, async: false);
        $firewall_advanced_settings->update(apply: true);
    }
}
