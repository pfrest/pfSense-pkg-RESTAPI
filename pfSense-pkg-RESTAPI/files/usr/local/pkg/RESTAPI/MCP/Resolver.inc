<?php

namespace RESTAPI\MCP;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Auth;
use RESTAPI\Core\Model;
use RESTAPI\Core\Response;
use RESTAPI\Responses\ForbiddenError;

/**
 * Defines a class that maps/resolves MCP tool calls to the appropriate Model methods.
 *
 * The MCP Resolver mirrors the GraphQL Resolver to ensure that MCP tool calls execute the same Model logic and
 * are subject to the same per-Endpoint privilege checks as their REST and GraphQL counterparts.
 */
class Resolver {
    /**
     * Constructs the MCP Resolver object with a given Model object.
     * @param Model $model The Model object to be used for resolving MCP tool calls.
     */
    public function __construct(public Model $model) {}

    /**
     * Resolver that maps an MCP tool call to the Model's 'query' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The list of objects matched by the query in array representation.
     */
    public function query(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'query', auth: $auth);

        $query = $this->model->query(
            query_params: $args['query_params'] ?? [],
            limit: $args['limit'] ?? 0,
            offset: $args['offset'] ?? 0,
            reverse: $args['reverse'] ?? false,
            sort_by: $args['sort_by'] ?? [],
            sort_order: $args['sort_order'] ?? SORT_ASC,
        );

        return $query->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'read' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The read object in array representation.
     */
    public function read(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'read', auth: $auth);
        $model = new $this->model(id: $args['id'] ?? null, parent_id: $args['parent_id'] ?? null);
        return $model->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'create' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The created object in array representation.
     */
    public function create(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'create', auth: $auth);
        $model = new $this->model(data: $args);
        return $model->create()->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'update' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The updated object in array representation.
     */
    public function update(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'update', auth: $auth);
        $model = new $this->model(data: $args);
        return $model->update()->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'delete' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The deleted object in array representation.
     */
    public function delete(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'delete', auth: $auth);
        $model = new $this->model(data: $args);
        return $model->delete()->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'replace_all' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The replaced objects in array representation.
     */
    public function replace_all(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'replace_all', auth: $auth);
        $model = new $this->model();
        return $model->replace_all(data: $args['objects'] ?? [])->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'delete_many' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The deleted objects in array representation.
     */
    public function delete_many(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'delete_many', auth: $auth);
        $model = new $this->model();
        $deleted_objects = $model->delete_many(
            query_params: $args['query_params'] ?? [],
            limit: $args['limit'] ?? 0,
            offset: $args['offset'] ?? 0,
        );
        return $deleted_objects->to_representation();
    }

    /**
     * Resolver that maps an MCP tool call to the Model's 'delete_all' method.
     * @param array $args The arguments passed to the MCP tool call.
     * @param Auth $auth The Auth object representing the authenticated MCP client.
     * @return array The deleted objects in array representation.
     */
    public function delete_all(array $args, Auth $auth): array {
        $this->check_privs(resolver: 'delete_all', auth: $auth);
        $model = new $this->model();
        return $model->delete_all()->to_representation();
    }

    /**
     * Checks that the authenticated client has the privileges necessary to perform the requested resolver action.
     * The privileges enforced here are pulled directly from the corresponding REST API Endpoint to guarantee that
     * MCP tool calls require the exact same privileges as their REST API equivalents.
     * @param string $resolver The resolver method to check permissions for.
     * @param Auth $auth The Auth object to use for checking permissions.
     * @throws ForbiddenError If the user does not have the required privileges.
     */
    public function check_privs(string $resolver, Auth $auth): void {
        # Obtain the endpoint that corresponds with the Model and resolve method
        switch ($resolver) {
            case 'query':
                $endpoint = $this->model->get_related_endpoint(many: true);
                $auth->required_privileges = $endpoint->get_privileges;
                break;
            case 'read':
                $endpoint = $this->model->get_related_endpoint(many: false);
                $auth->required_privileges = $endpoint->get_privileges;
                break;
            case 'create':
                $endpoint = $this->model->get_related_endpoint(many: false);
                $auth->required_privileges = $endpoint->post_privileges;
                break;
            case 'update':
                $endpoint = $this->model->get_related_endpoint(many: false);
                $auth->required_privileges = $endpoint->patch_privileges;
                break;
            case 'delete':
                $endpoint = $this->model->get_related_endpoint(many: false);
                $auth->required_privileges = $endpoint->delete_privileges;
                break;
            case 'replace_all':
                $endpoint = $this->model->get_related_endpoint(many: true);
                $auth->required_privileges = $endpoint->put_privileges;
                break;
            case 'delete_all':
            case 'delete_many':
                $endpoint = $this->model->get_related_endpoint(many: true);
                $auth->required_privileges = $endpoint->delete_privileges;
                break;
            default:
                throw new ForbiddenError(
                    message: 'Unknown MCP resolver action requested.',
                    response_id: 'MCP_RESOLVER_UNKNOWN_CHECK_PRIVS_ACTION',
                );
        }

        # Throw an error if the user does not have the required privileges
        if (!$auth->authorize()) {
            throw new ForbiddenError(
                message: 'Authorization failed. You do not have sufficient privileges to access this resource.',
                response_id: 'MCP_RESOLVER_UNAUTHORIZED',
            );
        }
    }
}

