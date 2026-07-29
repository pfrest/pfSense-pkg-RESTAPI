<?php
//   Copyright 2023 Jared Hendrickson
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.

require_once("api/framework/APIModel.inc");
require_once("api/framework/APIResponse.inc");


class APISystemAPISyncUpdate extends APIModel {
    public $pkg_index;
    public $pkg_conf;
    # Create our method constructor
    public function __construct() {
        parent::__construct();
        $this->change_note = "Synced API configuration from ".$this->client->ip_address;
        $this->privileges = ["page-all"];
        $this->set_auth_mode = "local";
        $this->retain_read_mode = false;
        $this->ignore_enabled = true;
        $this->ignore_ifs = true;
        $this->pkg_index = APITools\get_api_config()[0];
        $this->pkg_conf = APITools\get_api_config()[1];
    }

    public function action() {
        $this->set_config("installedpackages/package/{$this->pkg_index}/conf", $this->validated_data);
        $this->write_config();
        $this->backup();
        return APIResponse\get(0, $this->validated_data);
    }

    public function validate_payload() {
        # Do not sync API HA sync settings or authentication data
        unset($this->initial_data["hasync"]);
        unset($this->initial_data["hasync_hosts"]);
        unset($this->initial_data["hasync_username"]);
        unset($this->initial_data["hasync_password"]);

        # Require data to include at least a server key and auth mode
        # NOTE: This model is not intended for public use, input is created by the master node and is generally trusted
        if (isset($this->initial_data["server_key"]) and isset($this->initial_data["authmode"])) {
            $this->validated_data = $this->initial_data;
        } else {
            $this->errors[] = APIResponse\get(1);
        }
    }

    public function backup() {
        if (isset($this->pkg_conf["persist"])) {
            shell_exec("/usr/local/share/pfSense-pkg-API/manage.php backup");
        }
    }
}
