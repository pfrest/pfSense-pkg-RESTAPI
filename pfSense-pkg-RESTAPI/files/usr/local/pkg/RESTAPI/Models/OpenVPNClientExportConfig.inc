<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\ModelTraits\OpenVPNClientExportTraits;

/**
 * Defines a Model that represents an OpenVPN Client Export default config for specific OpenVPN servers.
 */
class OpenVPNClientExportConfig extends Model {
    # Inherit shared traits (fields and methods) from the OpenVPNClientExportTraits trait.
    # This allows the OpenVPNClientExportConfig and OpenVPNClientExport Model classes to share
    # the same fields since they are identical in structure, but serve two distinct purposes.
    use OpenVPNClientExportTraits;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/vpn_openvpn_export/serverconfig/item';
        $this->packages = ['pfSense-pkg-openvpn-client-export'];
        $this->package_includes = ['openvpn-client-export.inc'];
        $this->always_apply = true;
        $this->many = true;
        $this->verbose_name = 'OpenVPN Client Export Config';

        # Setup shared fields defined in OpenVPNClientExportTraits
        $this->__setup_fields();

        parent::__construct($id, $parent_id, $data, ...$options);
    }
}
