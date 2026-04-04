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
}
