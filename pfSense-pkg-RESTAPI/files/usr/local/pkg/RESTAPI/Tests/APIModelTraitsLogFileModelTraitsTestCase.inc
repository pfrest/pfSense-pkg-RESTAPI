<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\SystemLog;

class APIModelTraitsLogFileModelTraitsTestCase extends TestCase {
    /**
     * Clean up temporary test log files before and after each test to prevent cross-contamination.
     */
    private function cleanup_test_files(): void {
        foreach (glob('/tmp/test_*.log*') as $file) {
            @unlink($file);
        }
    }

    public function setup(): void {
        $this->cleanup_test_files();
    }

    public function teardown(): void {
        $this->cleanup_test_files();
    }

    /**
     * Checks that the LogFileModelTraits::check_file_exists() method throws a NotFoundError when the file does not exist.
     */
    public function test_check_file_exists(): void {
        # Ensure an error is thrown when the requested log file does not exist
        $this->assert_throws_response(
            response_id: 'LOG_FILE_TRAITS_FILE_NOT_FOUND',
            code: 404,
            callable: function () {
                # Use the SystemLog Model to test the LogFileModelTraits::check_file_exists() method
                $model = new SystemLog();

                # LogFileModelTraits::check_file_exists() is private, call it via read_log() instead
                $model->read_log('/var/log/does_not_exist.log');
            },
        );

        # Ensure no error is thrown when the requested log file exists
        $this->assert_does_not_throw(
            callable: function () {
                # Use the SystemLog Model to test the LogFileModelTraits::check_file_exists() method
                $model = new SystemLog();

                # LogFileModelTraits::check_file_exists() is private, call it via read_log() instead
                $model->read_log('/var/log/system.log');
            },
        );
    }

