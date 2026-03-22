<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Auth;
use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;
use RESTAPI\Core\TestCaseRetry;
use RESTAPI\Models\FirewallRule;
use RESTAPI\Models\FirewallSchedule;
use RESTAPI\Models\RoutingGateway;
use RESTAPI\Models\TrafficShaper;
use RESTAPI\Models\TrafficShaperLimiter;
use RESTAPI\Models\TrafficShaperLimiterQueue;
use RESTAPI\Responses\ServerError;

class APIModelsFirewallRuleTestCase extends TestCase {
    /**
     * Reads the active ruleset directly from pfctl. If pfctl is not ready, it will retry up to 5 times before
     * throwing an error.
     * @param string|null $needle Optional string to search for in the output before returning
     * @return Command The Command object containing the pfctl output
     */
    public function read_pfctl_rules(?string $needle = null): Command {
        # Keywords that indicate pf is not ready yet
        $not_ready_keywords = ['pfctl: DIOCGETRULE: Device busy', 'pfctl: DIOCGETRULENV: Device busy'];

        # Check the pf ruleset until it appears to be fully loaded, or until we've tried 5 times
        $attempt = 0;
        $max_attempts = 5;
        while ($attempt < $max_attempts) {
            $cmd = new Command('/sbin/pfctl -sr');
            $ready = true;

            foreach ($not_ready_keywords as $keyword) {
                # pf is not ready if any of the keywords are found in the output
                if (str_contains($cmd->output, $keyword)) {
                    $ready = false;
                    break;
                }
            }

            # If pfctl is ready and either no needle was specified or the needle was found, return the output
            if ($ready and (!$needle or str_contains($cmd->output, $needle))) {
                sleep(1);
                return $cmd;
            }

            # Otherwise, wait a second and try again
            sleep(1);
            $attempt++;
        }

        throw new ServerError(
            message: "pfctl ruleset was not ready after $max_attempts attempts.",
            response_id: 'API_MODELS_FIREWALL_RULE_TEST_CASE_PFCTL_NOT_READY',
        );
    }

    /**
     * Checks that multiple interfaces cannot be assigned to a FirewallRule unless `floating` is enabled.
     */
    public function test_no_multiple_interface_without_floating() {
        # Ensure multiple interface values are not allowed for non floating FirewallRules
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_MULTIPLE_INTERFACE_WITHOUT_FLOATING',
            code: 400,
            callable: function () {
                $rule = new FirewallRule();
                $rule->interface->value = ['wan', 'lan'];
                $rule->validate_interface('wan');
            },
        );

