<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\Certificate;
use RESTAPI\Models\CertificateAuthority;
use RESTAPI\Models\FreeRADIUSEAP;
use RESTAPI\Responses\ValidationError;

class APIModelsFreeRADIUSEAPTestCase extends TestCase {
    public array $required_packages = ['pfSense-pkg-freeradius3'];

    /**
     * Reads the rendered FreeRADIUS EAP module configuration file from disk.
     * Returns an empty string if the file does not exist (e.g. when the package has not yet
     * rendered the eap module config).
     */
    private function read_eap_conf(): string {
        foreach (
            [
                '/usr/local/etc/raddb/mods-enabled/eap',
                '/usr/local/etc/raddb/mods-available/eap',
                '/usr/local/etc/raddb/eap.conf',
            ]
            as $candidate
        ) {
            if (file_exists($candidate)) {
                return file_get_contents($candidate);
            }
        }
        return '';
    }

    /**
     * Resets the EAP module configuration back to its default state. Called after every test that
     * mutates the singleton so subsequent tests start from a known baseline.
     */
    private function reset_eap(): void {
        $eap = new FreeRADIUSEAP();
        $eap->update();
    }

    /**
     * Checks that the singleton FreeRADIUSEAP model returns its defaults on first read.
     */
    public function test_defaults_round_trip(): void {
        $eap = new FreeRADIUSEAP();
        $this->assert_equals($eap->disable_weak_eap_types->value, false);
        $this->assert_equals($eap->default_eap_type->value, 'md5');
        $this->assert_equals($eap->timer_expire->value, 60);
        $this->assert_equals($eap->ignore_unknown_eap_types->value, false);
        $this->assert_equals($eap->cisco_accounting_username_bug->value, false);
        $this->assert_equals($eap->max_sessions->value, 4096);
        $this->assert_equals($eap->tls_min_version->value, '1.0');
        $this->assert_equals($eap->tls_include_length->value, true);
        $this->assert_equals($eap->tls_fragment_size->value, 1024);
        $this->assert_equals($eap->tls_check_cert_issuer->value, false);
        $this->assert_equals($eap->tls_check_cert_cn->value, false);
        $this->assert_equals($eap->cache_enable->value, false);
        $this->assert_equals($eap->cache_lifetime->value, 24);
        $this->assert_equals($eap->cache_max_entries->value, 255);
        $this->assert_equals($eap->ocsp_enable->value, false);
        $this->assert_equals($eap->ocsp_override_cert_url->value, false);
        $this->assert_equals($eap->ocsp_url->value, 'http://127.0.0.1/ocsp/');
        $this->assert_equals($eap->ttls_default_eap_type->value, 'md5');
        $this->assert_equals($eap->ttls_copy_request_to_tunnel->value, false);
        $this->assert_equals($eap->ttls_use_tunneled_reply->value, false);
        $this->assert_equals($eap->ttls_include_length->value, true);
        $this->assert_equals($eap->peap_default_eap_type->value, 'mschapv2');
        $this->assert_equals($eap->peap_copy_request_to_tunnel->value, false);
        $this->assert_equals($eap->peap_use_tunneled_reply->value, false);
        $this->assert_equals($eap->peap_soh_enable->value, 'Disable');
    }