    /**
     * Checks that the LogFileModelTraits::read_uncompressed_log() method reads an uncompressed log file correctly.
     */
    public function test_read_uncompressed_log(): void {
        # Use the SystemLog Model to test the LogFileModelTraits::read_uncompressed_log() method
        $model = new SystemLog();

        # Create some mock log files that are uncompressed
        file_put_contents('/tmp/test_uncompressed.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_uncompressed.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_uncompressed.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read the system log file
        $log = $model->read_log('/tmp/test_uncompressed.log');

        # Ensure all log files were read correctly and in the correct order
        $this->assert_equals(
            [
                ['text' => 'Line 1'],
                ['text' => 'Line 2'],
                ['text' => 'Line 3'],
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that the LogFileModelTraits::read_bzip2_log() method reads a bzip2 compressed log file correctly.
     */
    public function test_read_bzip2_log(): void {
        # Use the SystemLog Model to test the LogFileModelTraits::read_bzip2_log() method
        $model = new SystemLog();

        # Create some mock log files that are bzip2 compressed
        file_put_contents('/tmp/test_bzip2.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_bzip2.log.0.bz2', bzcompress("Line 4\nLine 5\nLine 6\n"));
        file_put_contents('/tmp/test_bzip2.log.1.bz2', bzcompress("Line 1\nLine 2\nLine 3\n"));

        # Read the system log file
        $log = $model->read_log('/tmp/test_bzip2.log');

        # Ensure all log files were read correctly and in the correct order
        $this->assert_equals(
            [
                ['text' => 'Line 1'],
                ['text' => 'Line 2'],
                ['text' => 'Line 3'],
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that the LogFileModelTraits::read_gzip_log() method reads a gzip compressed log file correctly.
     */
    public function test_read_gzip_log(): void {
        # Use the SystemLog Model to test the LogFileModelTraits::read_gzip_log() method
        $model = new SystemLog();

        # Create some mock log files that are gzip compressed
        file_put_contents('/tmp/test_gzip.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_gzip.log.0.gz', gzencode("Line 4\nLine 5\nLine 6\n"));
        file_put_contents('/tmp/test_gzip.log.1.gz', gzencode("Line 1\nLine 2\nLine 3\n"));

        # Read the system log file
        $log = $model->read_log('/tmp/test_gzip.log');

        # Ensure all log files were read correctly and in the correct order
        $this->assert_equals(
            [
                ['text' => 'Line 1'],
                ['text' => 'Line 2'],
                ['text' => 'Line 3'],
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that the LogFileModelTraits::read_xz_log() method reads an xz compressed log file correctly.
     */
    public function test_read_xz_log(): void {
        # Use the SystemLog Model to test the LogFileModelTraits::read_xz_log() method
        $model = new SystemLog();

        # Create some mock log files that are xz compressed
        file_put_contents('/tmp/test_xz.log', "Line 7\nLine 8\nLine 9\n");
        new Command('printf "Line 4\nLine 5\nLine 6\n" | xz -c > /tmp/test_xz.log.0.xz');
        new Command('printf "Line 1\nLine 2\nLine 3\n" | xz -c > /tmp/test_xz.log.1.xz');

        # Read the system log file
        $log = $model->read_log('/tmp/test_xz.log');

        # Ensure all log files were read correctly and in the correct order
        $this->assert_equals(
            [
                ['text' => 'Line 1'],
                ['text' => 'Line 2'],
                ['text' => 'Line 3'],
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with a limit returns only the newest entries from uncompressed logs.
     */
    public function test_read_log_bounded_uncompressed(): void {
        $model = new SystemLog();

        # Create mock log files
        file_put_contents('/tmp/test_bounded.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_bounded.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_bounded.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with limit=4 should return the 4 newest lines
        $log = $model->read_log('/tmp/test_bounded.log', limit: 4);

        $this->assert_equals(
            [['text' => 'Line 6'], ['text' => 'Line 7'], ['text' => 'Line 8'], ['text' => 'Line 9']],
            $log,
        );
    }

    /**
     * Checks that read_log() with a limit smaller than the current log file only reads the current file.
     */
    public function test_read_log_bounded_current_file_only(): void {
        $model = new SystemLog();

        # Create mock log files
        file_put_contents('/tmp/test_current.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_current.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_current.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with limit=2 should return only the 2 newest lines from the current log
        $log = $model->read_log('/tmp/test_current.log', limit: 2);

        $this->assert_equals([['text' => 'Line 8'], ['text' => 'Line 9']], $log);
    }

    /**
     * Checks that read_log() with a limit works correctly with gzip compressed rotated logs.
     */
    public function test_read_log_bounded_gzip(): void {
        $model = new SystemLog();

        # Create mock log files with gzip compressed rotated logs
        file_put_contents('/tmp/test_bounded_gz.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_bounded_gz.log.0.gz', gzencode("Line 4\nLine 5\nLine 6\n"));
        file_put_contents('/tmp/test_bounded_gz.log.1.gz', gzencode("Line 1\nLine 2\nLine 3\n"));

        # Read with limit=5 should return the 5 newest lines across current and first rotated file
        $log = $model->read_log('/tmp/test_bounded_gz.log', limit: 5);

        $this->assert_equals(
            [
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with a limit works correctly with bzip2 compressed rotated logs (streaming).
     */
    public function test_read_log_bounded_bzip2(): void {
        $model = new SystemLog();

        # Create mock log files with bzip2 compressed rotated logs
        file_put_contents('/tmp/test_bounded_bz2.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_bounded_bz2.log.0.bz2', bzcompress("Line 4\nLine 5\nLine 6\n"));
        file_put_contents('/tmp/test_bounded_bz2.log.1.bz2', bzcompress("Line 1\nLine 2\nLine 3\n"));

        # Read with limit=5 should return the 5 newest lines
        $log = $model->read_log('/tmp/test_bounded_bz2.log', limit: 5);

        $this->assert_equals(
            [
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with limit=0 returns all entries (backward compatibility).
     */
    public function test_read_log_unbounded_returns_all(): void {
        $model = new SystemLog();

        # Create mock log files
        file_put_contents('/tmp/test_unbounded.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_unbounded.log.0', "Line 4\nLine 5\nLine 6\n");

        # Read with limit=0 (default) should return all entries
        $log = $model->read_log('/tmp/test_unbounded.log');

        $this->assert_equals(
            [
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with a limit larger than total entries returns all entries.
     */
    public function test_read_log_bounded_exceeds_total(): void {
        $model = new SystemLog();

        # Create mock log files with only 6 total lines
        file_put_contents('/tmp/test_exceed.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_exceed.log.0', "Line 1\nLine 2\nLine 3\n");

        # Read with limit=100 should return all 6 entries (not error)
        $log = $model->read_log('/tmp/test_exceed.log', limit: 100);

        $this->assert_equals(
            [
                ['text' => 'Line 1'],
                ['text' => 'Line 2'],
                ['text' => 'Line 3'],
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with limit=1 returns only the single newest entry.
     */
    public function test_read_log_bounded_single_line(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_single.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_single.log.0', "Line 1\nLine 2\nLine 3\n");

        # Read with limit=1 should return only the newest line
        $log = $model->read_log('/tmp/test_single.log', limit: 1);

        $this->assert_equals([['text' => 'Line 6']], $log);
    }

    /**
     * Checks that read_log() handles an empty current log file gracefully.
     */
    public function test_read_log_bounded_empty_current(): void {
        $model = new SystemLog();

        # Empty current log, rotated file has data
        file_put_contents('/tmp/test_empty.log', '');
        file_put_contents('/tmp/test_empty.log.0', "Line 1\nLine 2\nLine 3\n");

        # Bounded read should fall through to the rotated file
        $log = $model->read_log('/tmp/test_empty.log', limit: 2);

        $this->assert_equals([['text' => 'Line 2'], ['text' => 'Line 3']], $log);
    }

    /**
     * Checks that read_log() handles mixed compression types in rotated logs.
     */
    public function test_read_log_bounded_mixed_compression(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_mixed.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_mixed.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_mixed.log.1.gz', gzencode("Line 1\nLine 2\nLine 3\n"));

        # Read with limit=7 should span all three files with mixed formats
        $log = $model->read_log('/tmp/test_mixed.log', limit: 7);

        $this->assert_equals(
            [
                ['text' => 'Line 3'],
                ['text' => 'Line 4'],
                ['text' => 'Line 5'],
                ['text' => 'Line 6'],
                ['text' => 'Line 7'],
                ['text' => 'Line 8'],
                ['text' => 'Line 9'],
            ],
            $log,
        );
    }

    /**
     * Checks that gather_log_filepaths() does not match unrelated files sharing a prefix.
     */
    public function test_read_log_ignores_unrelated_files(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_prefix.log', "Line 1\nLine 2\n");
        # These files share the prefix but are NOT rotated logs - they should be ignored
        file_put_contents('/tmp/test_prefix.logrotate', "unrelated config data\n");
        file_put_contents('/tmp/test_prefix.log_backup', "backup data\n");

        # Unbounded read should only return lines from the actual log file
        $log = $model->read_log('/tmp/test_prefix.log');

        $this->assert_equals([['text' => 'Line 1'], ['text' => 'Line 2']], $log);
    }

    /**
     * Checks that read_log() with offset skips the specified number of newest entries.
     */
    public function test_read_log_with_offset(): void {
        $model = new SystemLog();

        # Create mock log files with 9 total lines
        file_put_contents('/tmp/test_offset.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_offset.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_offset.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with limit=3 and offset=2 should skip the 2 newest and return the next 3
        # Lines in chronological order: 1,2,3,4,5,6,7,8,9
        # Newest 2 are 8,9 - skip those
        # Next 3 are 5,6,7
        $log = $model->read_log('/tmp/test_offset.log', limit: 3, offset: 2);

        $this->assert_equals([['text' => 'Line 5'], ['text' => 'Line 6'], ['text' => 'Line 7']], $log);
    }

    /**
     * Checks that read_log() with offset=0 behaves the same as no offset.
     */
    public function test_read_log_with_zero_offset(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_zero_offset.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_zero_offset.log.0', "Line 1\nLine 2\nLine 3\n");

        # Read with offset=0 should return the same as without offset
        $log = $model->read_log('/tmp/test_zero_offset.log', limit: 3, offset: 0);

        $this->assert_equals([['text' => 'Line 4'], ['text' => 'Line 5'], ['text' => 'Line 6']], $log);
    }

    /**
     * Checks that read_log() with offset larger than total entries returns empty array.
     */
    public function test_read_log_offset_exceeds_total(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_offset_exceed.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_offset_exceed.log.0', "Line 1\nLine 2\nLine 3\n");

        # Read with offset=100 (larger than 6 total lines) should return empty
        $log = $model->read_log('/tmp/test_offset_exceed.log', limit: 3, offset: 100);

        $this->assert_equals([], $log);
    }

    /**
     * Checks that read_log() pagination works across multiple rotated files.
     */
    public function test_read_log_offset_spans_files(): void {
        $model = new SystemLog();

        # Create mock log files with 9 total lines, 3 per file
        file_put_contents('/tmp/test_offset_span.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_offset_span.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_offset_span.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with limit=2 and offset=5 should skip 5 newest (5,6,7,8,9) and return 2 (3,4)
        $log = $model->read_log('/tmp/test_offset_span.log', limit: 2, offset: 5);

        $this->assert_equals([['text' => 'Line 3'], ['text' => 'Line 4']], $log);
    }

    /**
     * Checks that read_log() with offset works correctly with compressed files.
     */
    public function test_read_log_offset_with_compressed(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_offset_gz.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_offset_gz.log.0.gz', gzencode("Line 4\nLine 5\nLine 6\n"));
        file_put_contents('/tmp/test_offset_gz.log.1.gz', gzencode("Line 1\nLine 2\nLine 3\n"));

        # Read with limit=3 and offset=3 should skip 3 newest (7,8,9) and return next 3 (4,5,6)
        $log = $model->read_log('/tmp/test_offset_gz.log', limit: 3, offset: 3);

        $this->assert_equals([['text' => 'Line 4'], ['text' => 'Line 5'], ['text' => 'Line 6']], $log);
    }

    /**
     * Checks that read_log() with offset handles the edge case where limit+offset equals total entries.
     */
    public function test_read_log_offset_exact_boundary(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_offset_exact.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_offset_exact.log.0', "Line 1\nLine 2\nLine 3\n");

        # Total is 6, limit=3, offset=3 should return exactly the 3 oldest (1,2,3)
        $log = $model->read_log('/tmp/test_offset_exact.log', limit: 3, offset: 3);

        $this->assert_equals([['text' => 'Line 1'], ['text' => 'Line 2'], ['text' => 'Line 3']], $log);
    }

    /**
     * Checks that read_log() with offset handles single line pagination.
     */
    public function test_read_log_offset_single_line(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_offset_single.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_offset_single.log.0', "Line 1\nLine 2\nLine 3\n");

        # Read line by line using offset for pagination
        $page1 = $model->read_log('/tmp/test_offset_single.log', limit: 1, offset: 0);
        $page2 = $model->read_log('/tmp/test_offset_single.log', limit: 1, offset: 1);
        $page3 = $model->read_log('/tmp/test_offset_single.log', limit: 1, offset: 5);

        $this->assert_equals([['text' => 'Line 6']], $page1);
        $this->assert_equals([['text' => 'Line 5']], $page2);
        $this->assert_equals([['text' => 'Line 1']], $page3);
    }

    /**
     * Checks that read_log() with reverse=true returns entries in reverse order (oldest entries first, displayed newest-first).
     */
    public function test_read_log_reverse_basic(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_reverse.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with reverse=true, limit=3 should return the 3 OLDEST entries, displayed in reverse order
        # Oldest 3 are 1,2,3 - reversed for display: 3,2,1
        $log = $model->read_log('/tmp/test_reverse.log', limit: 3, offset: 0, reverse: true);

        $this->assert_equals([['text' => 'Line 3'], ['text' => 'Line 2'], ['text' => 'Line 1']], $log);
    }

    /**
     * Checks that read_log() with reverse=true and offset skips oldest entries.
     */
    public function test_read_log_reverse_with_offset(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_offset.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_reverse_offset.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse_offset.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with reverse=true, limit=3, offset=2 should skip the 2 OLDEST (1,2) and return next 3 (3,4,5)
        # Displayed in reverse order: 5,4,3
        $log = $model->read_log('/tmp/test_reverse_offset.log', limit: 3, offset: 2, reverse: true);

        $this->assert_equals([['text' => 'Line 5'], ['text' => 'Line 4'], ['text' => 'Line 3']], $log);
    }

    /**
     * Checks that read_log() with reverse=true paginates correctly across files.
     */
    public function test_read_log_reverse_spans_files(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_span.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_reverse_span.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse_span.log.1', "Line 1\nLine 2\nLine 3\n");

        # Read with reverse=true, limit=4, offset=1 should skip oldest 1 (Line 1) and return next 4 (2,3,4,5)
        # Displayed in reverse order: 5,4,3,2
        $log = $model->read_log('/tmp/test_reverse_span.log', limit: 4, offset: 1, reverse: true);

        $this->assert_equals(
            [['text' => 'Line 5'], ['text' => 'Line 4'], ['text' => 'Line 3'], ['text' => 'Line 2']],
            $log,
        );
    }

    /**
     * Checks that read_log() with reverse=true works with compressed files.
     */
    public function test_read_log_reverse_with_compressed(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_gz.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_reverse_gz.log.0.gz', gzencode("Line 4\nLine 5\nLine 6\n"));
        file_put_contents('/tmp/test_reverse_gz.log.1.gz', gzencode("Line 1\nLine 2\nLine 3\n"));

        # Read with reverse=true, limit=3 should return the 3 OLDEST entries from compressed files
        # Oldest 3 are 1,2,3 - reversed for display: 3,2,1
        $log = $model->read_log('/tmp/test_reverse_gz.log', limit: 3, offset: 0, reverse: true);

        $this->assert_equals([['text' => 'Line 3'], ['text' => 'Line 2'], ['text' => 'Line 1']], $log);
    }

    /**
     * Checks that read_log() with reverse=true and offset exceeding total returns empty.
     */
    public function test_read_log_reverse_offset_exceeds_total(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_exceed.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse_exceed.log.0', "Line 1\nLine 2\nLine 3\n");

        # Offset exceeds total entries (6), should return empty
        $log = $model->read_log('/tmp/test_reverse_exceed.log', limit: 3, offset: 100, reverse: true);

        $this->assert_equals([], $log);
    }

    /**
     * Checks that read_log() with reverse=true returns all entries when limit exceeds total.
     */
    public function test_read_log_reverse_limit_exceeds_total(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_limit.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse_limit.log.0', "Line 1\nLine 2\nLine 3\n");

        # Limit exceeds total (6 entries), should return all in reverse order
        $log = $model->read_log('/tmp/test_reverse_limit.log', limit: 100, offset: 0, reverse: true);

        $this->assert_equals(
            [
                ['text' => 'Line 6'],
                ['text' => 'Line 5'],
                ['text' => 'Line 4'],
                ['text' => 'Line 3'],
                ['text' => 'Line 2'],
                ['text' => 'Line 1'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with reverse=true and no limit/offset reverses all entries.
     */
    public function test_read_log_reverse_unbounded(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_unbound.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse_unbound.log.0', "Line 1\nLine 2\nLine 3\n");

        # Unbounded with reverse=true should return all entries in reverse order (newest first)
        $log = $model->read_log('/tmp/test_reverse_unbound.log', limit: 0, offset: 0, reverse: true);

        $this->assert_equals(
            [
                ['text' => 'Line 6'],
                ['text' => 'Line 5'],
                ['text' => 'Line 4'],
                ['text' => 'Line 3'],
                ['text' => 'Line 2'],
                ['text' => 'Line 1'],
            ],
            $log,
        );
    }

    /**
     * Checks that read_log() with reverse=true single line pagination works correctly.
     */
    public function test_read_log_reverse_single_line(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_reverse_single.log', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_reverse_single.log.0', "Line 1\nLine 2\nLine 3\n");

        # Read line by line from the oldest end using offset for pagination
        $page1 = $model->read_log('/tmp/test_reverse_single.log', limit: 1, offset: 0, reverse: true);
        $page2 = $model->read_log('/tmp/test_reverse_single.log', limit: 1, offset: 1, reverse: true);
        $page3 = $model->read_log('/tmp/test_reverse_single.log', limit: 1, offset: 5, reverse: true);

        $this->assert_equals([['text' => 'Line 1']], $page1);
        $this->assert_equals([['text' => 'Line 2']], $page2);
        $this->assert_equals([['text' => 'Line 6']], $page3);
    }

    /**
     * Checks that read_log() correctly handles the boundary between reverse and normal pagination.
     * This verifies that the two modes are complementary - paginating through the entire log.
     */
    public function test_read_log_pagination_consistency(): void {
        $model = new SystemLog();

        file_put_contents('/tmp/test_consistency.log', "Line 7\nLine 8\nLine 9\n");
        file_put_contents('/tmp/test_consistency.log.0', "Line 4\nLine 5\nLine 6\n");
        file_put_contents('/tmp/test_consistency.log.1', "Line 1\nLine 2\nLine 3\n");

        # Normal mode (reverse=false): paginate from newest
        # Page 1: limit=3, offset=0 -> 7,8,9 (newest 3)
        # Page 2: limit=3, offset=3 -> 4,5,6
        # Page 3: limit=3, offset=6 -> 1,2,3 (oldest 3)
        $normal_page1 = $model->read_log('/tmp/test_consistency.log', limit: 3, offset: 0, reverse: false);
        $normal_page2 = $model->read_log('/tmp/test_consistency.log', limit: 3, offset: 3, reverse: false);
        $normal_page3 = $model->read_log('/tmp/test_consistency.log', limit: 3, offset: 6, reverse: false);

        $this->assert_equals([['text' => 'Line 7'], ['text' => 'Line 8'], ['text' => 'Line 9']], $normal_page1);
        $this->assert_equals([['text' => 'Line 4'], ['text' => 'Line 5'], ['text' => 'Line 6']], $normal_page2);
        $this->assert_equals([['text' => 'Line 1'], ['text' => 'Line 2'], ['text' => 'Line 3']], $normal_page3);

        # Reverse mode (reverse=true): paginate from oldest
        # Page 1: limit=3, offset=0 -> 1,2,3 (oldest 3, reversed: 3,2,1)
        # Page 2: limit=3, offset=3 -> 4,5,6 (reversed: 6,5,4)
        # Page 3: limit=3, offset=6 -> 7,8,9 (reversed: 9,8,7)
        $reverse_page1 = $model->read_log('/tmp/test_consistency.log', limit: 3, offset: 0, reverse: true);
        $reverse_page2 = $model->read_log('/tmp/test_consistency.log', limit: 3, offset: 3, reverse: true);
        $reverse_page3 = $model->read_log('/tmp/test_consistency.log', limit: 3, offset: 6, reverse: true);

        $this->assert_equals([['text' => 'Line 3'], ['text' => 'Line 2'], ['text' => 'Line 1']], $reverse_page1);
        $this->assert_equals([['text' => 'Line 6'], ['text' => 'Line 5'], ['text' => 'Line 4']], $reverse_page2);
        $this->assert_equals([['text' => 'Line 9'], ['text' => 'Line 8'], ['text' => 'Line 7']], $reverse_page3);
    }
}