        # Ensure multiple interface values are allowed for floating FirewallRules
        $this->assert_does_not_throw(
            callable: function () {
                $rule = new FirewallRule();
                $rule->interface->value = ['wan', 'lan'];
                $rule->floating->value = true;
                $rule->validate_interface('wan');
            },
        );
    }

    /**
     * Checks that the `statetype` can only be `synproxy state` if the `protocol` is `tcp`
     */
    public function test_no_synproxy_statetype_with_non_tcp_rule() {
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_SYNPROXY_STATE_TYPE_WITH_NON_TCP_PROTOCOL',
            code: 400,
            callable: function () {
                $rule = new FirewallRule();
                $rule->protocol->value = 'icmp';
                $rule->validate_statetype('synproxy state');
            },
        );
    }

    /**
     * Checks that the `statetype` can only be `synproxy state` if a `gateway` is not set.
     */
    public function test_no_synproxy_statetype_with_gateway_assigned() {
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_SYNPROXY_STATE_TYPE_WITH_GATEWAY',
            code: 400,
            callable: function () {
                $rule = new FirewallRule();
                $rule->protocol->value = 'tcp';
                $rule->gateway->value = 'TESTGW';
                $rule->validate_statetype('synproxy state');
            },
        );
    }

    /**
     * Checks that any values specific in `tcp_flags_set` must also be present in `tcp_flags_out_of`
     */
    public function test_tcp_flag_set_must_be_in_tcp_flags_out_of() {
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_TCP_FLAGS_SET_NOT_IN_TCP_FLAGS_OUT_OF',
            code: 400,
            callable: function () {
                $rule = new FirewallRule();
                $rule->tcp_flags_out_of->value = ['syn', 'ack'];
                $rule->validate_tcp_flags_set('rst');
            },
        );
    }

    /**
     * Checks that the `update_by` value is automatically updated with the current user regardless of what value
     * is currently is assigned.
     */
    public function test_updated_by_is_automatically_overwritten() {
        # Define a FirewallRule object and set it's client username and IP
        $rule = new FirewallRule();
        $rule->client->username = 'testuser1';
        $rule->client->ip_address = '127.0.0.1';

        # Ensure the `validate_updated_by()` method automatically generates a new value using the client user and IP
        $this->assert_equals(
            $rule->validate_updated_by("doesn't matter what this value is!"),
            "{$rule->client->username}@{$rule->client->ip_address} (API)",
        );

        # For good measure, update the client username and IP again and ensure it is automatically updated
        $rule->client->username = 'testuser2';
        $rule->client->ip_address = '127.0.0.2';
        $this->assert_equals(
            $rule->validate_updated_by("doesn't matter what this value is!"),
            "{$rule->client->username}@{$rule->client->ip_address} (API)",
        );
    }

    /**
     * Checks that basic firewall rules are actually configured by pfctl after creation, modification and deletion.
     */
    public function test_basic_crud() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'protocol' => 'tcp',
                'source' => '127.0.0.1',
                'source_port' => '80',
                'destination' => '127.0.0.2',
                'destination_port' => '443',
                'descr' => 'test description',
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Check that this rule is present in the pfctl rules
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $type = $rule->type->value;
        $if = "\${$rule->interface->get_related_models()->first()->descr->value}";
        $iptype = $rule->ipprotocol->value;
        $proto = $rule->protocol->value;
        $src = $rule->source->value;
        $srcport = $rule->source_port->value;
        $dst = $rule->destination->value;
        $dstport = $rule->destination_port->value;
        $descr = $rule->descr->value;
        $rid = $rule->tracker->value;
        $pfctl_rule_created =
            "$type  in  quick  on $if $iptype proto $proto  from $src port $srcport to $dst port " .
            "$dstport ridentifier $rid flags S/SA keep state label \"USER_RULE: $descr\" label \"id:$rid\"";
        $this->assert_str_contains($pfctl_rules, $pfctl_rule_created);

        # Update the firewall rule object with new values
        $rule->from_representation(
            data: [
                'type' => 'block',
                'interface' => ['lan'],
                'ipprotocol' => 'inet6',
                'protocol' => 'udp',
                'source' => 'fe80::1',
                'source_port' => '8080',
                'destination' => 'fe80::2',
                'destination_port' => '8443',
                'descr' => 'updated test description',
            ],
        );
        $rule->update(apply: true);

        # Check that the previous rule is no longer present in the pfctl rules but the updated rule is
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $type = $rule->type->value;
        $if = "\${$rule->interface->get_related_models()->first()->descr->value}";
        $iptype = $rule->ipprotocol->value;
        $proto = $rule->protocol->value;
        $src = $rule->source->value;
        $srcport = $rule->source_port->value;
        $dst = $rule->destination->value;
        $dstport = $rule->destination_port->value;
        $descr = $rule->descr->value;
        $rid = $rule->tracker->value;
        $pfctl_rule_updated =
            "$type  in  quick  on $if $iptype proto $proto  from $src port $srcport to $dst port " .
            "$dstport ridentifier $rid label \"USER_RULE: $descr\" label \"id:$rid\"";
        $this->assert_str_does_not_contain($pfctl_rules, $pfctl_rule_created);
        $this->assert_str_contains($pfctl_rules, $pfctl_rule_updated);

        # Delete the rule and ensure it is no longer present in pfctl
        $rule->delete(apply: true);
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $this->assert_str_does_not_contain($pfctl_rules, $pfctl_rule_updated);
    }

    /**
     * Checks that firewall rules created with a specific gateway assigned are properly set in pfctl
     */
    public function test_rule_with_gateway() {
        # Create a routing gatewy to test with
        $gateway = new RoutingGateway(
            data: [
                'name' => 'TEST_GW',
                'interface' => 'lan',
                'ipprotocol' => 'inet',
                'gateway' => '192.168.1.50',
            ],
        );
        $gateway->create();

        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'source' => 'any',
                'destination' => 'any',
                'gateway' => $gateway->name->value,
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Check that this rule is present in the pfctl rules
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $type = $rule->type->value;
        $if = "\${$rule->interface->get_related_models()->first()->descr->value}";
        $iptype = $rule->ipprotocol->value;
        $src = $rule->source->value;
        $dst = $rule->destination->value;
        $rid = $rule->tracker->value;
        $gw = $rule->gateway->value;
        $pfctl_rule_created = "pass  in  quick  on $if  \$GW$gw $iptype from $src to $dst ridentifier $rid keep state label \"USER_RULE\" label \"id:$rid\" label \"gw:$gw\"";
        $this->assert_str_contains($pfctl_rules, $pfctl_rule_created);

        # Delete the firewall rule and gateway
        $rule->delete(apply: true);
        $gateway->delete();
    }

    /**
     * Checks that disabled firewall rules are not present in pfctl
     */
    public function test_disabled() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'disabled' => true,
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'source' => 'any',
                'destination' => 'any',
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the created rule's tracker ID is not present in the pfctl rules
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $this->assert_str_does_not_contain($pfctl_rules, $rule->tracker->value);

        # Delete the firewall rule
        $rule->delete(apply: true);
    }

    /**
     * Checks that the `statetype` is properly set in the pfctl rule
     */
    public function test_statetype() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'source' => 'any',
                'destination' => 'any',
                'statetype' => 'sloppy state',
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the pfctl rule with this rule object's tracker is set to sloppy
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $pfctl_rule = "ridentifier {$rule->tracker->value} keep state (sloppy)";
        $this->assert_str_contains($pfctl_rules, $pfctl_rule);

        # Delete the firewall rule
        $rule->delete(apply: true);
    }

    /**
     * Checks that firewall rules with `log` enabled have the log flag set in pfctl
     */
    public function test_log() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'source' => 'any',
                'destination' => 'any',
                'log' => true,
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the log flag is set in the rule within pfctl
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $if = $rule->interface->get_related_models()->first()->descr->value;
        $type = $rule->type->value;
        $iptype = $rule->ipprotocol->value;
        $src = $rule->source->value;
        $dst = $rule->destination->value;
        $rid = $rule->tracker->value;
        $this->assert_str_contains(
            $pfctl_rules,
            "$type  in log  quick  on \$$if $iptype from $src to $dst ridentifier $rid",
        );

        # Delete the firewall rule
        $rule->delete(apply: true);
    }

    /**
     * Checks that firewall rules with `tcp_flags_any` enabled have the `any` flag set in pfctl
     */
    public function test_tcp_flags_any() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'protocol' => 'tcp',
                'source' => 'any',
                'destination' => 'any',
                'tcp_flags_any' => true,
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the `any` tcp flag is set in the rule within pfctl
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $this->assert_str_contains($pfctl_rules, "ridentifier {$rule->tracker->value} flags any");

        # Delete the firewall rule
        $rule->delete(apply: true);
    }

    /**
     * Checks that firewall rules with explicit TCP flags set are correctly added in pfctl
     */
    public function test_tcp_flags() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'protocol' => 'tcp',
                'source' => 'any',
                'destination' => 'any',
                'tcp_flags_set' => ['fin', 'syn', 'ack'],
                'tcp_flags_out_of' => ['fin', 'syn', 'rst', 'ack'],
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the `any` tcp flag is set in the rule within pfctl
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $this->assert_str_contains($pfctl_rules, "ridentifier {$rule->tracker->value} flags FSA/FSRA");

        # Delete the firewall rule
        $rule->delete(apply: true);
    }

    /**
     * Checks that firewall rules with `floating` enabled are correctly added in pfctl
     */
    public function test_floating() {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['wan', 'lan'],
                'ipprotocol' => 'inet',
                'protocol' => 'tcp',
                'source' => 'any',
                'destination' => 'any',
                'floating' => true,
                'quick' => false,
                'direction' => 'out',
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the floating rule is present in pfctl as configured above
        $pfctl_rules = file_get_contents('/tmp/rules.debug');
        $rid = $rule->tracker->value;
        $pfctl_rule = "pass  out  on {  {$this->env['PFREST_WAN_IF']}  {$this->env['PFREST_LAN_IF']}  } inet proto tcp  from any to any ridentifier $rid";
        $this->assert_str_contains($pfctl_rules, $pfctl_rule);

        # Delete the firewall rule
        $rule->delete(apply: true);
    }

    /**
     * Checks that firewall rules with a `sched` are correctly added in pfctl
     */
    public function test_sched_found_in_pfctl(): void {
        # Get the current month and day so we can test the schedule
        $current_month = (int) date('m');
        $current_day = (int) date('d');
        $test_day = $current_day !== 15 ? 15 : 14;

        # Create a new firewall schedule to test with
        $sched = new FirewallSchedule(
            data: [
                'name' => 'test_sched',
                'timerange' => [['month' => [$current_month], 'day' => [$current_day], 'hour' => '0:00-23:59']],
            ],
            async: false,
        );
        $sched->create();

        # Create a firewall rule to test with
        $rule = new FirewallRule(
            data: [
                'type' => 'pass',
                'interface' => ['lan'],
                'ipprotocol' => 'inet',
                'protocol' => 'tcp',
                'source' => 'any',
                'destination' => 'any',
                'sched' => $sched->name->value,
            ],
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the schedule is found tied to the rule in pfctl
        $pfctl = file_get_contents('/tmp/rules.debug');
        $this->assert_str_contains($pfctl, "s:{$sched->schedlabel->value}");

        # Update the schedule so it is inactive and ensure the rule is no longer present in pfctl
        $sched->timerange->value[0]['day'] = [$test_day];
        $sched->update();
        $pfctl = file_get_contents('/tmp/rules.debug');
        $this->assert_str_does_not_contain($pfctl, "s:{$sched->schedlabel->value}");

        # Update the rule to remove the schedule and ensure it is present again
        $rule->sched->value = null;
        $rule->update(apply: true);
        $pfctl = file_get_contents('/tmp/rules.debug');
        $this->assert_str_contains($pfctl, "id:{$rule->tracker->value}");
        $this->assert_str_does_not_contain($pfctl, "s:{$sched->schedlabel->value}");

        # Delete the rule and schedule
        $rule->delete();
        $sched->delete();
    }

    /**
     * Checks that the `defaultqueue` field is correctly shown in pfctl after creation, modification and deletion
     */
    public function test_defaultqueue_found_in_pfctl(): void {
        # Create TrafficShapers to test with
        $shaper1 = new TrafficShaper(
            interface: 'lan',
            scheduler: 'FAIRQ',
            bandwidth: 100,
            bandwidthtype: 'Mb',
            async: false,
            queue: [
                [
                    'name' => 'TestQueue1',
                    'qlimit' => 50,
                    'priority' => 1,
                    'bandwidth' => 50,
                    'bandwidthtype' => 'Mb',
                    'default' => true,
                ],
                [
                    'name' => 'TestQueue2',
                    'qlimit' => 50,
                    'priority' => 2,
                    'bandwidth' => 50,
                    'bandwidthtype' => 'Mb',
                    'red' => true,
                ],
            ],
        );
        $shaper1->create(apply: true);

        # Create a firewall rule that references a traffic shaper queue
        $rule = new FirewallRule(
            interface: ['lan'],
            type: 'pass',
            ipprotocol: 'inet',
            source: 'any',
            destination: 'any',
            defaultqueue: 'TestQueue1',
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the rule with the queue is seen in pfctl
        $pfctl = $this->read_pfctl_rules(needle: $rule->tracker->value);
        $this->assert_str_contains($pfctl->output, 'ridentifier ' . $rule->tracker->value . ' queue TestQueue1');

        # Update the rule to use a different queue and ensure it is seen in pfctl
        $rule->defaultqueue->value = 'TestQueue2';
        $rule->update(apply: true);
        $pfctl = $this->read_pfctl_rules(needle: $rule->tracker->value);
        $this->assert_str_contains($pfctl->output, 'ridentifier ' . $rule->tracker->value . ' queue TestQueue2');

        # Delete the rule and ensure the rule referencing the queue no longer exists
        $rule->delete();
        $shaper1->delete();
        $rule->apply();
        $pfctl = $this->read_pfctl_rules();
        $this->assert_str_does_not_contain(
            $pfctl->output,
            'ridentifier ' . $rule->tracker->value . ' queue TestQueue1',
        );
        $this->assert_str_does_not_contain(
            $pfctl->output,
            'ridentifier ' . $rule->tracker->value . ' queue TestQueue2',
        );
    }

    /**
     * Checks that the `ackqueue` field is correctly shown in pfctl after creation, modification and deletion
     */
    public function test_ackqueue_found_in_pfctl(): void {
        # Create TrafficShapers to test with
        $shaper1 = new TrafficShaper(
            interface: 'lan',
            scheduler: 'FAIRQ',
            bandwidth: 100,
            bandwidthtype: 'Mb',
            async: false,
            queue: [
                [
                    'name' => 'TestQueue1',
                    'qlimit' => 50,
                    'priority' => 1,
                    'bandwidth' => 50,
                    'bandwidthtype' => 'Mb',
                    'default' => true,
                ],
                [
                    'name' => 'TestQueue2',
                    'qlimit' => 50,
                    'priority' => 2,
                    'bandwidth' => 50,
                    'bandwidthtype' => 'Mb',
                    'red' => true,
                ],
            ],
        );
        $shaper1->create(apply: true);

        # Create a firewall rule that references a traffic shaper queue
        $rule = new FirewallRule(
            interface: ['lan'],
            type: 'pass',
            ipprotocol: 'inet',
            source: 'any',
            destination: 'any',
            defaultqueue: 'TestQueue1',
            ackqueue: 'TestQueue2',
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the rule with the queue is seen in pfctl
        $pfctl = $this->read_pfctl_rules(needle: $rule->tracker->value);
        $this->assert_str_contains(
            $pfctl->output,
            'ridentifier ' . $rule->tracker->value . ' queue(TestQueue1, TestQueue2)',
        );

        # Update the rule to use a different queue and ensure it is seen in pfctl
        $rule->defaultqueue->value = 'TestQueue2';
        $rule->ackqueue->value = 'TestQueue1';
        $rule->update(apply: true);
        $pfctl = $this->read_pfctl_rules();
        $this->assert_str_contains(
            $pfctl->output,
            'ridentifier ' . $rule->tracker->value . ' queue(TestQueue2, TestQueue1)',
        );

        # Delete the rule and ensure the rule referencing the queue no longer exists
        $rule->delete();
        $shaper1->delete();
        $rule->apply();
        $pfctl = $this->read_pfctl_rules();
        $this->assert_str_does_not_contain(
            $pfctl->output,
            'ridentifier ' . $rule->tracker->value . ' queue(TestQueue1, TestQueue2)',
        );
        $this->assert_str_does_not_contain(
            $pfctl->output,
            'ridentifier ' . $rule->tracker->value . ' queue(TestQueue2, TestQueue1)',
        );
    }

    /**
     * Checks that both the dnpipe and pdnpipe fields must be the name of an existing TrafficShaperLimiter or
     * TrafficShaperLimiterQueue.
     */
    public function test_dnpipe_and_pdnpipe_must_be_pipe_or_queue_name(): void {
        # Create a TrafficShaperLimiter object to test with
        $limiter = new TrafficShaperLimiter(
            name: 'test_limiter',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
        );
        $limiter->create();

        # Ensure an error is thrown if dnpipe is not an existing limiter name
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_DNPIPE_NOT_FOUND',
            code: 404,
            callable: function () {
                $rule = new FirewallRule(
                    interface: ['lan'],
                    type: 'pass',
                    ipprotocol: 'inet',
                    source: 'any',
                    destination: 'any',
                    dnpipe: 'does not exist!',
                    async: false,
                );
                $rule->validate();
            },
        );
        # Ensure an error is thrown if pdnpipe is not an existing limiter name
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_PDNPIPE_NOT_FOUND',
            code: 404,
            callable: function () use ($limiter) {
                $rule = new FirewallRule(
                    interface: ['lan'],
                    type: 'pass',
                    ipprotocol: 'inet',
                    source: 'any',
                    destination: 'any',
                    dnpipe: $limiter->name->value,
                    pdnpipe: 'does not exist!',
                    async: false,
                );
                $rule->validate();
            },
        );

        # Delete the limiter
        $limiter->delete(apply: true);
    }

    /**
     * Checks that a firewall rule cannot be created with a TrafficShaperLimiter AND a TrafficShaperLimiterQueue as
     * the dnpipe and pdnpipe fields. Both must be of the same class.
     */
    public function test_dnpipe_and_pdnpipe_limiter_types_must_match(): void {
        # Create a TrafficShaperLimiter and TrafficShaperLimiterQueue object to test with
        $limiter = new TrafficShaperLimiter(
            name: 'test_limiter',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
        );
        $limiter->create();
        $queue = new TrafficShaperLimiterQueue(
            parent_id: $limiter->id,
            name: 'test_queue',
            enabled: true,
            aqm: 'droptail',
        );
        $queue->create(apply: true);

        # Ensure an error is thrown when we try to use both a limiter and queue
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_PDNPIPE_DOES_NOT_MATCH_DNPIPE_LIMITER_TYPE',
            code: 409,
            callable: function () use ($limiter, $queue) {
                $rule = new FirewallRule(
                    interface: ['lan'],
                    type: 'pass',
                    ipprotocol: 'inet',
                    source: 'any',
                    destination: 'any',
                    dnpipe: $limiter->name->value,
                    pdnpipe: $queue->name->value,
                    async: false,
                );
                $rule->validate();
            },
        );

        # Delete the limiter and queue
        $queue->delete(apply: true);
        $limiter->delete(apply: true);
    }

    /**
     * Checks that the pdnpipe cannot be set unless dnpipe is already set
     */
    public function test_pdnpipe_cannot_be_set_without_dnpipe(): void {
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_PDNPIPE_SET_WITHOUT_DNPIPE',
            code: 400,
            callable: function () {
                $rule = new FirewallRule(
                    interface: ['lan'],
                    type: 'pass',
                    ipprotocol: 'inet',
                    source: 'any',
                    destination: 'any',
                    dnpipe: null,
                    pdnpipe: 'dnpipe should be set!',
                    async: false,
                );
                $rule->validate();
            },
        );
    }

    /**
     * Checks that the pdnpipe cannot use the same limiter as dnpipe
     */
    public function test_pdnpipe_cannot_match_dnpipe(): void {
        # Create a limiter to test with
        $limiter = new TrafficShaperLimiter(
            name: 'test_limiter',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
        );
        $limiter->create();

        # Ensure an error is thrown if dnpipe and pdnpipe both try to use the same limiter
        $this->assert_throws_response(
            response_id: 'FIREWALL_RULE_PDNPIPE_CANNOT_MATCH_DNPIPE',
            code: 400,
            callable: function () use ($limiter) {
                $rule = new FirewallRule(
                    interface: ['lan'],
                    type: 'pass',
                    ipprotocol: 'inet',
                    source: 'any',
                    destination: 'any',
                    dnpipe: $limiter->name->value,
                    pdnpipe: $limiter->name->value,
                    async: false,
                );
                $rule->validate();
            },
        );

        # Delete the limiter
        $limiter->delete(apply: true);
    }

    /**
     * Checks that a rule with a TrafficShaperLimiter object assigned to dnpipe is correctly represented in pfctl.
     */
    #[TestCaseRetry(retries: 3, delay: 2)]
    public function test_dnpipe_as_limiter_in_pfctl(): void {
        # Create a limiter and rule to test with
        $limiter = new TrafficShaperLimiter(
            name: 'test_limiter',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
            async: false,
        );
        $limiter->create(apply: true);
        $rule = new FirewallRule(
            interface: ['lan'],
            type: 'pass',
            ipprotocol: 'inet',
            protocol: 'tcp',
            source: 'any',
            destination: 'any',
            dnpipe: $limiter->name->value,
            descr: '',
            async: false,
        );
        $rule->create(apply: true);

        # Ensure the dnpipe is correctly represented
        $pfctl = $this->read_pfctl_rules(needle: $rule->tracker->value);
        $this->assert_str_contains(
            $pfctl->output,
            "ridentifier {$rule->tracker->value} dnpipe {$limiter->number->value}",
        );

        # Delete the limiter and rule
        $rule->delete(apply: true);
        $limiter->delete(apply: true);
    }

    /**
     * Checks that a rule with a TrafficShaperLimiterQueue object assigned to dnpipe is correctly represented in pfctl.
     */
    public function test_dnpipe_as_queue_in_pfctl(): void {
        # Create a limiter and rule to test with
        $limiter = new TrafficShaperLimiter(
            name: 'test_limiter',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
            async: false,
        );
        $limiter->create(apply: true);
        $queue = new TrafficShaperLimiterQueue(
            parent_id: $limiter->id,
            name: 'test_queue',
            enabled: true,
            aqm: 'droptail',
            async: false,
        );
        $queue->create(apply: true);
        $rule = new FirewallRule(
            interface: ['lan'],
            type: 'pass',
            ipprotocol: 'inet',
            protocol: 'tcp',
            source: 'any',
            destination: 'any',
            dnpipe: $queue->name->value,
            descr: '',
            async: false,
        );
        $rule->create(apply: true);

        # Check pfctl rules and ensure the dnpipe is correctly represented
        $pfctl = $this->read_pfctl_rules(needle: $rule->tracker->value);
        $this->assert_str_contains(
            $pfctl->output,
            "ridentifier {$rule->tracker->value} dnqueue {$queue->number->value}",
        );

        # Delete the limiter and rule
        $rule->delete(apply: true);
        $queue->delete(apply: true);
        $limiter->delete(apply: true);
    }

    /**
     * Checks that a rule with pdnpipe is correctly represented in pfctl.
     */
    public function test_pdnpipe_in_pfctl(): void {
        # Create a limiter and rule to test with
        $limiter1 = new TrafficShaperLimiter(
            name: 'test_limiter1',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
            async: false,
        );
        $limiter1->create(apply: true);
        $limiter2 = new TrafficShaperLimiter(
            name: 'test_limite2',
            enabled: true,
            aqm: 'droptail',
            sched: 'wf2q+',
            bandwidth: [['bw' => 20, 'bwscale' => 'Mb']],
            async: false,
        );
        $limiter2->create(apply: true);
        $rule = new FirewallRule(
            interface: ['lan'],
            type: 'pass',
            ipprotocol: 'inet',
            protocol: 'tcp',
            source: 'any',
            destination: 'any',
            dnpipe: $limiter1->name->value,
            pdnpipe: $limiter2->name->value,
            descr: '',
            async: false,
        );
        $rule->create(apply: true);

        # Check pfctl rules and ensure the dnpipe is correctly represented
        $pfctl = $this->read_pfctl_rules(needle: $rule->tracker->value);
        $this->assert_str_contains(
            $pfctl->output,
            "ridentifier {$rule->tracker->value} dnpipe({$limiter1->number->value}, {$limiter2->number->value})",
        );

        # Delete the limiter and rule
        $rule->delete(apply: true);
        $limiter2->delete(apply: true);
        $limiter1->delete(apply: true);
    }

    /**
     * Checks that the username of the client is correctly identified when replace_all is called.
     */
    public function test_replace_all_retains_client_username(): void {
        # Mock the authenticated client
        $client = new Auth();
        $client->username = 'testuser';
        $client->ip_address = '127.1.2.3';

        # Replace all firewall rules with a new set
        $rule_model = new FirewallRule(client: $client);
        $rules = $rule_model->replace_all(
            data: [
                [
                    'type' => 'pass',
                    'interface' => ['lan'],
                    'ipprotocol' => 'inet',
                    'source' => 'any',
                    'destination' => 'any',
                    'descr' => 'test description',
                ],
                [
                    'type' => 'pass',
                    'interface' => ['lan'],
                    'ipprotocol' => 'inet',
                    'source' => 'any',
                    'destination' => 'any',
                    'descr' => 'test description 2',
                ],
            ],
        );

        # Ensure the client username and IP are correctly set in the new rules
        foreach ($rules->model_objects as $rule) {
            $this->assert_equals($rule->client->username, $client->username);
            $this->assert_equals($rule->client->ip_address, $client->ip_address);
            $this->assert_equals($rule->created_by->value, "$client->username@$client->ip_address (API)");
            $this->assert_equals($rule->updated_by->value, "$client->username@$client->ip_address (API)");
        }
    }

    /**
     * Checks that we can create a floating firewall rule with the 'any' interface
     */
    public function test_floating_rule_with_any_interface(): void {
        # Create a firewall rule to test with
        $rule = new FirewallRule(
            type: 'pass',
            interface: ['any'],
            ipprotocol: 'inet',
            protocol: 'tcp',
            source: 'any',
            destination: 'any',
            floating: true,
        );
        $rule->validate();
    }
}
