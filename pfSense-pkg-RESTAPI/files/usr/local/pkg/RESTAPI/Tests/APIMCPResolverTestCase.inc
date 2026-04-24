<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Auth;
use RESTAPI\Core\TestCase;
use RESTAPI\MCP\Resolver;
use RESTAPI\Models\FirewallAlias;
use RESTAPI\Responses\ForbiddenError;

class APIMCPResolverTestCase extends TestCase {
    /**
     * @var Auth $admin_auth Auth object representing an admin user (used to verify privileged calls succeed).
     */
    public Auth $admin_auth;

    /**
     * @var Auth $no_priv_auth Auth object representing a user with no REST API privileges.
     */
    public Auth $no_priv_auth;

    /**
     * Setup Auth objects representing privileged and unprivileged clients, plus a few aliases to operate on.
     */
    public function setup(): void {
        $this->admin_auth = new Auth();
        $this->admin_auth->username = 'admin';

        # An auth object whose username does not exist will fail authorization for every privilege.
        $this->no_priv_auth = new Auth();
        $this->no_priv_auth->username = 'mcp_no_such_user';

        # Seed a single alias to operate on.
        $alias = new FirewallAlias();
        $alias->replace_all([
            ['name' => 'mcp_resolver_test', 'type' => 'host', 'address' => ['1.1.1.1'], 'detail' => ['x']],
        ]);
    }

    /**
     * Cleans up test data after the tests are complete.
     */
    public function teardown(): void {
        (new FirewallAlias())->delete_all();
    }

    /**
     * Ensures the resolver pulls per-Endpoint privileges from the related Endpoint and matches the privileges
     * required by the equivalent REST API method. This is critical for ensuring MCP tool calls require the
     * exact same privileges as their REST counterparts.
     */
    public function test_check_privs_uses_endpoint_privileges(): void {
        $resolver = new Resolver(new FirewallAlias());

        # Read uses GET privileges.
        $resolver->check_privs(resolver: 'read', auth: $this->admin_auth);
        $endpoint = (new FirewallAlias())->get_related_endpoint(many: false);
        $this->assert_equals($this->admin_auth->required_privileges, $endpoint->get_privileges);

        # Create uses POST privileges.
        $resolver->check_privs(resolver: 'create', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $endpoint->post_privileges);

        # Update uses PATCH privileges.
        $resolver->check_privs(resolver: 'update', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $endpoint->patch_privileges);

        # Delete uses DELETE privileges.
        $resolver->check_privs(resolver: 'delete', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $endpoint->delete_privileges);

        # Many-resolver privileges come from the many endpoint.
        $many_endpoint = (new FirewallAlias())->get_related_endpoint(many: true);
        $resolver->check_privs(resolver: 'query', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $many_endpoint->get_privileges);
        $resolver->check_privs(resolver: 'replace_all', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $many_endpoint->put_privileges);
        $resolver->check_privs(resolver: 'delete_many', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $many_endpoint->delete_privileges);
        $resolver->check_privs(resolver: 'delete_all', auth: $this->admin_auth);
        $this->assert_equals($this->admin_auth->required_privileges, $many_endpoint->delete_privileges);
    }

    /**
     * Ensures the resolver throws a ForbiddenError when the authenticated client does not have the required
     * privileges to perform the requested action.
     */
    public function test_check_privs_throws_when_unauthorized(): void {
        $resolver = new Resolver(new FirewallAlias());

        $this->assert_throws_response(
            response_id: 'MCP_RESOLVER_UNAUTHORIZED',
            code: 403,
            callable: function () use ($resolver) {
                $resolver->check_privs(resolver: 'create', auth: $this->no_priv_auth);
            },
        );
    }

    /**
     * Ensures unknown resolver actions throw an MCP_RESOLVER_UNKNOWN_CHECK_PRIVS_ACTION ForbiddenError.
     */
    public function test_check_privs_unknown_action(): void {
        $resolver = new Resolver(new FirewallAlias());

        $this->assert_throws_response(
            response_id: 'MCP_RESOLVER_UNKNOWN_CHECK_PRIVS_ACTION',
            code: 403,
            callable: function () use ($resolver) {
                $resolver->check_privs(resolver: 'totally_unknown', auth: $this->admin_auth);
            },
        );
    }

    /**
     * Ensures resolver methods correctly delegate to the underlying Model and return array representations.
     */
    public function test_resolver_methods_return_representations(): void {
        $resolver = new Resolver(new FirewallAlias());

        $read = $resolver->read(['id' => 0], $this->admin_auth);
        $this->assert_equals($read['name'], 'mcp_resolver_test');

        $query = $resolver->query([], $this->admin_auth);
        $this->assert_equals(count($query), 1);
        $this->assert_equals($query[0]['name'], 'mcp_resolver_test');
    }
}

