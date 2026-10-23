<?php

/**
 * The pfSense-pkg-RESTAPI's 'plugin_nginx' package hook. This function is automatically called by the pfSense
 * package system to add custom NGINX configurations. For this package, this function ensures an nginx server block
 * is defined for API endpoints to allow the utilization of additional HTTP methods like PUT, PATCH and DELETE and
 * URLs without a trailing slash.
 * @param mixed $pluginparams Plugin parameters passed in by the pfSense package system.
 * @return string The custom nginx block to be added to the webConfigurator's nginx.conf file
 */
function restapi_plugin_nginx(mixed $pluginparams): string {
    global $g;

    # Caller did not properly define data, exit early
    if (!is_array($pluginparams) || empty($pluginparams) || empty($pluginparams['section'])) {
        return '';
    }

    return match ($pluginparams['section']) {
        'location' => "
            # This block is required to allow URI's without trailing slash or .php extension.
            # This block also enables PHP to handle PATCH and DELETE requests for API functionality.
            location ^~ /api/v2/ {
                index index.php index.html index.htm;
                try_files \$uri/index.php =404; #  This line closes a potential security hole
                fastcgi_pass   unix:{$g['varrun_path']}/php-fpm.socket;
                fastcgi_index  index.php;
                fastcgi_param  SCRIPT_FILENAME  \$document_root\$fastcgi_script_name;
                fastcgi_param  HTTP_PROXY  \"\";
                fastcgi_param  HTTP_AUTHORIZATION  \$http_authorization;
                fastcgi_read_timeout 180;
                include        /usr/local/etc/nginx/fastcgi_params;
            }",
        default => '',
    };
}