    /**
     * Checks that we can update the singleton FreeRADIUSEAP model and read the values back,
     * and that the upstream EAP module config file reflects those changes after apply().
     */
    public function test_update_and_apply(): void {
        $eap = new FreeRADIUSEAP();
        $eap->default_eap_type->value = 'ttls';
        $eap->timer_expire->value = 120;
        $eap->ignore_unknown_eap_types->value = true;
        $eap->cisco_accounting_username_bug->value = true;
        $eap->max_sessions->value = 2048;
        $eap->tls_min_version->value = '1.2';
        $eap->tls_include_length->value = false;
        $eap->tls_fragment_size->value = 1500;
        $eap->cache_enable->value = true;
        $eap->cache_lifetime->value = 48;
        $eap->cache_max_entries->value = 512;
        $eap->ocsp_enable->value = true;
        $eap->ocsp_override_cert_url->value = true;
        $eap->ocsp_url->value = 'http://ocsp.example.org/check';
        $eap->ttls_default_eap_type->value = 'mschapv2';
        $eap->ttls_copy_request_to_tunnel->value = true;
        $eap->ttls_use_tunneled_reply->value = true;
        $eap->ttls_include_length->value = false;
        $eap->peap_default_eap_type->value = 'gtc';
        $eap->peap_copy_request_to_tunnel->value = true;
        $eap->peap_use_tunneled_reply->value = true;
        $eap->update();

        # Read the singleton back from config and verify values round-trip correctly
        $read = new FreeRADIUSEAP();
        $this->assert_equals($read->default_eap_type->value, 'ttls');
        $this->assert_equals($read->timer_expire->value, 120);
        $this->assert_equals($read->ignore_unknown_eap_types->value, true);
        $this->assert_equals($read->cisco_accounting_username_bug->value, true);
        $this->assert_equals($read->max_sessions->value, 2048);
        $this->assert_equals($read->tls_min_version->value, '1.2');
        $this->assert_equals($read->tls_include_length->value, false);
        $this->assert_equals($read->tls_fragment_size->value, 1500);
        $this->assert_equals($read->cache_enable->value, true);
        $this->assert_equals($read->cache_lifetime->value, 48);
        $this->assert_equals($read->cache_max_entries->value, 512);
        $this->assert_equals($read->ocsp_enable->value, true);
        $this->assert_equals($read->ocsp_override_cert_url->value, true);
        $this->assert_equals($read->ocsp_url->value, 'http://ocsp.example.org/check');
        $this->assert_equals($read->ttls_default_eap_type->value, 'mschapv2');
        $this->assert_equals($read->ttls_copy_request_to_tunnel->value, true);
        $this->assert_equals($read->ttls_use_tunneled_reply->value, true);
        $this->assert_equals($read->ttls_include_length->value, false);
        $this->assert_equals($read->peap_default_eap_type->value, 'gtc');
        $this->assert_equals($read->peap_copy_request_to_tunnel->value, true);
        $this->assert_equals($read->peap_use_tunneled_reply->value, true);

        # Confirm apply() rendered the EAP module configuration file with the new values
        $conf = $this->read_eap_conf();
        if ($conf !== '') {
            $this->assert_str_contains($conf, 'default_eap_type = ttls');
            $this->assert_str_contains($conf, 'timer_expire     = 120');
            $this->assert_str_contains($conf, 'ignore_unknown_eap_types = yes');
            $this->assert_str_contains($conf, 'cisco_accounting_username_bug = yes');
            $this->assert_str_contains($conf, 'max_sessions = 2048');
            $this->assert_str_contains($conf, 'tls_min_version = "1.2"');
            $this->assert_str_contains($conf, 'fragment_size = 1500');
            $this->assert_str_contains($conf, 'include_length = no');
            $this->assert_str_contains($conf, 'enable = yes');
            $this->assert_str_contains($conf, 'lifetime = 48');
            $this->assert_str_contains($conf, 'max_entries = 512');
            $this->assert_str_contains($conf, 'override_cert_url = yes');
            $this->assert_str_contains($conf, 'http://ocsp.example.org/check');
            $this->assert_str_contains($conf, 'default_eap_type = mschapv2');
            $this->assert_str_contains($conf, 'default_eap_type = gtc');
        }

        $this->reset_eap();
    }

