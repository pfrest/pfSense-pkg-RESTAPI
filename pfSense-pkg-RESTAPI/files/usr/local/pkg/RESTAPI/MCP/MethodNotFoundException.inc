<?php

namespace RESTAPI\MCP;

/**
 * Exception thrown by the MCP Server when a JSON-RPC request references an unknown method. The MCP Server converts
 * this exception into a JSON-RPC `Method not found` (-32601) error envelope.
 */
class MethodNotFoundException extends \RuntimeException {}

