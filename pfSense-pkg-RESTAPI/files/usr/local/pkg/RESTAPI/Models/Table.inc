<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;
use RESTAPI\Responses\ServerError;

/**
 * Defines a Model for interacting with pfSense tables.
 */
class Table extends Model {
    public StringField $entries;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_tables';
        $this->id_type = 'string';
        $this->many = true;

        $this->name = new StringField(required: true, verbose_name: 'Name', help_text: 'The name of the table');
        $this->entries = new StringField(many: true, delimiter: ' ', verbose_name: 'Entries', help_text: 'The entries currently in the table.');

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Retrieves the list of available tables from the pfctl command.
     * @return array The list of available table names.
     */
    public function get_available_table_names(): array {
        $table_names_ouptput = new Command('/sbin/pfctl -sT');
        return explode("\n", $table_names_ouptput->output);
    }

    /**
     * Obtains the auth log as an array. This method is the internal callable for this Model.
     * @return array The auth log as an array of objects.
     */
    protected function get_tables(): array {
        $tables = [];

        # Loop through each table and expand its entries
        foreach ($this->get_available_table_names() as $table_name) {
            # Get the entries for the table
            $table_entries_output = new Command(
                '/sbin/pfctl -t ' . escapeshellarg($table_name) . ' -T show',
                trim_whitespace: true,
            );
            $tables[$table_name] = ['name' => $table_name, 'entries' => trim($table_entries_output->output)];
        }

        return $tables;
    }

    /**
     * Overrides the default _delete method to flush the table entries instead.
     */
    protected function _delete(): void {
        # Flush the table entries using pfctl
        $flush_command = new Command('/sbin/pfctl -t ' . escapeshellarg($this->id) . ' -T flush');
        if ($flush_command->result_code !== 0) {
            throw new ServerError(
                message: 'Failed to flush table entries for ' . $this->id . ': ' . $flush_command->output,
                response_id: 'TABLE_FLUSH_FAILED',
            );
        }
    }
}
