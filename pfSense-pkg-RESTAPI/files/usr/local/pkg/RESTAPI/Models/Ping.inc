<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model for running ping diagnostics from pfSense.
 */
class Ping extends Model {
    public StringField $host;
    public IntegerField $count;
    public StringField $source_address;
    public StringField $output;
    public IntegerField $result_code;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define Model attributes
        $this->verbose_name = 'Ping';

        # Define Model Fields
        $this->host = new StringField(
            required: true,
            write_only: true,
            verbose_name: 'Host',
            help_text: 'The IP address or hostname to ping.',
        );
        $this->count = new IntegerField(
            default: 3,
            minimum: 1,
            maximum: 10,
            write_only: true,
            verbose_name: 'Count',
            help_text: 'The number of ping requests to send.',
        );
        $this->source_address = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            verbose_name: 'Source Address',
            help_text: 'The source IP address to use for ping requests.',
        );
        $this->output = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Output',
            help_text: 'The output from the ping command.',
        );
        $this->result_code = new IntegerField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Result Code',
            help_text: 'The result code from the ping command. 0 indicates success.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Execute the ping command and populate the output.
     */
    public function _create(): void {
        # Build the ping command
        $host = escapeshellarg($this->host->value);
        $count = intval($this->count->value);
        $cmd_str = "ping -c {$count}";

        # Add source address if specified
        if (!empty($this->source_address->value)) {
            $source = escapeshellarg($this->source_address->value);
            $cmd_str .= " -S {$source}";
        }

        $cmd_str .= " {$host}";

        # Execute the command
        $cmd = new Command($cmd_str);
        $this->output->value = $cmd->output;
        $this->result_code->value = $cmd->result_code;
    }
}