    /**
     * Ensures the choice on `default_eap_type` is enforced by the StringField.
     */
    public function test_default_eap_type_choice(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                $eap = new FreeRADIUSEAP(default_eap_type: 'totally-bogus');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures the choice on `ttls_default_eap_type` is enforced by the StringField.
     */
    public function test_ttls_default_eap_type_choice(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                # `peap` is intentionally not in the TTLS allowed set
                $eap = new FreeRADIUSEAP(ttls_default_eap_type: 'peap');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures the choice on `peap_default_eap_type` is enforced by the StringField.
     */
    public function test_peap_default_eap_type_choice(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                $eap = new FreeRADIUSEAP(peap_default_eap_type: 'leap');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures the choice on `tls_min_version` is enforced by the StringField.
     */
    public function test_tls_min_version_choice(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                $eap = new FreeRADIUSEAP(tls_min_version: '1.3');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures the choice on `peap_soh_enable` is enforced by the StringField.
     */
    public function test_peap_soh_enable_choice(): void {
        $this->assert_throws_response(
            response_id: 'FIELD_INVALID_CHOICE',
            code: 400,
            callable: function () {
                $eap = new FreeRADIUSEAP(peap_soh_enable: 'maybe');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures `max_sessions` rejects values below the configured minimum.
     */
    public function test_max_sessions_minimum(): void {
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () {
                $eap = new FreeRADIUSEAP(max_sessions: 0);
                $eap->validate();
            },
        );
    }

    /**
     * Ensures `tls_fragment_size` rejects values above 4096 (the upstream FreeRADIUS limit).
     */
    public function test_tls_fragment_size_maximum(): void {
        $this->assert_throws(
            exceptions: [ValidationError::class],
            callable: function () {
                $eap = new FreeRADIUSEAP(tls_fragment_size: 8192);
                $eap->validate();
            },
        );
    }

    /**
     * Ensures `ocsp_url` requires a valid URL when set.
     */
    public function test_ocsp_url_invalid(): void {
        $this->assert_throws_response(
            response_id: 'URL_VALIDATOR_FAILED',
            code: 400,
            callable: function () {
                $eap = new FreeRADIUSEAP(ocsp_url: 'not a url');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures the upstream `freeradius_validate_eap()` cross-field validation is enforced when
     * `disable_weak_eap_types` is enabled together with a weak `default_eap_type`.
     */
    public function test_validate_extra_rejects_weak_eap_when_disabled(): void {
        $this->assert_throws_response(
            response_id: 'FREERADIUS_EAP_UNEXPECTED_VALIDATION_ERROR',
            code: 400,
            callable: function () {
                $eap = new FreeRADIUSEAP(disable_weak_eap_types: true, default_eap_type: 'md5');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures `ssl_ca_cert`, `ssl_ca_crl` and `ssl_server_cert` are validated against existing
     * Cert Manager refids when set.
     */
    public function test_ssl_cert_foreign_validation(): void {
        $this->assert_throws_response(
            response_id: 'FOREIGN_MODEL_FIELD_VALUE_NOT_FOUND',
            code: 404,
            callable: function () {
                $eap = new FreeRADIUSEAP(ssl_ca_cert: 'doesnotexist1234');
                $eap->validate();
            },
        );
        $this->assert_throws_response(
            response_id: 'FOREIGN_MODEL_FIELD_VALUE_NOT_FOUND',
            code: 404,
            callable: function () {
                $eap = new FreeRADIUSEAP(ssl_server_cert: 'doesnotexist1234');
                $eap->validate();
            },
        );
        $this->assert_throws_response(
            response_id: 'FOREIGN_MODEL_FIELD_VALUE_NOT_FOUND',
            code: 404,
            callable: function () {
                $eap = new FreeRADIUSEAP(ssl_ca_crl: 'doesnotexist1234');
                $eap->validate();
            },
        );
    }

    /**
     * Ensures that an existing CertificateAuthority refid is accepted by `ssl_ca_cert`
     * (verifying the ForeignModelField relationship resolves correctly).
     */
    public function test_ssl_cert_foreign_accepts_existing(): void {
        $cas = CertificateAuthority::read_all();
        if ($cas->count() === 0) {
            return;
        }
        $ca_refid = $cas->first()->refid->value;

        $eap = new FreeRADIUSEAP(ssl_ca_cert: $ca_refid);
        $this->assert_does_not_throw(
            callable: function () use ($eap) {
                $eap->validate();
            },
        );

        $certs = Certificate::read_all();
        if ($certs->count() > 0) {
            $cert_refid = $certs->first()->refid->value;
            $eap2 = new FreeRADIUSEAP(ssl_server_cert: $cert_refid);
            $this->assert_does_not_throw(
                callable: function () use ($eap2) {
                    $eap2->validate();
                },
            );
        }
    }
}
