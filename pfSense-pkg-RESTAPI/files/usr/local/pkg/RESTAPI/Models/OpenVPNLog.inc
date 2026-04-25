<?php

namespace RESTAPI\Models;

use RESTAPI\Core\Model;
use RESTAPI\Fields\StringField;
use RESTAPI\ModelTraits\LogFileModelTraits;

/**
 * Defines a Model for interacting with the OpenVPN log at /var/log/openvpn.log.
 */
class OpenVPNLog extends Model {
    use LogFileModelTraits;

    /**
     * @var string $log_file The path to the base openvpn log file this Model will read. Any log files that have been rotated
     * will be read as well.
     */
    public string $log_file = '/var/log/openvpn.log';
    public StringField $text;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], ...$options) {
        # Set model attributes
        $this->internal_callable = 'get_openvpn_log';
        $this->many = true;
        $this->verbose_name = 'OpenVPN Log';

        $this->text = new StringField(
            default: '',
            verbose_name: 'Text',
            help_text: 'The raw text of the openvpn log entry.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Obtains the openvpn log as an array. This method is the internal callable for this Model.
     * @param int $limit Maximum entries to return (0 = all). Passed through from pagination.
     * @param int $offset Number of entries to skip. Passed through from pagination.
     * @param bool $reverse Whether to read from oldest entries first.
     * @return array The openvpn log as an array of objects.
     */
    protected function get_openvpn_log(int $limit = 0, int $offset = 0, bool $reverse = false): array {
        return $this->read_log($this->log_file, $limit, $offset, $reverse);
    }
}
