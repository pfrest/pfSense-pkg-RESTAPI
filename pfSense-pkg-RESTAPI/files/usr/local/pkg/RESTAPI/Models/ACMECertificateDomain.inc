<?php

namespace RESTAPI\Models;

use RESTAPI;
use RESTAPI\Core\Model;
use RESTAPI\Fields\BooleanField;
use RESTAPI\Fields\StringField;
use RESTAPI\Validators\HostnameValidator;

/**
 * Defines a Model for interacting with ACME certificate configurations.
 */
class ACMECertificateDomain extends Model {
    public StringField $name;
    public StringField $status;
    public StringField $method;
    public StringField $webrootfolder;
    public StringField $webrootftpftpserver;
    public StringField $webrootftpusername;
    public StringField $webrootftppassword;
    public StringField $webrootftpfolder;
    public StringField $standaloneport;
    public BooleanField $standaloneipv6;
    public StringField $standalonetlsport;
    public StringField $nsupdate_server;
    public StringField $nsupdate_keyname;
    public StringField $nsupdate_keyalgo;
    public StringField $nsupdate_key;
    public StringField $nsupdate_zone;
    public StringField $one984hosting_username;
    public StringField $one984hosting_password;
    public StringField $acmeproxy_endpoint;
    public StringField $acmeproxy_username;
    public StringField $acmeproxy_password;
    public StringField $acmedns_username;
    public StringField $acmedns_password;
    public StringField $acmedns_subdomain;
    public StringField $acmedns_update_url;
    public StringField $active24_token;
    public StringField $akamai_host;
    public StringField $akamai_access_token;
    public StringField $akamai_client_token;
    public StringField $akamai_client_secret;
    public StringField $ali_key;
    public StringField $ali_secret;
    public StringField $kas_login;
    public StringField $kas_authtype;
    public StringField $kas_authdata;
    public StringField $ad_api_key;
    public StringField $anx_token;
    public StringField $af_api_username;
    public StringField $af_api_password;
    public StringField $arvan_token;
    public StringField $aurora_key;
    public StringField $aurora_secret;
    public StringField $autodns_user;
    public StringField $autodns_password;
    public StringField $autodns_context;
    public StringField $aws_access_key_id;
    public StringField $aws_secret_access_key;
    public StringField $aws_dns_slowrate;
    public StringField $azion_email;
    public StringField $azion_password;
    public StringField $azuredns_subscriptionid;
    public StringField $azuredns_tenantid;
    public StringField $azuredns_appid;
    public StringField $azuredns_clientsecret;
    public StringField $bookmyname_username;
    public StringField $bookmyname_password;
    public StringField $bunny_api_key;
    public StringField $clouddns_email;
    public StringField $clouddns_client_id;
    public StringField $clouddns_password;
    public StringField $cloudns_auth_id;
    public StringField $cloudns_sub_auth_id;
    public StringField $cloudns_auth_password;
    public StringField $cf_key;
    public StringField $cf_email;
    public StringField $cf_token;
    public StringField $cf_account_id;
    public StringField $cf_zone_id;
    public StringField $conoha_username;
    public StringField $conoha_password;
    public StringField $conoha_tenantid;
    public StringField $conoha_identityserviceapi;
    public StringField $constellix_key;
    public StringField $constellix_secret;
    public StringField $cpanel_username;
    public StringField $cpanel_apitoken;
    public StringField $cpanel_hostname;
    public StringField $cn_user;
    public StringField $cn_password;
    public StringField $curanet_authclientid;
    public StringField $curanet_authsecret;
    public StringField $cy_username;
    public StringField $cy_password;
    public StringField $ddnss_token;
    public StringField $dedyn_token;
    public StringField $dedyn_name;
    public StringField $do_api_key;
    public StringField $da_api;
    public StringField $da_api_insecure;
    public StringField $dnsexit_api_key;
    public StringField $dnsexit_auth_user;
    public StringField $dnsexit_auth_pass;
    public StringField $dnshome_subdomain;
    public StringField $dnshome_subdomainpassword;
    public StringField $dnsimple_oauth_token;
    public StringField $me_key;
    public StringField $me_secret;
    public StringField $dnsservices_username;
    public StringField $dnsservices_password;
    public StringField $do_letoken;
    public StringField $do_pid;
    public StringField $do_pw;
    public StringField $domeneshop_token;
    public StringField $domeneshop_secret;
    public StringField $dp_id;
    public StringField $dp_key;
    public StringField $dpi_id;
    public StringField $dpi_key;
    public StringField $dh_api_key;
    public StringField $duckdns_token;
    public StringField $dd_api_user;
    public StringField $dd_api_key;
    public StringField $dyn_customer;
    public StringField $dyn_username;
    public StringField $dyn_password;
    public StringField $df_user;
    public StringField $df_password;
    public StringField $dynu_clientid;
    public StringField $dynu_secret;
    public StringField $easydns_key;
    public StringField $easydns_token;
    public StringField $euserv_username;
    public StringField $euserv_password;
    public StringField $exoscale_api_key;
    public StringField $exoscale_secret_key;
    public StringField $fornex_api_key;
    public StringField $freedns_user;
    public StringField $freedns_password;
    public StringField $gandi_livedns_key;
    public StringField $gcore_key;
    public StringField $geoscaling_username;
    public StringField $geoscaling_password;
    public StringField $gd_key;
    public StringField $gd_secret;
    public StringField $googledomains_access_token;
    public StringField $googledomains_zone;
    public StringField $hetzner_token;
    public StringField $hexonet_login;
    public StringField $hexonet_password;
    public StringField $huaweicloud_username;
    public StringField $huaweicloud_password;
    public StringField $huaweicloud_domainname;
    public StringField $he_username;
    public StringField $he_password;
    public StringField $hostingde_apikey;
    public StringField $hostingde_endpoint;
    public StringField $infoblox_creds;
    public StringField $infoblox_server;
    public StringField $infoblox_view;
    public StringField $infomaniak_api_token;
    public StringField $default_infomaniak_api_url;
    public StringField $infomaniak_ttl;
    public StringField $ionos_prefix;
    public StringField $ionos_secret;
    public StringField $ipv64_token;
    public StringField $internetbs_api_key;
    public StringField $internetbs_api_password;
    public StringField $inwx_username;
    public StringField $inwx_password;
    public StringField $inwx_shared_secret;
    public StringField $ispc_user;
    public StringField $ispc_password;
    public StringField $ispc_api;
    public StringField $ispc_api_insecure;
    public StringField $jd_access_key_id;
    public StringField $jd_access_key_secret;
    public StringField $jd_region;
    public StringField $joker_username;
    public StringField $joker_password;
    public StringField $kappernetdns_key;
    public StringField $kappernetdns_secret;
    public StringField $kinghost_username;
    public StringField $kinghost_password;
    public StringField $knot_server;
    public StringField $knot_key;
    public StringField $la_id;
    public StringField $la_key;
    public StringField $lsw_key;
    public StringField $limacity_apikey;
    public StringField $linode_api_key;
    public StringField $linode_v4_api_key;
    public StringField $loopia_user;
    public StringField $loopia_password;
    public StringField $lua_key;
    public StringField $lua_email;
    public StringField $miab_username;
    public StringField $miab_password;
    public StringField $miab_server;
    public StringField $misaka_key;
    public StringField $mydnsjp_masterid;
    public StringField $mydnsjp_password;
    public StringField $mb_ak;
    public StringField $mb_as;
    public StringField $namecom_username;
    public StringField $namecom_token;
    public StringField $namecheap_api_key;
    public StringField $namecheap_username;
    public StringField $nm_user;
    public StringField $nm_sha256;
    public StringField $nanelo_token;
    public StringField $nederhost_key;
    public StringField $namesilo_key;
    public StringField $neodigit_api_token;
    public StringField $nc_apikey;
    public StringField $nc_apipw;
    public StringField $nc_cid;
    public StringField $netlify_access_token;
    public StringField $nic_clientid;
    public StringField $nic_clientsecret;
    public StringField $nic_username;
    public StringField $nic_password;
    public StringField $ns1_key;
    public StringField $nw_api_token;
    public StringField $nw_api_endpoint;
    public StringField $onecom_user;
    public StringField $onecom_password;
    public StringField $online_api_key;
    public StringField $oci_cli_tenancy;
    public StringField $oci_cli_user;
    public StringField $oci_cli_region;
    public StringField $oci_cli_key;
    public StringField $openprovider_user;
    public StringField $openprovider_passwordhash;
    public StringField $ovh_ak;
    public StringField $ovh_as;
    public StringField $ovh_ck;
    public StringField $ovh_end_point;
    public StringField $pleskxml_user;
    public StringField $pleskxml_pass;
    public StringField $pleskxml_uri;
    public StringField $pointhq_key;
    public StringField $pointhq_email;
    public StringField $porkbun_api_key;
    public StringField $porkbun_secret_api_key;
    public StringField $pdns_url;
    public StringField $pdns_serverid;
    public StringField $pdns_token;
    public StringField $pdns_ttl;
    public StringField $rackcorp_apiuuid;
    public StringField $rackcorp_apisecret;
    public StringField $rackspace_username;
    public StringField $rackspace_apikey;
    public StringField $rage4_username;
    public StringField $rage4_token;
    public StringField $rcode0_api_token;
    public StringField $rcode0_url;
    public StringField $rcode0_ttl;
    public StringField $regru_api_username;
    public StringField $regru_api_password;
    public StringField $scaleway_api_token;
    public StringField $schlundtech_user;
    public StringField $schlundtech_password;
    public StringField $sl_key;
    public StringField $selfhostdns_username;
    public StringField $selfhostdns_password;
    public StringField $selfhostdns_map;
    public StringField $servercow_api_username;
    public StringField $servercow_api_password;
    public StringField $simply_accountname;
    public StringField $simply_apikey;
    public StringField $simply_api;
    public StringField $tele3_key;
    public StringField $tele3_secret;
    public StringField $tencent_secretid;
    public StringField $tencent_secretkey;
    public StringField $udr_user;
    public StringField $udr_pass;
    public StringField $ultra_usr;
    public StringField $ULTRA_PWD;
    public StringField $uno_user;
    public StringField $uno_key;
    public StringField $variomedia_api_token;
    public StringField $veesp_user;
    public StringField $veesp_password;
    public StringField $vercel_token;
    public StringField $vscale_api_key;
    public StringField $vultr_api_key;
    public StringField $ws_apikey;
    public StringField $ws_apisecret;
    public StringField $west_username;
    public StringField $west_key;
    public StringField $world4you_username;
    public StringField $world4you_password;
    public StringField $pdd_token;
    public StringField $yc_zone_id;
    public StringField $yc_folder_id;
    public StringField $yc_sa_id;
    public StringField $yc_sa_key_id;
    public StringField $yc_sa_key_file_pem_b64;
    public StringField $zm_key;
    public StringField $zone_username;
    public StringField $zone_key;
    public StringField $zilore_key;
    public StringField $technitium_server;
    public StringField $technitium_token;
    public StringField $anydnschallengealias;
    public BooleanField $anydnschallengedomain;

