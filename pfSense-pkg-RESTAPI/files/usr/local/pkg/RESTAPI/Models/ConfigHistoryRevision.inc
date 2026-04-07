<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\IntegerField;
use RESTAPI\Fields\StringField;
use RESTAPI\Fields\UnixTimeField;

/**
 * Defines a Model that interacts with the pfSense configuration history.
 */
class ConfigHistoryRevision extends Model {
    public UnixTimeField $time;
    public StringField $description;
    public StringField $version;
    public IntegerField $filesize;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_config_history';
        $this->verbose_name = 'Configuration History Entry';
        $this->many = true;

        # Set model fields
        $this->time = new UnixTimeField(required: true, verbose_name: 'Time', help_text: 'The time the configuration change was made.');
        $this->description = new StringField(default: '', verbose_name: 'Description', help_text: 'The description of the configuration change.');
        $this->version = new StringField(
            default: '',
            verbose_name: 'Version',
            help_text: 'The configuration version associated with this change.',
        );
        $this->filesize = new IntegerField(
            default: 0,
            verbose_name: 'File Size',
            help_text: 'The file size (in bytes) of the configuration file associated with this change.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the configuration history as an array. This method is the internal callable for this Model.
     * @return array The configuration history as an array of objects.
     */
    protected function get_config_history(): array {
        # Get our current configuration history, but remove the versions key. These are not needed for the Model.
        $config_history = get_backups();
        unset($config_history['versions']);

        # Loop through each entry in the configuration history and normalize the data
        foreach ($config_history as &$entry) {
            # If filesize is not a numeric, set it to the default
            if (!is_numeric($entry['filesize'])) {
                $entry['filesize'] = $this->filesize->default;
            }
        }

        return $config_history;
    }

    /**
     * Deletes a configuration history entry.
     */
    public function _delete(): void {
        unlink("/cf/conf/backup/config-{$this->time->value}.xml");
    }
}
