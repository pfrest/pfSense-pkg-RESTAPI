<?php

namespace RESTAPI\Models;

require_once 'RESTAPI/autoloader.inc';

use RESTAPI\Core\Model;
use RESTAPI\Fields\ForeignModelField;
use RESTAPI\Fields\StringField;
use RESTAPI\ModelTraits\OpenVPNClientExportTraits;
use RESTAPI\Responses\ConflictError;
use RESTAPI\Responses\NotAcceptableError;
use RESTAPI\Responses\NotFoundError;
use RESTAPI\Responses\ServerError;
use RESTAPI\Responses\ValidationError;

/**
 * Defines a Model that represents an OpenVPN Client Export. This Model is responsible for generating the actual
 * client export files. It does not define or store default client export configurations in the pfSense
 * configuration.
 */
class OpenVPNClientExport extends Model {
    # Inherit shared traits (fields and methods) from the OpenVPNClientExportTraits trait.
    # This allows the OpenVPNClientExportConfig and OpenVPNClientExport Model classes to share
    # the same fields since they are identical in structure, but serve two distinct purposes.
    use OpenVPNClientExportTraits;

    public StringField $type;
    public ForeignModelField $certref;
    public ForeignModelField $username;
    public StringField $filename;
    public StringField $binary_data;

    const USETOKEN_FORBIDDEN_TYPES = [
        'confzip',
        'conf_yealink_t28',
        'conf_yealink_t38g',
        'conf_yealink_t38g2',
        'conf_snom',
        'confinline',
        'confinlinedroid',
        'confinlineconnect',
        'confinlinevisc',
    ];

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Set model attributes
        $this->config_path = 'installedpackages/vpn_openvpn_export/serverconfig/item';
        $this->packages = ['pfSense-pkg-openvpn-client-export'];
        $this->package_includes = ['openvpn-client-export.inc'];
        $this->verbose_name = 'OpenVPN Client Export';
        $this->auto_create_id = false; // Create is used to initiate the export process, not creating new objects
        $this->many = true;

        # Setup shared fields defined in OpenVPNClientExportTraits
        $this->__setup_fields();

