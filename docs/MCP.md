# Model Context Protocol (MCP)

The package includes a fully-dynamic [Model Context Protocol](https://modelcontextprotocol.io/) (MCP) server that
exposes every REST API operation as an MCP tool. MCP is a JSON-RPC 2.0 based protocol designed to give large language
models (LLMs) and AI agents a standardized way to discover and invoke tools, fetch resources and exchange context with
external systems. The MCP server in this package is built on top of the official [`mcp/sdk`](https://packagist.org/packages/mcp/sdk)
PHP SDK and is generated dynamically from the same Endpoint and Model classes that power the REST and GraphQL APIs, so
any new Endpoint added to the package is automatically exposed as an MCP tool.

!!! Tips
    If you are new to APIs in general, it is recommended to start with the REST API first. The MCP server is intended
    primarily for AI agents/LLM clients (e.g. Claude Desktop, Cursor, custom agent runtimes) that already speak MCP.

## Enabling the MCP Server

The MCP server is **disabled by default** and must be explicitly enabled before use. You can enable it by setting the
`mcp_enabled` field on the REST API settings:

=== "REST"

    ```http
    PATCH /api/v2/system/restapi/settings
    Content-Type: application/json

    { "mcp_enabled": true }
    ```

=== "GraphQL"

    ```graphql
    mutation {
      updateRestapiSettings(mcp_enabled: true) {
        mcp_enabled
      }
    }
    ```

When `mcp_enabled` is `false`, all calls to `/api/v2/mcp` return a JSON-RPC error envelope with a `response_id` of
`MCP_SERVER_NOT_ENABLED`.

## Authentication and Authorization

The MCP server uses the same authentication methods and privileges as the REST API. For more information on
authentication, please refer to the [Authentication and Authorization](AUTHENTICATION_AND_AUTHORIZATION.md) documentation.

!!! Important
    The user must at least have the `api-v2-mcp-post` privilege to access the MCP server. From there, the user must
    also have the relevant privileges for the underlying REST API operation each MCP tool maps to. For example,
    invoking the `createFirewallAlias` MCP tool requires the same privilege as performing a `POST` request to
    `/api/v2/firewall/alias`.

## Endpoint and Transport

| Property        | Value                                  |
|-----------------|----------------------------------------|
| URL             | `/api/v2/mcp`                          |
| Method          | `POST`                                 |
| Content-Type    | `application/json`                     |
| Transport       | Stateless HTTP request/response        |
| Protocol        | JSON-RPC 2.0 (MCP)                     |

Every MCP request is a single HTTP `POST` containing a JSON-RPC 2.0 envelope. The server returns a single JSON-RPC
response in the body. Notifications (requests without an `id`) receive an empty response. The MCP server intentionally
does **not** use Streamable HTTP / SSE transports — every interaction is a single request/response cycle.

## Supported MCP Methods

The MCP server implements the following standard MCP methods:

| Method        | Description                                                                  |
|---------------|------------------------------------------------------------------------------|
| `initialize`  | Returns server capabilities and identification information.                  |
| `ping`        | Returns an empty result. Useful for liveness checks.                         |
| `tools/list`  | Returns every tool dynamically generated from this API's Endpoints.          |
| `tools/call`  | Invokes a specific tool with the supplied arguments.                         |

## Tool Naming Convention

To stay consistent with the GraphQL API, MCP tool names use the same naming convention as GraphQL operations:

| Operation       | Pattern                              | Example                       |
|-----------------|--------------------------------------|-------------------------------|
| Read            | `read{ModelName}`                    | `readFirewallAlias`           |
| Query (many)    | `query{ModelName}s`                  | `queryFirewallAliases`        |
| Create          | `create{ModelName}`                  | `createFirewallAlias`         |
| Update          | `update{ModelName}`                  | `updateFirewallAlias`         |
| Delete          | `delete{ModelName}`                  | `deleteFirewallAlias`         |
| Replace All     | `replaceAll{ModelName}s`             | `replaceAllFirewallAliases`   |
| Delete Many     | `deleteMany{ModelName}s`             | `deleteManyFirewallAliases`   |
| Delete All      | `deleteAll{ModelName}s`              | `deleteAllFirewallAliases`    |

The complete list of tools available on your pfSense instance can always be obtained by issuing a `tools/list` request.

## Examples

### `initialize`

```http
POST /api/v2/mcp
Content-Type: application/json

{
  "jsonrpc": "2.0",
  "id": 1,
  "method": "initialize",
  "params": { "protocolVersion": "2025-06-18" }
}
```

```json
{
  "jsonrpc": "2.0",
  "id": 1,
  "result": {
    "protocolVersion": "2025-06-18",
    "capabilities": { "tools": true },
    "serverInfo": { "name": "pfSense-pkg-RESTAPI MCP Server", "version": "..." },
    "instructions": "This MCP server exposes the pfSense REST API as dynamically generated tools..."
  }
}
```

### `tools/list`

```http
POST /api/v2/mcp
Content-Type: application/json

{ "jsonrpc": "2.0", "id": 2, "method": "tools/list" }
```

```json
{
  "jsonrpc": "2.0",
  "id": 2,
  "result": {
    "tools": [
      {
        "name": "readFirewallAlias",
        "description": "Reads a single Firewall Alias object. Equivalent to GET /api/v2/firewall/alias.",
        "inputSchema": {
          "type": "object",
          "properties": { "id": { "type": "integer", "description": "..." } },
          "required": ["id"]
        }
      }
    ]
  }
}
```

### `tools/call`

```http
POST /api/v2/mcp
Content-Type: application/json

{
  "jsonrpc": "2.0",
  "id": 3,
  "method": "tools/call",
  "params": {
    "name": "createFirewallAlias",
    "arguments": {
      "name": "example",
      "type": "host",
      "address": ["1.1.1.1"],
      "descr": "Created via MCP"
    }
  }
}
```

```json
{
  "jsonrpc": "2.0",
  "id": 3,
  "result": {
    "isError": false,
    "content": [
      { "type": "text", "text": "{\n  \"id\": 0,\n  \"name\": \"example\",\n  \"type\": \"host\",\n  \"address\": [\"1.1.1.1\"]\n}" }
    ]
  }
}
```

## Error Handling

The MCP server returns errors using two distinct mechanisms, depending on where the failure originated:

- **Protocol-level errors** (unknown method, invalid params, internal error, MCP server disabled, authorization
  failure) are returned as a JSON-RPC `error` envelope:

  ```json
  {
    "jsonrpc": "2.0",
    "id": 3,
    "error": {
      "code": -32601,
      "message": "Unknown MCP method 'foo'."
    }
  }
  ```

- **Tool-level errors** (a Model validation error during a `tools/call`, for example) are returned inside a successful
  `CallToolResult` with `isError` set to `true`. This is per the MCP specification so the LLM can see the error and
  self-correct.

  ```json
  {
    "jsonrpc": "2.0",
    "id": 3,
    "result": {
      "isError": true,
      "content": [{ "type": "text", "text": "{\n  \"message\": \"...\",\n  \"response_id\": \"...\"\n}" }]
    }
  }
  ```

All MCP responses always return HTTP 200, regardless of whether the request succeeded — exactly like the GraphQL API.

## Differences from REST and GraphQL

- All MCP requests are made to the `/api/v2/mcp` endpoint via a `POST`.
- Tools mirror the GraphQL operation names so the same vocabulary is used between MCP and GraphQL.
- The MCP server is disabled by default and must be enabled in the REST API settings.
- The MCP server itself is excluded from the `tools/list` response — it cannot expose itself as a tool.
- Privileges enforced by MCP tool calls are pulled directly from the corresponding REST API Endpoint, ensuring an
  MCP tool call requires the exact same privileges as its REST API equivalent.

## Why Use MCP?

MCP is an emerging standard supported by a growing number of LLM clients and agent runtimes. Exposing the pfSense
REST API as MCP tools allows AI agents to discover and operate on your firewall using a standard, well-defined
protocol — without you needing to write any glue code. Because the tool catalog is generated dynamically from the
Endpoint and Model classes, every new feature added to the REST API is automatically available to MCP clients.