    public function __construct(mixed $id = null, mixed $parent_id = null, mixed $data = [], mixed ...$options) {
        # Define model attributes
        $this->always_apply = true;
        $this->many = true;
        $this->many_minimum = 1;
        $this->many_maximum = 100;
        $this->parent_model_class = 'ACMECertificate';
        $this->config_path = 'a_domainlist/item';
        $this->packages = ['pfSense-pkg-acme'];
        $this->package_includes = [
            'acme/acme.inc',
            'acme/acme_utils.inc',
            'acme/acme_htmllist.inc',
            'acme/pkg_acme_tabs.inc',
        ];

        # Define model Fields
        $this->name = new StringField(
            required: true,
            unique: true,
            validators: [new HostnameValidator(allow_hostname: false, allow_fqdn: true)],
            help_text: 'The fully-qualified domain name of this domain (SAN).',
        );
        $this->status = new StringField(
            default: 'enable',
            choices: ['enable', 'disable'],
            help_text: 'The activation status of the ACME certificate.',
        );
        $this->method = new StringField(
            required: true,
            choices_callable: 'get_method_choices',
            help_text: 'The method to use to validate this domain.',
        );
        $this->webrootfolder = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['method' => 'webroot'],
            help_text: 'Folder into which the acme challenge response is written; for example: /usr/local/www/.well-known/acme-challenge/',
        );
        $this->webrootftpftpserver = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['method' => 'webrootftp'],
            help_text: 'Hostname of FTP server to which ACME will connect (e.g. ftps://www.webserver.tld ). Currently supports FTPS (passive) and SFTP.',
        );
        $this->webrootftpusername = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['method' => 'webrootftp'],
            help_text: 'Username for the remote server',
        );
        $this->webrootftppassword = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            conditions: ['method' => 'webrootftp'],
            help_text: 'Password to authenticate this user on the remote server',
        );
        $this->webrootftpfolder = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['method' => 'webrootftp'],
            help_text: 'Folder into which the acme challenge response is written (e.g. /.well-known/acme-challenge/)',
        );
        $this->standaloneport = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['method' => 'standalone'],
            help_text: 'HTTP listen port for stand-alone server. Must be 80 or have port 80 on WAN forwarded to this port. Firewall rules must allow traffic to reach this port.',
        );
        $this->standaloneipv6 = new BooleanField(
            default: false,
            conditions: ['method' => 'standalone'],
            help_text: 'Bind to IPv6 instead of IPv4.',
        );
        $this->standalonetlsport = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['method' => 'standalonetls'],
            help_text: 'TLS listen port for stand-alone server. Must be 443 or have port 443 on WAN forwarded to this port. Firewall rules must allow traffic to reach this port.',
        );
        $this->nsupdate_server = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nsupdatensupdate_server',
            conditions: ['method' => 'dns_nsupdate'],
            help_text: 'The DNS server to which updates are sent (IP address or hostname)',
        );
        $this->nsupdate_keyname = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nsupdatensupdate_keyname',
            conditions: ['method' => 'dns_nsupdate'],
            help_text: '(Optional) A name for the key, if it is different than _acme-challenge.[DomainName]',
        );
        $this->nsupdate_keyalgo = new StringField(
            default: '',
            choices: ['', '157', '165', '164', '163', '162', '161'],
            allow_empty: true,
            internal_name: 'dns_nsupdatensupdate_keyalgo',
            conditions: ['method' => 'dns_nsupdate'],
            help_text: 'Algorithm used to generate the authentication Key for this zone',
        );
        $this->nsupdate_key = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nsupdatensupdate_key',
            conditions: ['method' => 'dns_nsupdate'],
            help_text: 'The key which authenticates updates for this zone',
        );
        $this->nsupdate_zone = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nsupdatensupdate_zone',
            conditions: ['method' => 'dns_nsupdate'],
            help_text: '(Optional) Explicitly set the zone name for updates.',
        );
        $this->one984hosting_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_1984hostingone984hosting_username',
            conditions: ['method' => 'dns_1984hosting'],
            help_text: '1984Hosting Username',
        );
        $this->one984hosting_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_1984hostingone984hosting_password',
            conditions: ['method' => 'dns_1984hosting'],
            help_text: '1984Hosting Password',
        );
        $this->acmeproxy_endpoint = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_acmeproxyacmeproxy_endpoint',
            conditions: ['method' => 'dns_acmeproxy'],
            help_text: 'Acmeproxy Endpoint URL (https://ip:port)',
        );
        $this->acmeproxy_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_acmeproxyacmeproxy_username',
            conditions: ['method' => 'dns_acmeproxy'],
            help_text: 'Acmeproxy Username',
        );
        $this->acmeproxy_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_acmeproxyacmeproxy_password',
            conditions: ['method' => 'dns_acmeproxy'],
            help_text: 'Acmeproxy Password',
        );
        $this->acmedns_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_acmednsacmedns_username',
            conditions: ['method' => 'dns_acmedns'],
            help_text: 'acme-dns.io Username',
        );
        $this->acmedns_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_acmednsacmedns_password',
            conditions: ['method' => 'dns_acmedns'],
            help_text: 'acme-dns.io Password',
        );
        $this->acmedns_subdomain = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_acmednsacmedns_subdomain',
            conditions: ['method' => 'dns_acmedns'],
            help_text: 'acme-dns.io subdomain',
        );
        $this->acmedns_update_url = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_acmednsacmedns_update_url',
            conditions: ['method' => 'dns_acmedns'],
            help_text: '(optional) Custom ACME DNS Base URL',
        );
        $this->active24_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_active24active24_token',
            conditions: ['method' => 'dns_active24'],
            help_text: 'Active24 Token',
        );
        $this->akamai_host = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_edgednsakamai_host',
            conditions: ['method' => 'dns_edgedns'],
            help_text: 'Hostname',
        );
        $this->akamai_access_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_edgednsakamai_access_token',
            conditions: ['method' => 'dns_edgedns'],
            help_text: 'Access Token',
        );
        $this->akamai_client_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_edgednsakamai_client_token',
            conditions: ['method' => 'dns_edgedns'],
            help_text: 'Client Token',
        );
        $this->akamai_client_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_edgednsakamai_client_secret',
            conditions: ['method' => 'dns_edgedns'],
            help_text: 'Client Secret',
        );
        $this->ali_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_aliali_key',
            conditions: ['method' => 'dns_ali'],
            help_text: 'API Key',
        );
        $this->ali_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_aliali_secret',
            conditions: ['method' => 'dns_ali'],
            help_text: 'API Secret',
        );
        $this->kas_login = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_kaskas_login',
            conditions: ['method' => 'dns_kas'],
            help_text: 'Login',
        );
        $this->kas_authtype = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_kaskas_authtype',
            conditions: ['method' => 'dns_kas'],
            help_text: 'Auth type (default: sha1)',
        );
        $this->kas_authdata = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_kaskas_authdata',
            conditions: ['method' => 'dns_kas'],
            help_text: 'Auth data',
        );
        $this->ad_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_adad_api_key',
            conditions: ['method' => 'dns_ad'],
            help_text: 'Alwaysdata API Key',
        );
        $this->anx_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_anxanx_token',
            conditions: ['method' => 'dns_anx'],
            help_text: 'API Token',
        );
        $this->af_api_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_artfilesaf_api_username',
            conditions: ['method' => 'dns_artfiles'],
            help_text: 'ArtFiles Username',
        );
        $this->af_api_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_artfilesaf_api_password',
            conditions: ['method' => 'dns_artfiles'],
            help_text: 'ArtFiles Password',
        );
        $this->arvan_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_arvanarvan_token',
            conditions: ['method' => 'dns_arvan'],
            help_text: 'Arvan API Token',
        );
        $this->aurora_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_auroraaurora_key',
            conditions: ['method' => 'dns_aurora'],
            help_text: 'API Key',
        );
        $this->aurora_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_auroraaurora_secret',
            conditions: ['method' => 'dns_aurora'],
            help_text: 'API Secret. Obtain the key and secret from https://cp.pcextreme.nl/auroradns/users.',
        );
        $this->autodns_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_autodnsautodns_user',
            conditions: ['method' => 'dns_autodns'],
            help_text: 'autoDNS Username',
        );
        $this->autodns_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_autodnsautodns_password',
            conditions: ['method' => 'dns_autodns'],
            help_text: 'autoDNS Password',
        );
        $this->autodns_context = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_autodnsautodns_context',
            conditions: ['method' => 'dns_autodns'],
            help_text: 'autoDNS Context',
        );
        $this->aws_access_key_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_awsaws_access_key_id',
            conditions: ['method' => 'dns_aws'],
            help_text: 'AWS Access Key / API ID',
        );
        $this->aws_secret_access_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_awsaws_secret_access_key',
            conditions: ['method' => 'dns_aws'],
            help_text: 'AWS Secret Access / API Key',
        );
        $this->aws_dns_slowrate = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_awsaws_dns_slowrate',
            conditions: ['method' => 'dns_aws'],
            help_text: 'Sleep interval after TXT record update, in seconds (default: 1)',
        );
        $this->azion_email = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_azionazion_email',
            conditions: ['method' => 'dns_azion'],
            help_text: 'Account e-mail address',
        );
        $this->azion_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_azionazion_password',
            conditions: ['method' => 'dns_azion'],
            help_text: 'Account password',
        );
        $this->azuredns_subscriptionid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_azureazuredns_subscriptionid',
            conditions: ['method' => 'dns_azure'],
            help_text: 'Azure Subscription ID. First, <a href="https://github.com/acmesh-official/acme.sh/wiki/How-to-use-Azure-DNS">setup a service principal for access to the DNS Zone</a>.',
        );
        $this->azuredns_tenantid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_azureazuredns_tenantid',
            conditions: ['method' => 'dns_azure'],
            help_text: 'Azure Tenant ID',
        );
        $this->azuredns_appid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_azureazuredns_appid',
            conditions: ['method' => 'dns_azure'],
            help_text: 'Azure App ID',
        );
        $this->azuredns_clientsecret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_azureazuredns_clientsecret',
            conditions: ['method' => 'dns_azure'],
            help_text: 'Azure Client Secret',
        );
        $this->bookmyname_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_bookmynamebookmyname_username',
            conditions: ['method' => 'dns_bookmyname'],
            help_text: 'BookMyName Username',
        );
        $this->bookmyname_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_bookmynamebookmyname_password',
            conditions: ['method' => 'dns_bookmyname'],
            help_text: 'BookMyName Password',
        );
        $this->bunny_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_bunnybunny_api_key',
            conditions: ['method' => 'dns_bunny'],
            help_text: 'Bunny DNS API Key',
        );
        $this->clouddns_email = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_clouddnsclouddns_email',
            conditions: ['method' => 'dns_clouddns'],
            help_text: 'CloudDNS e-mail address',
        );
        $this->clouddns_client_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_clouddnsclouddns_client_id',
            conditions: ['method' => 'dns_clouddns'],
            help_text: 'CloudDNS client ID',
        );
        $this->clouddns_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_clouddnsclouddns_password',
            conditions: ['method' => 'dns_clouddns'],
            help_text: 'CloudDNS Password',
        );
        $this->cloudns_auth_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cloudnscloudns_auth_id',
            conditions: ['method' => 'dns_cloudns'],
            help_text: 'Authentication ID',
        );
        $this->cloudns_sub_auth_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cloudnscloudns_sub_auth_id',
            conditions: ['method' => 'dns_cloudns'],
            help_text: 'Sub authentication ID',
        );
        $this->cloudns_auth_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_cloudnscloudns_auth_password',
            conditions: ['method' => 'dns_cloudns'],
            help_text: 'ClouDNS Password',
        );
        $this->cf_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_cfcf_key',
            conditions: ['method' => 'dns_cf'],
            help_text: 'Cloudflare API Key',
        );
        $this->cf_email = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cfcf_email',
            conditions: ['method' => 'dns_cf'],
            help_text: 'Cloudflare API Email Address',
        );
        $this->cf_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_cfcf_token',
            conditions: ['method' => 'dns_cf'],
            help_text: 'Cloudflare API Token',
        );
        $this->cf_account_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cfcf_account_id',
            conditions: ['method' => 'dns_cf'],
            help_text: 'Cloudflare API Account ID',
        );
        $this->cf_zone_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cfcf_zone_id',
            conditions: ['method' => 'dns_cf'],
            help_text: 'Cloudflare API Zone ID',
        );
        $this->conoha_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_conohaconoha_username',
            conditions: ['method' => 'dns_conoha'],
            help_text: 'Conoha Username',
        );
        $this->conoha_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_conohaconoha_password',
            conditions: ['method' => 'dns_conoha'],
            help_text: 'Conoha Password',
        );
        $this->conoha_tenantid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_conohaconoha_tenantid',
            conditions: ['method' => 'dns_conoha'],
            help_text: 'Conoha Tenant ID',
        );
        $this->conoha_identityserviceapi = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_conohaconoha_identityserviceapi',
            conditions: ['method' => 'dns_conoha'],
            help_text: 'Conoha Identity Service API',
        );
        $this->constellix_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_constellixconstellix_key',
            conditions: ['method' => 'dns_constellix'],
            help_text: 'Constellix Key',
        );
        $this->constellix_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_constellixconstellix_secret',
            conditions: ['method' => 'dns_constellix'],
            help_text: 'Constellix Secret',
        );
        $this->cpanel_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cpanelcpanel_username',
            conditions: ['method' => 'dns_cpanel'],
            help_text: 'cPanel username',
        );
        $this->cpanel_apitoken = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_cpanelcpanel_apitoken',
            conditions: ['method' => 'dns_cpanel'],
            help_text: 'cPanel API token',
        );
        $this->cpanel_hostname = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cpanelcpanel_hostname',
            conditions: ['method' => 'dns_cpanel'],
            help_text: 'URL to cPanel instance',
        );
        $this->cn_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cncn_user',
            conditions: ['method' => 'dns_cn'],
            help_text: 'Core Networks Username',
        );
        $this->cn_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_cncn_password',
            conditions: ['method' => 'dns_cn'],
            help_text: 'Core Networks Password',
        );
        $this->curanet_authclientid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_curanetcuranet_authclientid',
            conditions: ['method' => 'dns_curanet'],
            help_text: 'Authentication Client ID',
        );
        $this->curanet_authsecret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_curanetcuranet_authsecret',
            conditions: ['method' => 'dns_curanet'],
            help_text: 'Authentication Secret',
        );
        $this->cy_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_cyoncy_username',
            conditions: ['method' => 'dns_cyon'],
            help_text: 'CY username',
        );
        $this->cy_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_cyoncy_password',
            conditions: ['method' => 'dns_cyon'],
            help_text: 'CY Password',
        );
        $this->ddnss_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_ddnssddnss_token',
            conditions: ['method' => 'dns_ddnss'],
            help_text: 'API Token (e.g. aaaaaaaa-bbbb-cccc-dddd-eeeeeeeeeeee)',
        );
        $this->dedyn_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_desecdedyn_token',
            conditions: ['method' => 'dns_desec'],
            help_text: 'deSEC.io API Token',
        );
        $this->dedyn_name = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_desecdedyn_name',
            conditions: ['method' => 'dns_desec'],
            help_text: 'deSEC.io Username',
        );
        $this->do_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dgondo_api_key',
            conditions: ['method' => 'dns_dgon'],
            help_text: 'DigitalOcean API Key',
        );
        $this->da_api = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dada_api',
            conditions: ['method' => 'dns_da'],
            help_text: 'DirectAdmin API URI (e.g. https://remoteUser:remotePassword@da.example.com:8443)',
        );
        $this->da_api_insecure = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dada_api_insecure',
            conditions: ['method' => 'dns_da'],
            help_text: 'DirectAdmin API Security check, 0=check for valid SSL certificate, 1=always accept',
        );
        $this->dnsexit_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dnsexitdnsexit_api_key',
            conditions: ['method' => 'dns_dnsexit'],
            help_text: 'DNSExit API Key',
        );
        $this->dnsexit_auth_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dnsexitdnsexit_auth_user',
            conditions: ['method' => 'dns_dnsexit'],
            help_text: 'DNSExit Username',
        );
        $this->dnsexit_auth_pass = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dnsexitdnsexit_auth_pass',
            conditions: ['method' => 'dns_dnsexit'],
            help_text: 'DNSExit Password',
        );
        $this->dnshome_subdomain = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dnshomednshome_subdomain',
            conditions: ['method' => 'dns_dnshome'],
            help_text: 'Subdomain',
        );
        $this->dnshome_subdomainpassword = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dnshomednshome_subdomainpassword',
            conditions: ['method' => 'dns_dnshome'],
            help_text: 'Subdomain Password',
        );
        $this->dnsimple_oauth_token = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dnsimplednsimple_oauth_token',
            conditions: ['method' => 'dns_dnsimple'],
            help_text: 'DNSimple oauth token, visit <a href="https://dnsimple.com/user">https://dnsimple.com/user</a> to generate.',
        );
        $this->me_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_meme_key',
            conditions: ['method' => 'dns_me'],
            help_text: 'DNSMadeEasy API Key',
        );
        $this->me_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_meme_secret',
            conditions: ['method' => 'dns_me'],
            help_text: 'DNSMadeEasy API Secret',
        );
        $this->dnsservices_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dnsservicesdnsservices_username',
            conditions: ['method' => 'dns_dnsservices'],
            help_text: 'dns.services Username',
        );
        $this->dnsservices_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dnsservicesdnsservices_password',
            conditions: ['method' => 'dns_dnsservices'],
            help_text: 'dns.services Password',
        );
        $this->do_letoken = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_doapido_letoken',
            conditions: ['method' => 'dns_doapi'],
            help_text: 'DO.de API Token',
        );
        $this->do_pid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dodo_pid',
            conditions: ['method' => 'dns_do'],
            help_text: 'DO Customer ID',
        );
        $this->do_pw = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dodo_pw',
            conditions: ['method' => 'dns_do'],
            help_text: 'DO Password',
        );
        $this->domeneshop_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_domeneshopdomeneshop_token',
            conditions: ['method' => 'dns_domeneshop'],
            help_text: 'Domeneshop API Token',
        );
        $this->domeneshop_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_domeneshopdomeneshop_secret',
            conditions: ['method' => 'dns_domeneshop'],
            help_text: 'Domeneshop API Secret',
        );
        $this->dp_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dpdp_id',
            conditions: ['method' => 'dns_dp'],
            help_text: 'Dnspod API ID',
        );
        $this->dp_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dpdp_key',
            conditions: ['method' => 'dns_dp'],
            help_text: 'Dnspod API Key',
        );
        $this->dpi_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dpidpi_id',
            conditions: ['method' => 'dns_dpi'],
            help_text: 'Dnspod API ID',
        );
        $this->dpi_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dpidpi_key',
            conditions: ['method' => 'dns_dpi'],
            help_text: 'Dnspod API Key',
        );
        $this->dh_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dreamhostdh_api_key',
            conditions: ['method' => 'dns_dreamhost'],
            help_text: 'Dreamhost API Token',
        );
        $this->duckdns_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_duckdnsduckdns_token',
            conditions: ['method' => 'dns_duckdns'],
            help_text: 'DuckDNS API Token (Look in DuckDNS account settings)',
        );
        $this->dd_api_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_durablednsdd_api_user',
            conditions: ['method' => 'dns_durabledns'],
            help_text: 'DurableDNS API User',
        );
        $this->dd_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_durablednsdd_api_key',
            conditions: ['method' => 'dns_durabledns'],
            help_text: 'DurableDNS API Key',
        );
        $this->dyn_customer = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dyndyn_customer',
            conditions: ['method' => 'dns_dyn'],
            help_text: 'dyn.com customer ID',
        );
        $this->dyn_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dyndyn_username',
            conditions: ['method' => 'dns_dyn'],
            help_text: 'dyn.com API Username (Dyn Managed DNS user, Needs Z&R Permissions for RecordAdd, RecordUpdate, RecordDelete, RecordGet, ZoneGet, ZoneAddNode, ZoneRemoveNode, ZonePublish)',
        );
        $this->dyn_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dyndyn_password',
            conditions: ['method' => 'dns_dyn'],
            help_text: 'dyn.com Password',
        );
        $this->df_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dfdf_user',
            conditions: ['method' => 'dns_df'],
            help_text: 'dyndnsfree.de Username',
        );
        $this->df_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dfdf_password',
            conditions: ['method' => 'dns_df'],
            help_text: 'dyndnsfree.de Password',
        );
        $this->dynu_clientid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_dynudynu_clientid',
            conditions: ['method' => 'dns_dynu'],
            help_text: 'Dynu API Client ID created in the Dynu account settings',
        );
        $this->dynu_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_dynudynu_secret',
            conditions: ['method' => 'dns_dynu'],
            help_text: 'Dynu API Secret',
        );
        $this->easydns_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_easydnseasydns_key',
            conditions: ['method' => 'dns_easydns'],
            help_text: 'easyDNS API Key. Sign up for a key at https://cp.easydns.com/manage/security/api/signup.php',
        );
        $this->easydns_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_easydnseasydns_token',
            conditions: ['method' => 'dns_easydns'],
            help_text: 'easyDNS API Token',
        );
        $this->euserv_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_euserveuserv_username',
            conditions: ['method' => 'dns_euserv'],
            help_text: 'Euserv.eu Username',
        );
        $this->euserv_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_euserveuserv_password',
            conditions: ['method' => 'dns_euserv'],
            help_text: 'Euserv.eu Password',
        );
        $this->exoscale_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_exoscaleexoscale_api_key',
            conditions: ['method' => 'dns_exoscale'],
            help_text: 'Exoscale API Key',
        );
        $this->exoscale_secret_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_exoscaleexoscale_secret_key',
            conditions: ['method' => 'dns_exoscale'],
            help_text: 'Exoscale Secret Key',
        );
        $this->fornex_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_fornexfornex_api_key',
            conditions: ['method' => 'dns_fornex'],
            help_text: 'Fornex API Key',
        );
        $this->freedns_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_freednsfreedns_user',
            conditions: ['method' => 'dns_freedns'],
            help_text: 'FreeDNS username',
        );
        $this->freedns_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_freednsfreedns_password',
            conditions: ['method' => 'dns_freedns'],
            help_text: 'FreeDNS Password',
        );
        $this->gandi_livedns_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_gandi_livednsgandi_livedns_key',
            conditions: ['method' => 'dns_gandi_livedns'],
            help_text: 'Gandi LiveDNS API Key, retrieved from <a href="https://account.gandi.net">https://account.gandi.net</a>',
        );
        $this->gcore_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_gcoregcore_key',
            conditions: ['method' => 'dns_gcore'],
            help_text: 'Gcore API Key',
        );
        $this->geoscaling_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_geoscalinggeoscaling_username',
            conditions: ['method' => 'dns_geoscaling'],
            help_text: 'Username',
        );
        $this->geoscaling_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_geoscalinggeoscaling_password',
            conditions: ['method' => 'dns_geoscaling'],
            help_text: 'Password',
        );
        $this->gd_key = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_gdgd_key',
            conditions: ['method' => 'dns_gd'],
            help_text: 'GoDaddy API Key',
        );
        $this->gd_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_gdgd_secret',
            conditions: ['method' => 'dns_gd'],
            help_text: 'GoDaddy API Secret',
        );
        $this->googledomains_access_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_googledomainsgoogledomains_access_token',
            conditions: ['method' => 'dns_googledomains'],
            help_text: 'Google Domains API Access Token',
        );
        $this->googledomains_zone = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_googledomainsgoogledomains_zone',
            conditions: ['method' => 'dns_googledomains'],
            help_text: 'Google Domains DNS Zone',
        );
        $this->hetzner_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_hetznerhetzner_token',
            conditions: ['method' => 'dns_hetzner'],
            help_text: 'Hetzner API Token. Visit https://dns.hetzner.com/settings/api-token to retrieve.',
        );
        $this->hexonet_login = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_hexonethexonet_login',
            conditions: ['method' => 'dns_hexonet'],
            help_text: 'Hexonet Username',
        );
        $this->hexonet_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_hexonethexonet_password',
            conditions: ['method' => 'dns_hexonet'],
            help_text: 'Hexonet Password',
        );
        $this->huaweicloud_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_huaweicloudhuaweicloud_username',
            conditions: ['method' => 'dns_huaweicloud'],
            help_text: 'Username',
        );
        $this->huaweicloud_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_huaweicloudhuaweicloud_password',
            conditions: ['method' => 'dns_huaweicloud'],
            help_text: 'Password',
        );
        $this->huaweicloud_domainname = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_huaweicloudhuaweicloud_domainname',
            conditions: ['method' => 'dns_huaweicloud'],
            help_text: 'Domain Name',
        );
        $this->he_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_hehe_username',
            conditions: ['method' => 'dns_he'],
            help_text: 'Hurricane Electric username',
        );
        $this->he_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_hehe_password',
            conditions: ['method' => 'dns_he'],
            help_text: 'Hurricane Electric password',
        );
        $this->hostingde_apikey = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_hostingdehostingde_apikey',
            conditions: ['method' => 'dns_hostingde'],
            help_text: 'Hosting.de API Key',
        );
        $this->hostingde_endpoint = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_hostingdehostingde_endpoint',
            conditions: ['method' => 'dns_hostingde'],
            help_text: 'Hosting.de API endpoint, e.g. https://secure.hosting.de',
        );
        $this->infoblox_creds = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_infobloxinfoblox_creds',
            conditions: ['method' => 'dns_infoblox'],
            help_text: 'Infoblox credentials in <strong>username:password</strong> format',
        );
        $this->infoblox_server = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_infobloxinfoblox_server',
            conditions: ['method' => 'dns_infoblox'],
            help_text: 'Infoblox server IP address or hostname',
        );
        $this->infoblox_view = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_infobloxinfoblox_view',
            conditions: ['method' => 'dns_infoblox'],
            help_text: 'Infoblox DNS View name, or enter "default"',
        );
        $this->infomaniak_api_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_infomaniakinfomaniak_api_token',
            conditions: ['method' => 'dns_infomaniak'],
            help_text: 'Infomaniak API token. Visit https://manager.infomaniak.com/v3/&lt;account_id&gt;/api/dashboard and generate a token with the scope Domain.',
        );
        $this->default_infomaniak_api_url = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_infomaniakdefault_infomaniak_api_url',
            conditions: ['method' => 'dns_infomaniak'],
            help_text: 'Infomaniak API URL (Default: https://api.infomaniak.com)',
        );
        $this->infomaniak_ttl = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_infomaniakinfomaniak_ttl',
            conditions: ['method' => 'dns_infomaniak'],
            help_text: 'Infomaniak DNS record TTL (Default: 300)',
        );
        $this->ionos_prefix = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ionosionos_prefix',
            conditions: ['method' => 'dns_ionos'],
            help_text: 'Prefix',
        );
        $this->ionos_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_ionosionos_secret',
            conditions: ['method' => 'dns_ionos'],
            help_text: 'Secret. Read https://beta.developer.hosting.ionos.de/docs/getstarted to learn how to get a prefix and secret.',
        );
        $this->ipv64_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_ipv64ipv64_token',
            conditions: ['method' => 'dns_ipv64'],
            help_text: 'IPv64.net Access Token',
        );
        $this->internetbs_api_key = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_internetbsinternetbs_api_key',
            conditions: ['method' => 'dns_internetbs'],
            help_text: 'Internet.BS API Key',
        );
        $this->internetbs_api_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_internetbsinternetbs_api_password',
            conditions: ['method' => 'dns_internetbs'],
            help_text: 'Internet.BS API Password',
        );
        $this->inwx_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_inwxinwx_username',
            conditions: ['method' => 'dns_inwx'],
            help_text: 'INWX.de username',
        );
        $this->inwx_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_inwxinwx_password',
            conditions: ['method' => 'dns_inwx'],
            help_text: 'INWX.de password',
        );
        $this->inwx_shared_secret = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_inwxinwx_shared_secret',
            conditions: ['method' => 'dns_inwx'],
            help_text: 'INWX.de shared secret',
        );
        $this->ispc_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ispconfigispc_user',
            conditions: ['method' => 'dns_ispconfig'],
            help_text: 'ISPConfig remoteUser',
        );
        $this->ispc_password = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_ispconfigispc_password',
            conditions: ['method' => 'dns_ispconfig'],
            help_text: 'ISPConfig remotePassword',
        );
        $this->ispc_api = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ispconfigispc_api',
            conditions: ['method' => 'dns_ispconfig'],
            help_text: 'API URL (e.g. https://ispc.domain.tld:8080/remote/json.php )',
        );
        $this->ispc_api_insecure = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ispconfigispc_api_insecure',
            conditions: ['method' => 'dns_ispconfig'],
            help_text: 'Set 1 for insecure and 0 for secure. Controls whether the server TLS certificate is checked for validity (0) or always accepted (1)',
        );
        $this->jd_access_key_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_jdjd_access_key_id',
            conditions: ['method' => 'dns_jd'],
            help_text: 'jdcloud Access Key ID',
        );
        $this->jd_access_key_secret = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_jdjd_access_key_secret',
            conditions: ['method' => 'dns_jd'],
            help_text: 'jdcloud Access Key Secret',
        );
        $this->jd_region = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_jdjd_region',
            conditions: ['method' => 'dns_jd'],
            help_text: 'jdcloud Region',
        );
        $this->joker_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_jokerjoker_username',
            conditions: ['method' => 'dns_joker'],
            help_text: 'Joker.com Username',
        );
        $this->joker_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_jokerjoker_password',
            conditions: ['method' => 'dns_joker'],
            help_text: 'Joker.com Password',
        );
        $this->kappernetdns_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_kappernetkappernetdns_key',
            conditions: ['method' => 'dns_kappernet'],
            help_text: 'kapper.net API Key',
        );
        $this->kappernetdns_secret = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_kappernetkappernetdns_secret',
            conditions: ['method' => 'dns_kappernet'],
            help_text: 'kapper.net API Secret',
        );
        $this->kinghost_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_kinghostkinghost_username',
            conditions: ['method' => 'dns_kinghost'],
            help_text: 'Kinghost API Username',
        );
        $this->kinghost_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_kinghostkinghost_password',
            conditions: ['method' => 'dns_kinghost'],
            help_text: 'Kinghost API Password',
        );
        $this->knot_server = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_knotknot_server',
            conditions: ['method' => 'dns_knot'],
            help_text: 'IP address of the Knot server',
        );
        $this->knot_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_knotknot_key',
            conditions: ['method' => 'dns_knot'],
            help_text: 'Knot TSIG Key',
        );
        $this->la_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_lala_id',
            conditions: ['method' => 'dns_la'],
            help_text: 'ID',
        );
        $this->la_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_lala_key',
            conditions: ['method' => 'dns_la'],
            help_text: 'Key',
        );
        $this->lsw_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_leaseweblsw_key',
            conditions: ['method' => 'dns_leaseweb'],
            help_text: 'Leaseweb API Key',
        );
        $this->limacity_apikey = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_limacitylimacity_apikey',
            conditions: ['method' => 'dns_limacity'],
            help_text: 'API Key must have the following roles: dns.admin, domains.reader',
        );
        $this->linode_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_linodelinode_api_key',
            conditions: ['method' => 'dns_linode'],
            help_text: 'Linode API Key',
        );
        $this->linode_v4_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_linode_v4linode_v4_api_key',
            conditions: ['method' => 'dns_linode_v4'],
            help_text: 'Linode v4 API Key',
        );
        $this->loopia_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_loopialoopia_user',
            conditions: ['method' => 'dns_loopia'],
            help_text: 'Loopia username',
        );
        $this->loopia_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_loopialoopia_password',
            conditions: ['method' => 'dns_loopia'],
            help_text: 'Loopia Password',
        );
        $this->lua_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_lualua_key',
            conditions: ['method' => 'dns_lua'],
            help_text: 'Luadns API Key',
        );
        $this->lua_email = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_lualua_email',
            conditions: ['method' => 'dns_lua'],
            help_text: 'Luadns API Email Address',
        );
        $this->miab_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_miabmiab_username',
            conditions: ['method' => 'dns_miab'],
            help_text: 'MailinaBox Username',
        );
        $this->miab_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_miabmiab_password',
            conditions: ['method' => 'dns_miab'],
            help_text: 'MailinaBox Password',
        );
        $this->miab_server = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_miabmiab_server',
            conditions: ['method' => 'dns_miab'],
            help_text: 'MailinaBox Server',
        );
        $this->misaka_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_misakamisaka_key',
            conditions: ['method' => 'dns_misaka'],
            help_text: 'misaka.io Key',
        );
        $this->mydnsjp_masterid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_mydnsjpmydnsjp_masterid',
            conditions: ['method' => 'dns_mydnsjp'],
            help_text: 'MyDNS.jp Master ID',
        );
        $this->mydnsjp_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_mydnsjpmydnsjp_password',
            conditions: ['method' => 'dns_mydnsjp'],
            help_text: 'MyDNS.jp Password',
        );
        $this->mb_ak = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_mythic_beastsmb_ak',
            conditions: ['method' => 'dns_mythic_beasts'],
            help_text: 'OAuth2 Key',
        );
        $this->mb_as = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_mythic_beastsmb_as',
            conditions: ['method' => 'dns_mythic_beasts'],
            help_text: 'OAuth2 Secret',
        );
        $this->namecom_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_namecomnamecom_username',
            conditions: ['method' => 'dns_namecom'],
            help_text: 'Name.com username',
        );
        $this->namecom_token = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_namecomnamecom_token',
            conditions: ['method' => 'dns_namecom'],
            help_text: 'Name.com API Token',
        );
        $this->namecheap_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_namecheapnamecheap_api_key',
            conditions: ['method' => 'dns_namecheap'],
            help_text: 'Namecheap API Key',
        );
        $this->namecheap_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_namecheapnamecheap_username',
            conditions: ['method' => 'dns_namecheap'],
            help_text: 'Namecheap Username',
        );
        $this->nm_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nmnm_user',
            conditions: ['method' => 'dns_nm'],
            help_text: 'namemaster.de API username',
        );
        $this->nm_sha256 = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_nmnm_sha256',
            conditions: ['method' => 'dns_nm'],
            help_text: 'namemaster.de API password as SHA256 hash',
        );
        $this->nanelo_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_nanelonanelo_token',
            conditions: ['method' => 'dns_nanelo'],
            help_text: 'Nanelo.com Access Token',
        );
        $this->nederhost_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_nederhostnederhost_key',
            conditions: ['method' => 'dns_nederhost'],
            help_text: 'NederHost API Key',
        );
        $this->namesilo_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_namesilonamesilo_key',
            conditions: ['method' => 'dns_namesilo'],
            help_text: 'Namesilo API Key',
        );
        $this->neodigit_api_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_neodigitneodigit_api_token',
            conditions: ['method' => 'dns_neodigit'],
            help_text: 'Neodigit API Token',
        );
        $this->nc_apikey = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_netcupnc_apikey',
            conditions: ['method' => 'dns_netcup'],
            help_text: 'Netcup API Key',
        );
        $this->nc_apipw = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_netcupnc_apipw',
            conditions: ['method' => 'dns_netcup'],
            help_text: 'Netcup API Password',
        );
        $this->nc_cid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_netcupnc_cid',
            conditions: ['method' => 'dns_netcup'],
            help_text: 'Netcup Customer ID/Number',
        );
        $this->netlify_access_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_netlifynetlify_access_token',
            conditions: ['method' => 'dns_netlify'],
            help_text: 'Netlify API Token',
        );
        $this->nic_clientid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nicnic_clientid',
            conditions: ['method' => 'dns_nic'],
            help_text: 'nic.ru API Client ID',
        );
        $this->nic_clientsecret = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nicnic_clientsecret',
            conditions: ['method' => 'dns_nic'],
            help_text: 'nic.ru API Client Secret',
        );
        $this->nic_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_nicnic_username',
            conditions: ['method' => 'dns_nic'],
            help_text: 'nic.ru Username',
        );
        $this->nic_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_nicnic_password',
            conditions: ['method' => 'dns_nic'],
            help_text: 'nic.ru Password',
        );
        $this->ns1_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_nsonens1_key',
            conditions: ['method' => 'dns_nsone'],
            help_text: 'NS1 API Key',
        );
        $this->nw_api_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_nwnw_api_token',
            conditions: ['method' => 'dns_nw'],
            help_text: 'NW API Token',
        );
        $this->nw_api_endpoint = new StringField(
            default: '',
            choices: ['', 'https:\/\/portal.nexcess.net', 'https:\/\/core.thermo.io', 'https:\/\/my.futurehosting.com'],
            allow_empty: true,
            internal_name: 'dns_nwnw_api_endpoint',
            conditions: ['method' => 'dns_nw'],
            help_text: 'Choose the NW API Endpoint',
        );
        $this->onecom_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_oneonecom_user',
            conditions: ['method' => 'dns_one'],
            help_text: 'One.com Username',
        );
        $this->onecom_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_oneonecom_password',
            conditions: ['method' => 'dns_one'],
            help_text: 'One.com Password',
        );
        $this->online_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_onlineonline_api_key',
            conditions: ['method' => 'dns_online'],
            help_text: 'Online.net API Key',
        );
        $this->oci_cli_tenancy = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ocioci_cli_tenancy',
            conditions: ['method' => 'dns_oci'],
            help_text: 'OCID of tenancy that contains the target DNS zone',
        );
        $this->oci_cli_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ocioci_cli_user',
            conditions: ['method' => 'dns_oci'],
            help_text: 'OCID of user with permission to add/remove records from zones',
        );
        $this->oci_cli_region = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ocioci_cli_region',
            conditions: ['method' => 'dns_oci'],
            help_text: 'Tenancy home region',
        );
        $this->oci_cli_key = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ocioci_cli_key',
            conditions: ['method' => 'dns_oci'],
            help_text: 'The private API signing key in PEM format. Using an encrypted private key that needs a passphrase is not supported.',
        );
        $this->openprovider_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_openprovideropenprovider_user',
            conditions: ['method' => 'dns_openprovider'],
            help_text: 'OpenProvider Username',
        );
        $this->openprovider_passwordhash = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_openprovideropenprovider_passwordhash',
            conditions: ['method' => 'dns_openprovider'],
            help_text: 'OpenProvider Password Hash',
        );
        $this->ovh_ak = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_ovhovh_ak',
            conditions: ['method' => 'dns_ovh'],
            help_text: 'OVH Application Key',
        );
        $this->ovh_as = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_ovhovh_as',
            conditions: ['method' => 'dns_ovh'],
            help_text: 'OVH Application Secret',
        );
        $this->ovh_ck = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_ovhovh_ck',
            conditions: ['method' => 'dns_ovh'],
            help_text: 'OVH Consumer Key',
        );
        $this->ovh_end_point = new StringField(
            default: '',
            choices: [
                '',
                'ovh-eu',
                'ovh-ca',
                'kimsufi-eu',
                'kimsufi-ca',
                'soyoustart-eu',
                'soyoustart-ca',
                'runabove-ca',
            ],
            allow_empty: true,
            internal_name: 'dns_ovhovh_end_point',
            conditions: ['method' => 'dns_ovh'],
            help_text: 'Choose the OVH API Endpoint / Region',
        );
        $this->pleskxml_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_pleskxmlpleskxml_user',
            conditions: ['method' => 'dns_pleskxml'],
            help_text: 'Plesk User',
        );
        $this->pleskxml_pass = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_pleskxmlpleskxml_pass',
            conditions: ['method' => 'dns_pleskxml'],
            help_text: 'Plesk Password',
        );
        $this->pleskxml_uri = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_pleskxmlpleskxml_uri',
            conditions: ['method' => 'dns_pleskxml'],
            help_text: 'Plesk Server URI',
        );
        $this->pointhq_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_pointhqpointhq_key',
            conditions: ['method' => 'dns_pointhq'],
            help_text: 'PointHQ API Key',
        );
        $this->pointhq_email = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_pointhqpointhq_email',
            conditions: ['method' => 'dns_pointhq'],
            help_text: 'PointHQ account E-mail address',
        );
        $this->porkbun_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_porkbunporkbun_api_key',
            conditions: ['method' => 'dns_porkbun'],
            help_text: 'API Key',
        );
        $this->porkbun_secret_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_porkbunporkbun_secret_api_key',
            conditions: ['method' => 'dns_porkbun'],
            help_text: 'Secret API Key. Obtain the key and secret from https://porkbun.com/account/api',
        );
        $this->pdns_url = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_pdnspdns_url',
            conditions: ['method' => 'dns_pdns'],
            help_text: 'PowerDNS URL (e.g. http://ns.example.com:8081 )',
        );
        $this->pdns_serverid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_pdnspdns_serverid',
            conditions: ['method' => 'dns_pdns'],
            help_text: 'PowerDNS ServerId (e.g. localhost )',
        );
        $this->pdns_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_pdnspdns_token',
            conditions: ['method' => 'dns_pdns'],
            help_text: 'PowerDNS Token (e.g. 0123456789ABCDEF )',
        );
        $this->pdns_ttl = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_pdnspdns_ttl',
            conditions: ['method' => 'dns_pdns'],
            help_text: 'PowerDNS Record TTL (e.g. 60 )',
        );
        $this->rackcorp_apiuuid = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_rackcorprackcorp_apiuuid',
            conditions: ['method' => 'dns_rackcorp'],
            help_text: 'API UUID',
        );
        $this->rackcorp_apisecret = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_rackcorprackcorp_apisecret',
            conditions: ['method' => 'dns_rackcorp'],
            help_text: 'API Secret',
        );
        $this->rackspace_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_rackspacerackspace_username',
            conditions: ['method' => 'dns_rackspace'],
            help_text: 'Rackspace Username',
        );
        $this->rackspace_apikey = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_rackspacerackspace_apikey',
            conditions: ['method' => 'dns_rackspace'],
            help_text: 'Rackspace API Key',
        );
        $this->rage4_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_rage4rage4_username',
            conditions: ['method' => 'dns_rage4'],
            help_text: 'Username',
        );
        $this->rage4_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_rage4rage4_token',
            conditions: ['method' => 'dns_rage4'],
            help_text: 'Token',
        );
        $this->rcode0_api_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_rcode0rcode0_api_token',
            conditions: ['method' => 'dns_rcode0'],
            help_text: 'Rcode0 API Token',
        );
        $this->rcode0_url = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_rcode0rcode0_url',
            conditions: ['method' => 'dns_rcode0'],
            help_text: 'Rcode0 URL',
        );
        $this->rcode0_ttl = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_rcode0rcode0_ttl',
            conditions: ['method' => 'dns_rcode0'],
            help_text: 'Rcode0 TTL',
        );
        $this->regru_api_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_regruregru_api_username',
            conditions: ['method' => 'dns_regru'],
            help_text: 'reg.ru Username',
        );
        $this->regru_api_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_regruregru_api_password',
            conditions: ['method' => 'dns_regru'],
            help_text: 'reg.ru API Password',
        );
        $this->scaleway_api_token = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_scalewayscaleway_api_token',
            conditions: ['method' => 'dns_scaleway'],
            help_text: 'API Token',
        );
        $this->schlundtech_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_schlundtechschlundtech_user',
            conditions: ['method' => 'dns_schlundtech'],
            help_text: 'schlundtech.de Username',
        );
        $this->schlundtech_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_schlundtechschlundtech_password',
            conditions: ['method' => 'dns_schlundtech'],
            help_text: 'schlundtech.de Password',
        );
        $this->sl_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_selectelsl_key',
            conditions: ['method' => 'dns_selectel'],
            help_text: 'Selectel API Key',
        );
        $this->selfhostdns_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_selfhostselfhostdns_username',
            conditions: ['method' => 'dns_selfhost'],
            help_text: 'Username (Customer number, not email address or DynDNS account)',
        );
        $this->selfhostdns_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_selfhostselfhostdns_password',
            conditions: ['method' => 'dns_selfhost'],
            help_text: 'Password',
        );
        $this->selfhostdns_map = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_selfhostselfhostdns_map',
            conditions: ['method' => 'dns_selfhost'],
            help_text: 'Record ID (Edit the record, value is shown in brackets)',
        );
        $this->servercow_api_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_servercowservercow_api_username',
            conditions: ['method' => 'dns_servercow'],
            help_text: 'Servercow username',
        );
        $this->servercow_api_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_servercowservercow_api_password',
            conditions: ['method' => 'dns_servercow'],
            help_text: 'Servercow password',
        );
        $this->simply_accountname = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_simplysimply_accountname',
            conditions: ['method' => 'dns_simply'],
            help_text: 'Account Name',
        );
        $this->simply_apikey = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_simplysimply_apikey',
            conditions: ['method' => 'dns_simply'],
            help_text: 'API Key',
        );
        $this->simply_api = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_simplysimply_api',
            conditions: ['method' => 'dns_simply'],
            help_text: 'API Endpoint URL. Default: https://api.simply.com/1',
        );
        $this->tele3_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_tele3tele3_key',
            conditions: ['method' => 'dns_tele3'],
            help_text: 'Tele3 Key',
        );
        $this->tele3_secret = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_tele3tele3_secret',
            conditions: ['method' => 'dns_tele3'],
            help_text: 'Tele3 Secret',
        );
        $this->tencent_secretid = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_tencenttencent_secretid',
            conditions: ['method' => 'dns_tencent'],
            help_text: 'Tencent Secret ID',
        );
        $this->tencent_secretkey = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_tencenttencent_secretkey',
            conditions: ['method' => 'dns_tencent'],
            help_text: 'Tencent Secret Key',
        );
        $this->udr_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_udrudr_user',
            conditions: ['method' => 'dns_udr'],
            help_text: 'Username',
        );
        $this->udr_pass = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_udrudr_pass',
            conditions: ['method' => 'dns_udr'],
            help_text: 'Password',
        );
        $this->ultra_usr = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ultraultra_usr',
            conditions: ['method' => 'dns_ultra'],
            help_text: 'UltraDNS Username',
        );
        $this->ULTRA_PWD = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_ultraULTRA_PWD',
            conditions: ['method' => 'dns_ultra'],
            help_text: 'UltraDNS Password',
        );
        $this->uno_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_unoeurouno_user',
            conditions: ['method' => 'dns_unoeuro'],
            help_text: 'UnoEuro username',
        );
        $this->uno_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_unoeurouno_key',
            conditions: ['method' => 'dns_unoeuro'],
            help_text: 'UnoEuro API Key',
        );
        $this->variomedia_api_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_variomediavariomedia_api_token',
            conditions: ['method' => 'dns_variomedia'],
            help_text: 'variomedia.de API Token',
        );
        $this->veesp_user = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_veespveesp_user',
            conditions: ['method' => 'dns_veesp'],
            help_text: 'Username',
        );
        $this->veesp_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_veespveesp_password',
            conditions: ['method' => 'dns_veesp'],
            help_text: 'Password',
        );
        $this->vercel_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_vercelvercel_token',
            conditions: ['method' => 'dns_vercel'],
            help_text: 'Vercel Token',
        );
        $this->vscale_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_vscalevscale_api_key',
            conditions: ['method' => 'dns_vscale'],
            help_text: 'vscale API Key',
        );
        $this->vultr_api_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_vultrvultr_api_key',
            conditions: ['method' => 'dns_vultr'],
            help_text: 'vultr.com API Key',
        );
        $this->ws_apikey = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_websupportws_apikey',
            conditions: ['method' => 'dns_websupport'],
            help_text: 'API Key / "Identifier" in the WS Admin',
        );
        $this->ws_apisecret = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_websupportws_apisecret',
            conditions: ['method' => 'dns_websupport'],
            help_text: 'API Secret / "Secret key" in the WS Admin. Obtain the API Key and Secret from https://admin.websupport.sk/en/auth/apiKey.',
        );
        $this->west_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_west_cnwest_username',
            conditions: ['method' => 'dns_west_cn'],
            help_text: 'West.cn Domain API Username',
        );
        $this->west_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_west_cnwest_key',
            conditions: ['method' => 'dns_west_cn'],
            help_text: 'West.cn Domain API Key',
        );
        $this->world4you_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_world4youworld4you_username',
            conditions: ['method' => 'dns_world4you'],
            help_text: 'Username',
        );
        $this->world4you_password = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_world4youworld4you_password',
            conditions: ['method' => 'dns_world4you'],
            help_text: 'Password',
        );
        $this->pdd_token = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_yandexpdd_token',
            conditions: ['method' => 'dns_yandex'],
            help_text: 'Yandex PDD Token, generate at <a href="https://pddimp.yandex.ru/api2/admin/get_token">https://pddimp.yandex.ru/api2/admin/get_token</a>',
        );
        $this->yc_zone_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ycyc_zone_id',
            conditions: ['method' => 'dns_yc'],
            help_text: 'DNS Zone ID',
        );
        $this->yc_folder_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ycyc_folder_id',
            conditions: ['method' => 'dns_yc'],
            help_text: 'Yandex Cloud Folder ID',
        );
        $this->yc_sa_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ycyc_sa_id',
            conditions: ['method' => 'dns_yc'],
            help_text: 'Service Account ID',
        );
        $this->yc_sa_key_id = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ycyc_sa_key_id',
            conditions: ['method' => 'dns_yc'],
            help_text: 'Service Account IAM Key ID',
        );
        $this->yc_sa_key_file_pem_b64 = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_ycyc_sa_key_file_pem_b64',
            conditions: ['method' => 'dns_yc'],
            help_text: 'Base64 content of private key.',
        );
        $this->zm_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_zonomizm_key',
            conditions: ['method' => 'dns_zonomi'],
            help_text: 'Zonomi API Key',
        );
        $this->zone_username = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_zonezone_username',
            conditions: ['method' => 'dns_zone'],
            help_text: 'Zone.ee Username',
        );
        $this->zone_key = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_zonezone_key',
            conditions: ['method' => 'dns_zone'],
            help_text: 'Zone.ee API Key',
        );
        $this->zilore_key = new StringField(
            default: '',
            allow_empty: true,
            write_only: true,
            internal_name: 'dns_zilorezilore_key',
            conditions: ['method' => 'dns_zilore'],
            help_text: 'Zilore API Key',
        );
        $this->technitium_server = new StringField(
            default: '',
            allow_empty: true,
            internal_name: 'dns_technitiumtechnitium_server',
            conditions: ['method' => 'dns_technitium'],
            help_text: 'Technitium DNS Server address',
        );
        $this->technitium_token = new StringField(
            default: '',
            allow_empty: true,
            sensitive: true,
            internal_name: 'dns_technitiumtechnitium_token',
            conditions: ['method' => 'dns_technitium'],
            help_text: 'Technitium DNS Server API Token',
        );
        $this->anydnschallengealias = new StringField(
            default: '',
            allow_empty: true,
            conditions: ['!method' => ['webroot', 'webrootftp', 'standalone', 'standalonetls']],
            help_text: '(Optional) Adds the --challenge-alias flag to the acme.sh call.<br/>To use a CNAME for _acme-challenge.importantDomain.tld to direct the acme validation to a different (sub)domain _acme-challenge.aliasDomainForValidationOnly.tld, configure the alternate domain here.<br/>More information can be found <a href="https://github.com/acmesh-official/acme.sh/wiki/DNS-alias-mode" target="_new">here</a>.',
        );
        $this->anydnschallengedomain = new BooleanField(
            default: false,
            conditions: ['!method' => ['webroot', 'webrootftp', 'standalone', 'standalonetls']],
            help_text: '(Optional) Uses the challenge domain alias value as --domain-alias instead in the acme.sh call.',
        );

        parent::__construct($id, $parent_id, $data, ...$options);
    }

    /**
     * Returns the available 'method' choices for the method field.
     * @return array The available 'method' choices.
     */
    public static function get_method_choices(): array {
        global $acme_domain_validation_method;
        return array_keys($acme_domain_validation_method);
    }

    /**
     * Iterates through the DNS providers/verification methods supported by pfSense and returns Field definitions
     * for each. This is used to dynamically generate the PHP code required, but does not actually define the Field
     * properties. They must be added and committed to the repository manually.
     * @return array An array of Field definitions for each DNS provider/verification method.
     */
    public static function generate_method_fields_php(): array {
        global $acme_domain_validation_method;
        $properties = [];
        $fields = [];

        foreach ($acme_domain_validation_method as $method_name => $method_attrs) {
            foreach ($method_attrs['fields'] as $field_attrs) {
                $is_dns = str_starts_with($method_name, 'dns');
                $prop_name = $is_dns ? $field_attrs['name'] : "{$method_name}{$field_attrs['name']}";
                $field_type = $field_attrs['type'];

                $properties[] =
                    'public ' . ($field_type === 'checkbox' ? 'BooleanField' : 'StringField') . " \$$prop_name;";
                $field =
                    "\$this->{$prop_name} = new " .
                    ($field_type === 'checkbox' ? 'BooleanField' : 'StringField') .
                    "(\n";
                $field .= '    default: ' . ($field_type === 'checkbox' ? 'false' : "''") . ",\n";

                if ($field_type === 'select') {
                    $choices = json_encode(array_keys($field_attrs['items']));
                    $field .= "    choices: $choices,\n";
                }

                if ($field_type === 'password') {
                    $field .= "    write_only: true,\n";
                }

                $field .= '    allow_empty: ' . ($field_type !== 'checkbox' ? 'true' : 'false') . ",\n";

                if ($is_dns) {
                    $field .= "    internal_name: '$method_name{$field_attrs['name']}',\n";
                }

                $field .= "    conditions: ['method' => '$method_name'],\n";
                $field .= "    help_text: '{$field_attrs['description']}',\n";
                $field .= ');';

                $fields[] = $field;
            }
        }

        return ['properties' => $properties, 'fields' => $fields];
    }
}
