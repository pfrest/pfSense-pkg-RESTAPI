<?php

namespace RESTAPI\ContentHandlers;

use RESTAPI\Core\ContentHandler;
use RESTAPI\Core\Endpoint;
use RESTAPI\Core\Response;
use RESTAPI\Responses\NotAcceptableError;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\Success;

/**
 * Defines a ContentHandler for handling binary data. This is primarily used to handle file downloads.
 * @note In context to Models, this content handler can only encode the data as a binary file for download if
 * the model contains both a $filename containing the name of the file to download and a $binary_data containing
 * the binary data to download.
 */
class BinaryContentHandler extends ContentHandler {
    /**
     * The MIME type associated with this content handler.
     * @var string $mime_type
     */
    public string $mime_type = 'application/octet-stream';

    /**
     * The help text for encoding content with this content handler.
     * @var string $encode_help_text
     */
    public string $encode_help_text = 'Downloads the response as generic binary data.';

    /**
     * Encodes/preps the given content as a binary file for download.
     * @param mixed|null $content The content to encode.
     * @param Response|null $context The Response object that contains the content to encode.
     * @return mixed The encoded content.
     */
    protected function _encode(mixed $content = null, ?Response $context = null): mixed {
        # We cannot encode error messages. If an error, rethrow it to be handled by the default Response handler
        if (!($context instanceof Success)) {
            throw $context;
        }

        # Extract the content's data array
        $content = $content['data'];

        # Ensure te content provided can even be encoded as a binary file
        if (!is_array($content) or !$content['binary_data'] or !$content['filename']) {
            throw new NotAcceptableError(
                message: 'The requested resource cannot be encoded as a binary file.',
                response_id: 'BINARY_CONTENT_HANDLER_RESOURCE_NOT_SUPPORTED',
            );
        }

        # Ensure the filename is actually a filename
        if (!preg_match('/^[a-zA-Z0-9_\-.]+$/', $content['filename'])) {
            throw new ServerError(
                message: 'The requested filename for this resource is invalid.',
                response_id: 'BINARY_CONTENT_HANDLER_INVALID_FILENAME',
            );
        }

        # Ensure the data is actually binary data
        if (mb_detect_encoding($content['binary_data'], 'UTF-8', true) !== false) {
            throw new ServerError(
                message: 'The requested data does not appear to be binary data.',
                response_id: 'BINARY_CONTENT_HANDLER_INVALID_DATA',
            );
        }

        header('Content-Description: File Transfer');
        header('Content-Disposition: attachment; filename="' . $content['filename'] . '"');
        header('Cache-Control: must-revalidate');
        header('Pragma: public');
        header('Expires: 0');
        return $content['binary_data'];
    }

    /**
     * @inheritDoc
     */
    public function to_openapi_schema(Response $response, Endpoint $endpoint): array {
        return [
            $this->mime_type => [
                'schema' => [
                    'type' => 'string',
                    'format' => 'binary',
                ],
            ],
        ];
    }
}
