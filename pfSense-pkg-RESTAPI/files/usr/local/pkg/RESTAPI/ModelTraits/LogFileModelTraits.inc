<?php

namespace RESTAPI\ModelTraits;

use RESTAPI\Core\Command;
use RESTAPI\Responses\NotAcceptableError;
use RESTAPI\Responses\NotFoundError;

/**
 * Defines a set of traits that are intended to be used by Models to read, decompress and process log files.
 */
trait LogFileModelTraits {
    /**
     * Checks if a given log file exists.
     * @param string $filepath The path to the log file.
     * @throws NotFoundError If the log file does not exist.
     */
    private function check_file_exists(string $filepath): void {
        # Check if the file exists
        if (!file_exists($filepath)) {
            throw new NotFoundError(
                message: "The log file at $filepath does not exist.",
                response_id: 'LOG_FILE_TRAITS_FILE_NOT_FOUND',
            );
        }
    }

    /**
     * Reads a given log file with no compression and returns the contents as an array of lines.
     * @param string $filepath The path to the log file.
     * @return array An array of log file contents.
     */
    private function read_uncompressed_log(string $filepath): array {
        # Check if the file exists
        $this->check_file_exists($filepath);

        # Read the file and return the contents as an array of lines
        return file($filepath, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
    }

    /**
     * Decompresses a bzip2 compressed log file and returns the contents as an array of lines.
     * @param string $filepath The path to the log file.
     * @return array An array of log file contents.
     */
    private function read_bzip2_log(string $filepath): array {
        # Check if the file exists
        $this->check_file_exists($filepath);

        # Decompress the file and return the contents as an array of lines
        return explode(PHP_EOL, bzdecompress(file_get_contents($filepath)));
    }

    /**
     * Decompresses a gzip compressed log file and returns the contents as an array of lines.
     * @param string $filepath The path to the log file.
     * @return array An array of log file contents.
     */
    private function read_gzip_log(string $filepath): array {
        # Check if the file exists
        $this->check_file_exists($filepath);

        # Decompress the file and return the contents as an array of lines
        return explode(PHP_EOL, gzdecode(file_get_contents($filepath)));
    }

    /**
     * Decompresses a xz compressed log file and returns the contents as an array of lines.
     * @param string $filepath The path to the log file.
     * @return array An array of log file contents.
     */
    private function read_xz_log(string $filepath): array {
        # Check if the file exists
        $this->check_file_exists($filepath);

        # Run xz to read the compressed file
        $xz_decompress = new Command('xz -dc ' . escapeshellarg($filepath));

        # Decompress the file and return the contents as an array of lines
        return explode(PHP_EOL, $xz_decompress->output);
    }

    /**
     * Gather all log filepaths for a given base log, ordered newest-first. The current (unrotated) log file is
     * always first, followed by rotated logs in ascending rotation number (0, 1, 2...).
     * Only matches the base log itself and files with a rotation suffix (e.g. .0, .1.gz, .2.bz2).
     * @param string $base_log The base log file path.
     * @return array An array of all log file paths ordered newest-first.
     */
    private function gather_log_filepaths(string $base_log): array {
        $log_filepaths = [];

        # Collect the base log and files with a dot-prefixed suffix (e.g. filter.log.0, filter.log.1.gz).
        # Using '.*' instead of '*' prevents matching unrelated files like filter.logrotate.
        if (file_exists($base_log)) {
            $log_filepaths[] = $base_log;
        }
        foreach (glob($base_log . '.*') as $log_filepath) {
            # Skip any glob result that is the base log itself (shouldn't happen with '.*' but be safe)
            if ($log_filepath !== $base_log) {
                $log_filepaths[] = $log_filepath;
            }
        }

        # Sort so the base log comes first (newest entries), then rotated by number ascending.
        # This ensures the newest log data is always read first regardless of filesystem ordering.
        usort($log_filepaths, function (string $a, string $b) use ($base_log): int {
            # Base log always comes first (it has the newest entries)
            if ($a === $base_log) {
                return -1;
            }
            if ($b === $base_log) {
                return 1;
            }

            # Extract rotation number from the suffix after the base log name.
            # For paths like filter.log.0, filter.log.1.gz, filter.log.10.bz2, we extract the
            # numeric portion immediately after the base name dot separator.
            # Anchoring to the end with an optional compression extension prevents false matches
            # on digits elsewhere in the path (e.g. daemon2.log would not match here since
            # we're only looking at the suffix after the base_log prefix).
            $suffix_a = substr($a, strlen($base_log));
            $suffix_b = substr($b, strlen($base_log));
            preg_match('/^\.(\d+)/', $suffix_a, $ma);
            preg_match('/^\.(\d+)/', $suffix_b, $mb);
            $na = isset($ma[1]) ? (int) $ma[1] : PHP_INT_MAX;
            $nb = isset($mb[1]) ? (int) $mb[1] : PHP_INT_MAX;
            return $na <=> $nb;
        });

        return $log_filepaths;
    }

    /**
     * Determines the compression type of a log file by normalizing its path (stripping numeric rotation suffixes).
     * @param string $filepath The log file path.
     * @return string The file extension indicating the log type (e.g. 'log', 'gz', 'bz2', 'xz').
     */
    private function get_log_type(string $filepath): string {
        # For uncompressed rotated logs, remove numeric extension to detect the correct type
        $normalized = preg_replace('/\.\d+$/', '', $filepath);
        return pathinfo($normalized, PATHINFO_EXTENSION);
    }

    /**
     * Reads the last $limit lines from a plain text file by seeking backwards from EOF.
     * Memory usage is O(limit) instead of O(file_size).
     * @param string $filepath The path to the log file.
     * @param int $limit Maximum number of lines to return from the end of the file.
     * @return array The last $limit non-empty lines from the file, in chronological order (oldest first).
     */
    private function read_tail_lines(string $filepath, int $limit): array {
        if ($limit <= 0) {
            return [];
        }

        $this->check_file_exists($filepath);

        $fh = fopen($filepath, 'rb');
        if ($fh === false) {
            return [];
        }

        fseek($fh, 0, SEEK_END);
        $pos = ftell($fh);
        $buffer = '';
        $lines = [];
        $chunk_size = 8192;

        while ($pos > 0 && count($lines) < $limit) {
            $read = min($chunk_size, $pos);
            $pos -= $read;
            fseek($fh, $pos);
            $buffer = fread($fh, $read) . $buffer;

            # Split on newlines and collect lines from the end
            $parts = explode("\n", $buffer);
            # The first element is a partial line (unless we hit BOF) - keep it in buffer
            $buffer = array_shift($parts);

            # Collect non-empty lines from the end of the chunk
            while (!empty($parts) && count($lines) < $limit) {
                $line = array_pop($parts);
                $line = rtrim($line, "\r");
                if ($line !== '') {
                    $lines[] = $line;
                }
            }
        }

        fclose($fh);

        # Don't forget remaining buffer content at the start of the file
        $buffer = rtrim($buffer, "\r");
        if ($buffer !== '' && count($lines) < $limit) {
            $lines[] = $buffer;
        }

        # $lines was collected newest-first; reverse to return in chronological order
        return array_reverse($lines);
    }

    /**
     * Streams a compressed file line-by-line using a file handle, keeping only the last $limit lines
     * via a ring buffer. Memory usage is O(limit) instead of O(decompressed_file_size).
     * @param resource $fh An already-opened readable file handle (from gzopen, bzopen, or popen).
     * @param int $limit Maximum number of lines to keep.
     * @return array The last $limit non-empty lines from the stream, in chronological order.
     */
    private function stream_tail_lines($fh, int $limit): array {
        if ($limit <= 0) {
            return [];
        }

        $ring = [];
        $ring_pos = 0;
        $ring_full = false;
        $line_buffer = '';

        # Read in chunks and split into lines manually for compatibility across
        # gzopen, bzopen, and popen handles (gzgets works for gz but not others)
        while (!feof($fh)) {
            $chunk = fread($fh, 8192);
            if ($chunk === false || $chunk === '') {
                break;
            }
            $line_buffer .= $chunk;

            # Process complete lines from the buffer
            while (($newline_pos = strpos($line_buffer, "\n")) !== false) {
                $line = rtrim(substr($line_buffer, 0, $newline_pos), "\r");
                $line_buffer = substr($line_buffer, $newline_pos + 1);

                if ($line === '') {
                    continue;
                }

                # Ring buffer: overwrite oldest entry when full
                if (!$ring_full && count($ring) < $limit) {
                    $ring[] = $line;
                    if (count($ring) === $limit) {
                        $ring_full = true;
                    }
                } else {
                    $ring[$ring_pos] = $line;
                    $ring_pos = ($ring_pos + 1) % $limit;
                }
            }
        }

        # Handle any remaining content without a trailing newline
        $line_buffer = rtrim($line_buffer, "\r");
        if ($line_buffer !== '') {
            if (!$ring_full && count($ring) < $limit) {
                $ring[] = $line_buffer;
            } else {
                $ring[$ring_pos] = $line_buffer;
                $ring_pos = ($ring_pos + 1) % $limit;
            }
        }

        # Reorder ring buffer to chronological order
        if ($ring_full || ($ring_pos > 0 && count($ring) === $limit)) {
            $ring = array_merge(array_slice($ring, $ring_pos), array_slice($ring, 0, $ring_pos));
        }

        return $ring;
    }

    /**
     * Reads a compressed log file and returns only the last $limit lines using streaming.
     * Uses PHP stream wrappers (compress.zlib://, compress.bzip2://) and popen for xz,
     * all of which return standard file handles compatible with fread()/feof().
     * Memory usage is O(limit) instead of O(decompressed_file_size).
     * @param string $filepath The path to the compressed log file.
     * @param int $limit Maximum number of lines to return.
     * @param string $type Compression type: 'gz', 'bz2', or 'xz'.
     * @return array The last $limit non-empty lines from the file, in chronological order.
     * @throws NotAcceptableError If the compression type is not supported.
     */
    private function read_compressed_tail_lines(string $filepath, int $limit, string $type): array {
        if ($limit <= 0) {
            return [];
        }

        $this->check_file_exists($filepath);

        # Use PHP stream wrappers for gz/bz2 so we get standard file handles that work with
        # fread()/feof(). This avoids needing separate gzread/bzread functions.
        # For xz, use popen since PHP has no built-in xz stream wrapper.
        $fh = match ($type) {
            'gz' => fopen('compress.zlib://' . $filepath, 'rb'),
            'bz2' => fopen('compress.bzip2://' . $filepath, 'rb'),
            'xz' => popen('xz -dc ' . escapeshellarg($filepath), 'r'),
            default => throw new NotAcceptableError(
                message: "Unsupported compression type '$type' for bounded log reading.",
                response_id: 'LOG_FILE_TRAITS_UNSUPPORTED_COMPRESSION_TYPE',
            ),
        };

        if ($fh === false) {
            return [];
        }

        $lines = $this->stream_tail_lines($fh, $limit);

        # Close the handle with the appropriate function
        match ($type) {
            'xz' => pclose($fh),
            default => fclose($fh),
        };

        return $lines;
    }

    /**
     * Reads all available logs for a given base log file path and returns the contents as an array of lines.
     * This will include rotated logs, including compressed logs.
     *
     * When $limit > 0 and $reverse is false, reads newest log files first and stops as soon as enough lines
     * are collected, avoiding loading the entire log history into memory.
     *
     * When $reverse is true, reads from the oldest entries first (beginning of oldest log files) and
     * paginates from there. This is useful for reading logs in chronological order.
     *
     * When $limit is 0 (default), all log files are read for full backward compatibility.
     *
     * @note zstd compressed logs are not supported.
     * @param string $base_log The base log file path.
     * @param int $limit Maximum total lines to return. 0 means unlimited (read everything).
     * @param int $offset Number of entries to skip before collecting.
     * @param bool $reverse When true, read from oldest entries first; when false, read from newest first.
     * @return array An array of all log file contents for the given base log.
     */
    public function read_log(string $base_log, int $limit = 0, int $offset = 0, bool $reverse = false): array {
        # Ensure the base log file exists
        $this->check_file_exists($base_log);

        # Gather all log file paths (ordered newest-first)
        $log_filepaths = $this->gather_log_filepaths($base_log);

        # Unbounded read: read all entries
        if ($limit <= 0 && $offset <= 0) {
            $log_contents = [];

            # Read oldest-first (reverse of our newest-first ordering)
            foreach (array_reverse($log_filepaths) as $log_filepath) {
                $type = $this->get_log_type($log_filepath);

                # Determine the log file type and add the contents to the log contents array
                $log_contents = match ($type) {
                    'bz2' => array_merge($log_contents, $this->read_bzip2_log($log_filepath)),
                    'gz' => array_merge($log_contents, $this->read_gzip_log($log_filepath)),
                    'xz' => array_merge($log_contents, $this->read_xz_log($log_filepath)),
                    'log' => array_merge($log_contents, $this->read_uncompressed_log($log_filepath)),
                    default => throw new NotAcceptableError(
                        message: "The log file at $log_filepath has an unsupported file extension.",
                        response_id: 'LOG_FILE_TRAITS_UNSUPPORTED_LOG_FILE_EXTENSION',
                    ),
                };
            }

            # Clear out empty array elements and reindex the array
            $log_contents = array_filter($log_contents);
            $log_contents = array_values($log_contents);

            # Reverse if requested (show newest first)
            if ($reverse) {
                $log_contents = array_reverse($log_contents);
            }

            # Map the log contents so each entry is an object with a 'text' property
            return array_map(fn($line) => ['text' => $line], $log_contents);
        }

        # Handle reverse pagination (reading from oldest entries first)
        if ($reverse) {
            return $this->read_log_reverse($log_filepaths, $limit, $offset);
        }

        # Normal pagination: read from newest entries first
        return $this->read_log_forward($log_filepaths, $limit, $offset);
    }

    /**
     * Reads log entries starting from the newest, with pagination support.
     * @param array $log_filepaths Log file paths ordered newest-first.
     * @param int $limit Maximum entries to return.
     * @param int $offset Number of newest entries to skip.
     * @return array Log entries with 'text' property.
     */
    private function read_log_forward(array $log_filepaths, int $limit, int $offset): array {
        $collected = [];
        $total_needed = $limit + $offset;

        foreach ($log_filepaths as $log_filepath) {
            $remaining = $total_needed - count($collected);
            if ($remaining <= 0) {
                break;
            }

            $type = $this->get_log_type($log_filepath);

            # Use memory-efficient tail readers for bounded reads
            $lines = match ($type) {
                'log' => $this->read_tail_lines($log_filepath, $remaining),
                'gz', 'bz2', 'xz' => $this->read_compressed_tail_lines($log_filepath, $remaining, $type),
                default => throw new NotAcceptableError(
                    message: "The log file at $log_filepath has an unsupported file extension.",
                    response_id: 'LOG_FILE_TRAITS_UNSUPPORTED_LOG_FILE_EXTENSION',
                ),
            };

            if (!empty($lines)) {
                # Prepend: lines from older (higher rotation number) files go before newer ones
                $collected = array_merge($lines, $collected);
            }
        }

        # Apply offset: skip the newest $offset entries (from the end)
        # $collected is in chronological order (oldest first), newest at end
        if (count($collected) > $total_needed) {
            $collected = array_slice($collected, -$total_needed);
        }

        if ($offset > 0 && count($collected) > $offset) {
            # Remove the newest $offset entries (from the end)
            $collected = array_slice($collected, 0, -$offset);
        } elseif ($offset > 0) {
            $collected = [];
        }

        # Take exactly $limit entries from the end (the newest remaining ones)
        if (count($collected) > $limit) {
            $collected = array_slice($collected, -$limit);
        }

        # Filter empties, reindex, and wrap in ['text' => ...] format
        $collected = array_values(array_filter($collected));
        return array_map(fn($line) => ['text' => $line], $collected);
    }

    /**
     * Reads log entries starting from the oldest, with pagination support.
     * Returns entries in reverse chronological order (newest first within the page).
     * @param array $log_filepaths Log file paths ordered newest-first.
     * @param int $limit Maximum entries to return.
     * @param int $offset Number of oldest entries to skip.
     * @return array Log entries with 'text' property, in reverse chronological order.
     */
    private function read_log_reverse(array $log_filepaths, int $limit, int $offset): array {
        $collected = [];
        $total_needed = $limit + $offset;
        $skipped = 0;

        # Read oldest-first (reverse of newest-first ordering)
        foreach (array_reverse($log_filepaths) as $log_filepath) {
            if (count($collected) >= $total_needed) {
                break;
            }

            $type = $this->get_log_type($log_filepath);

            # Read the entire file (we need to read from the beginning for oldest-first)
            $lines = match ($type) {
                'bz2' => $this->read_bzip2_log($log_filepath),
                'gz' => $this->read_gzip_log($log_filepath),
                'xz' => $this->read_xz_log($log_filepath),
                'log' => $this->read_uncompressed_log($log_filepath),
                default => throw new NotAcceptableError(
                    message: "The log file at $log_filepath has an unsupported file extension.",
                    response_id: 'LOG_FILE_TRAITS_UNSUPPORTED_LOG_FILE_EXTENSION',
                ),
            };

            # Filter empty lines
            $lines = array_filter($lines, fn($line) => $line !== '');

            foreach ($lines as $line) {
                # Skip the first $offset entries
                if ($skipped < $offset) {
                    $skipped++;
                    continue;
                }

                # Collect until we have $limit entries
                if (count($collected) < $limit) {
                    $collected[] = $line;
                }

                if (count($collected) >= $limit) {
                    break 2;
                }
            }
        }

        # Reverse the collected entries so newest (within the page) comes first
        $collected = array_reverse($collected);

        # Wrap in ['text' => ...] format
        return array_map(fn($line) => ['text' => $line], $collected);
    }
}
