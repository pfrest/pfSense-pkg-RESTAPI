<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;

/**
 * Defines a Model object that can be used to run shell commands and retrieve the output and result code.
 */
class CommandPrompt extends Model {
    public StringField $command;
    public StringField $output;
    public IntegerField $result_code;
    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define Model Fields
        $this->command = new StringField(
            required: true,
            write_only: true,
            verbose_name: 'Command',
            help_text: 'The command to be executed.',
        );
        $this->output = new StringField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Output',
            help_text: 'The output of the executed command.',
        );
        $this->result_code = new IntegerField(
            allow_null: true,
            read_only: true,
            verbose_name: 'Result Code',
            help_text: 'The result code of the executed command.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Execute the requested command and populate the output and result code.
     */
    public function _create(): void {
        $cmd = new Command($this->command->value);
        $this->output->value = $cmd->output;
        $this->result_code->value = $cmd->result_code;
    }
}
