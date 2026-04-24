<?php

namespace RESTAPI\MCP;

/**
 * Exception thrown by the MCP Server when a JSON-RPC request contains invalid parameters. The MCP Server converts
 * this exception into a JSON-RPC `Invalid params` (-32602) error envelope.
 */
class InvalidParamsException extends \RuntimeException {}