        # Set fields specific to this model
        $this->type = new StringField(
            required: true,
            choices: [
                'confzip',
                'conf_yealink_t28',
                'conf_yealink_t38g',
                'conf_yealink_t38g2',
                'conf_snom',
                'confinline',
                'confinlinedroid',
                'confinlineconnect',
                'confinlinevisc',
                'inst-Win7',
                'inst-Win10',
                'inst-x86-previous',
                'inst-x64-previous',
                'inst-x86-current',
                'inst-x64-current',
                'visc',
            ],
            help_text: 'The type of OpenVPN client export to generate. This determines the format and content of ' .
                'the export file.',
        );
        $this->username = new ForeignModelField(
            model_name: 'User',
            model_field: 'name',
            default: null,
            allow_null: true,
            help_text: 'The username of the user this client export corresponds to. This is only applicable ' .
                'for OpenVPN servers that use the Local Database AND client certificates.',
        );
        $this->certref = new ForeignModelField(
            model_name: 'Certificate',
            model_field: 'refid',
            default: null,
            allow_null: true,
            help_text: 'The reference ID of the certificate to use for this OpenVPN client export. This is only ' .
                'applicable for OpenVPN servers that require client certificates.',
        );
        $this->filename = new StringField(
            required: false,
            default: null,
            allow_null: true,
            read_only: true,
            help_text: 'The filename used when exporting the OpenVPN client export. This value cannot be changed',
        );
        $this->binary_data = new StringField(
            required: false,
            default: '',
            allow_empty: true,
            read_only: true,
            help_text: 'The binary data of the OpenVPN client export. This is used to store the actual ' .
                'exported configuration file content. When the content-type is set to "application/octet-stream", ' .
                'this field will contain the data of the OpenVPN client export download.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Performs extra validation on the server field. This is primarily intended to ensure that the requested
     * export parameters are compatible with the OpenVPN server settings.
     * @param int $server The OpenVPN server ID to validate.
     * @return int The validated OpenVPN server ID.
     * @throws ValidationError If the server mode requires a certificate reference but none is provided,
     * @throws ValidationError If the server mode requires a username but none is provided.
     */
    public function validate_server(int $server): int {
        # Obtain the server object
        $server_obj = $this->server->get_related_model();

        # If this server supports client certs, require certref to be set.
        if (in_array($server_obj->mode->value, ['server_tls', 'server_tls_user']) and !$this->certref->value) {
            throw new ValidationError(
                message: "Field 'certref' is required for OpenVPN server mode '{$server_obj->mode->value}'.",
                response_id: 'OPENVPN_CLIENT_EXPORT_CERTREF_REQUIRED',
            );
        }

        # Require a username to be set if the server is only using the Local Database for authentication.
        if ($server_obj->authmode->value === ['Local Database'] and !$this->username->value) {
            throw new ValidationError(
                message: "Field 'username' is required for OpenVPN server mode '{$server_obj->mode->value}' " .
                    "with authentication mode 'Local Database'.",
                response_id: 'OPENVPN_CLIENT_EXPORT_USERNAME_REQUIRED',
            );
        }

        return $server;
    }

    /**
     * Perform extra validation to the type field. This is primarily used to ensure that certain parameters are set
     * or not set when a specific type of export is requested.
     * @param string $type The type of OpenVPN client export to validate.
     * @return string The validated type of OpenVPN client export.
     * @throws ValidationError If the type is not compatible with the 'usetoken' field.
     */
    public function validate_type(string $type): string {
        # Do not allow confinline types to have 'usetoken' enabled
        if (in_array($type, self::USETOKEN_FORBIDDEN_TYPES) and $this->usetoken->value) {
            throw new ConflictError(
                message: "Field 'usetoken' cannot be enabled for OpenVPN client export type '$type'.",
                response_id: 'OPENVPN_CLIENT_EXPORT_TYPE_USETOKEN_NOT_ALLOWED',
            );
        }

        return $type;
    }

    /**
     * Perform extra validation to the certref field. This is primarily intended to ensure the specified user is the
     * owner of the certificate being referenced.
     * @param string $certref The certificate reference ID to validate.
     * @return string The validated certificate reference ID.
     * @throws NotFoundError If the specified certificate reference ID does not belong to the user.
     * @throws NotAcceptableError If the certificate does not have a private key associated with it and is required
     */
    public function validate_certref(string $certref): string {
        # Skip this validation if no username is specified.
        if (!$this->username->value) {
            return $certref;
        }

        # Ensure the certref is listed in the user's certificates.
        if (!in_array($certref, $this->username->get_related_model()->cert->value)) {
            throw new NotFoundError(
                message: "User '{$this->username->value}' is not assigned a certificate with reference ID '$certref'.",
                response_id: 'OPENVPN_CLIENT_EXPORT_USER_CERT_NOT_FOUND',
            );
        }

        # Do not allow the referenced cert to not have a private key unless usepkcs11 or usetoken is set
        $cert_obj = $this->certref->get_related_model();
        if (!$cert_obj->prv->value and (!$this->usepkcs11->value or !$this->usetoken->value)) {
            throw new ValidationError(
                message: "Certificate with reference ID '$certref' does not have a private key. " .
                    'Ensure the certificate is valid and has a private key.',
                response_id: 'OPENVPN_CLIENT_EXPORT_CERT_NO_PRIVATE_KEY',
            );
        }

        return $certref;
    }

    /**
     * Generates the OpenVPN client export configuration file based on the provided parameters.
     * @throws ServerError If the export fails for an unknown reason
     */
    protected function _create(): void {
        global $input_errors;

        # Generate a the file export based on the export type
        $export_data = null;
        if (str_starts_with($this->type->value, 'conf')) {
            $export_data = $this->export_config();
        }
        if (str_starts_with($this->type->value, 'inst-')) {
            $export_data = $this->export_installer();
        }
        if ($this->type->value === 'visc') {
            $export_data = $this->export_viscosity();
        }

        # If import errors were found during the export, raise an error
        if ($input_errors) {
            throw new ServerError(
                message: "The OpenVPN client export failed for the following reason: $input_errors[0]",
                response_id: 'OPENVPN_CLIENT_EXPORT_CREATION_FAILED_FOR_KNOWN_REASON',
            );
        }

        # If no valid filepath was given after generating, we know the export failed. Throw an error.
        if (!$export_data) {
            throw new ServerError(
                message: 'The OpenVPN client export could not be created for unknown reasons.',
                response_id: 'OPENVPN_CLIENT_EXPORT_CREATION_FAILED_FOR_UNKNOWN_REASON',
            );
        }

        # When the export data is a filepath, set the binary data to its contents
        if (is_file($export_data)) {
            $this->filename->value = $this->__get_export_filename();
            $this->binary_data->value = file_get_contents($export_data);
            @unlink($export_data);
            return;
        }

        # Otherwise, just use the value directly
        $this->binary_data->value = $export_data;
    }

    /**
     * Generates the OpenVPN client export configuration file based on this objects properties.
     * @return string|null The filepath to the generate OpenVPN client export configuration file, or null if the
     * export failed.
     * @note If a confinline type is used, the configuration file will be returned as a string instead of a filepath
     */
    public function export_config(): string|null {
        return openvpn_client_export_config(
            srvid: $this->server->value,
            usrid: $this->username->value ? $this->username->get_related_model()->id : null,
            crtid: $this->locate_crtid(),
            useaddr: $this->useaddr_hostname->value ?? $this->useaddr->value,
            verifyservercn: $this->verifyservercn->value,
            blockoutsidedns: $this->blockoutsidedns->value,
            legacy: $this->legacy->value,
            bindmode: $this->bindmode->value,
            usetoken: $this->usetoken->value,
            nokeys: $this->server->get_related_model()->mode->value === 'server_user',
            proxy: $this->__get_proxy_config(),
            expformat: str_replace(['conf_', 'conf'], '', $this->type->value),
            outpass: $this->pass->value,
            p12encryption: $this->p12encryption->value,
            skiptls: false,
            doslines: false,
            advancedoptions: $this->advancedoptions->value,
            usepkcs11: $this->usepkcs11->value,
            pkcs11providers: $this->pkcs11providers->value,
            pkcs11id: $this->pkcs11id->value,
        );
    }

    /**
     * Exports the OpenVPN client export installer file based on the provided parameters.
     * @return string|null The filepath to the generated OpenVPN client export installer file, or null if the
     * export failed.
     */
    public function export_installer(): string|null {
        # Ensure legacy incompatible ciphers are always an array
        global $legacy_incompatible_ciphers;
        $legacy_incompatible_ciphers = $legacy_incompatible_ciphers ?? [];

        return openvpn_client_export_installer(
            srvid: $this->server->value,
            usrid: $this->username->value ? $this->username->get_related_model()->id : null,
            crtid: $this->locate_crtid(),
            useaddr: $this->useaddr_hostnam->value ?? $this->useaddr->value,
            verifyservercn: $this->verifyservercn->value,
            blockoutsidedns: $this->blockoutsidedns->value,
            legacy: $this->legacy->value,
            bindmode: $this->bindmode->value,
            usetoken: $this->usetoken->value,
            outpass: $this->pass->value,
            p12encryption: $this->p12encryption->value,
            proxy: $this->__get_proxy_config(),
            advancedoptions: $this->advancedoptions->value,
            openvpn_version: substr($this->type->value, 5),
            usepkcs11: $this->usepkcs11->value,
            pkcs11providers: $this->pkcs11providers->value,
            pkcs11id: $this->pkcs11id->value,
            silent: $this->silent->value,
        );
    }

    /**
     * Exports the OpenVPN client export configuration file for Viscosity based on the provided parameters.
     * @return string|null The filepath to the generated OpenVPN client export configuration file for Viscosity,
     * or null if the export failed.
     */
    public function export_viscosity(): string|null {
        return viscosity_openvpn_client_config_exporter(
            srvid: $this->server->value,
            usrid: $this->username->value ? $this->username->get_related_model()->id : null,
            crtid: $this->locate_crtid(),
            useaddr: $this->useaddr_hostname->value ?? $this->useaddr->value,
            verifyservercn: $this->verifyservercn->value,
            blockoutsidedns: $this->blockoutsidedns->value,
            legacy: $this->legacy->value,
            bindmode: $this->bindmode->value,
            usetoken: $this->usetoken->value,
            outpass: $this->pass->value,
            p12encryption: $this->p12encryption->value,
            proxy: $this->__get_proxy_config(),
            advancedoptions: $this->advancedoptions->value,
            usepkcs11: $this->usepkcs11->value,
            pkcs11providers: $this->pkcs11providers->value,
            pkcs11id: $this->pkcs11id->value,
        );
    }

    /**
     * Obtains the crtid value pfSense expects for a given certref given the current OpenVPN server and username.
     * This is necessary because pfSense's openvpn_client_export_validate_config function sometimes expects
     * crtid to be the index of the 'cert' config path, and sometimes expects it to be the index of the
     * 'system/user/{$usrid}/cert' config path.
     * @returns int|null The crtid value pfSense expects for a given certref, or null if no certref is set.
     */
    public function locate_crtid(): int|null {
        # If no certref is set, return null
        if (!$this->certref->value) {
            return null;
        }

        # Load related models
        $ovpnsrv = $this->server->get_related_model();
        $user = $this->username->get_related_model();
        $cert = $this->certref->get_related_model();

        # If a username was provided, the server is using server_tls_user and the local database, pfSense expects
        # the crtid to be the index of the 'system/user/{$usrid}/cert' config path not the 'cert' config path.
        if (
            $user and
            $ovpnsrv->mode->value === 'server_tls_user' and
            $ovpnsrv->authmode->value === ['Local Database']
        ) {
            foreach ($user->cert->value as $idx => $user_cert) {
                if ($user_cert === $cert->refid->value) {
                    return $idx;
                }
            }
        }

        # Otherwise, just return the index of the certificate directly
        return $cert->id;
    }

    /**
     * Obtains the proxy configuration array expected by pfSense functions.
     * @return array the proxy configuration array expected by pfSense functions
     */
    private function __get_proxy_config(): array {
        return $this->useproxy->value
            ? [
                'ip' => $this->proxyaddr->value,
                'port' => $this->proxyport->value,
                'user' => $this->proxyuser->value,
                'password' => $this->proxypass->value,
                'proxy_type' => $this->useproxytype->value,
                'proxy_authtype' => $this->useproxypass->value,
            ]
            : [];
    }

    private function __get_export_filename(): string {
        global $current_openvpn_version, $current_openvpn_version_rev;
        global $previous_openvpn_version, $previous_openvpn_version_rev;
        global $legacy_openvpn_version, $legacy_openvpn_version_rev;

        # Obtain the filename prefix
        $filename_prefix = openvpn_client_export_prefix(
            srvid: $this->server->value,
            usrid: $this->username->value ? $this->username->get_related_model()->id : null,
            crtid: $this->certref->value ? $this->certref->get_related_model()->id : null,
        );

        # Determine the conf file suffix based on the export type
        if (str_starts_with($this->type->value, 'conf')) {
            $filename_suffix = match ($this->type->value) {
                'confzip' => '-config.zip',
                'conf_yealink_t38g', 'conf_yealink_t28', 'conf_yealink_t38g2' => 'client.tar',
                'conf_snom' => 'vpnclient.tar',
                'confinlinedroid' => '-android-config.ovpn',
                'confinlineconnect' => '-connect-config.ovpn',
                'confinlinevisc' => '-viscosity-config.ovpn',
                default => '-config.ovpn',
            };
        }
        # Determine the installer file suffix based on the export type
        elseif (str_starts_with($this->type->value, 'inst-')) {
            $filename_suffix = match ($this->type->value) {
                'inst-Win7' => "$legacy_openvpn_version}-I$legacy_openvpn_version_rev-Win7.exe",
                'inst-Win10' => "$legacy_openvpn_version-I$legacy_openvpn_version_rev-Win10.exe",
                'inst-x86-previous' => "$previous_openvpn_version-I$previous_openvpn_version_rev-x86.exe",
                'inst-x64-previous' => "$previous_openvpn_version-I$previous_openvpn_version_rev-amd64.exe",
                'inst-x86-current' => "$current_openvpn_version-I$current_openvpn_version_rev-x86.exe",
                default => "$current_openvpn_version-I$current_openvpn_version_rev-amd64.exe",
            };
        }
        # If the type is not recognized, throw an error
        else {
            throw new ServerError(
                message: 'Could not determine appropriate file name.',
                response_id: 'OPENVPN_CLIENT_EXPORT_FILENAME_NOT_DETERMINED',
            );
        }

        return urlencode($filename_prefix . $filename_suffix);
    }
}
