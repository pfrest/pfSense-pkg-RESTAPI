<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Model;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\Certificate;
use RESTAPI\Models\CertificateAuthority;
use RESTAPI\Models\CertificateAuthorityGenerate;
use RESTAPI\Models\CertificateGenerate;
use RESTAPI\Models\CertificateRevocationList;
use RESTAPI\Models\CertificateRevocationListRevokedCertificate;

class APIModelsCertificateRevocationListRevokedCertificateTestCase extends TestCase {
    /**
     * @var CertificateAuthority|Model $ca The CertificateAuthority model to use for testing.
     */
    private CertificateAuthority|Model $ca;

    /**
     * @var Certificate|Model $cert The Certificate model to use for testing.
     */
    private Certificate|Model $cert;

    /**
     * @var CertificateRevocationList|Model $crl The CertificateRevocationList model to use for testing.
     */
    private CertificateRevocationList|Model $crl;

    /**
     * Sets up the test cases by ensuring a CA and certificate are generated
     */
    public function setup(): void {
        $this->ca = new CertificateAuthorityGenerate(
            descr: 'test_ca',
            trust: true,
            randomserial: true,
            is_intermediate: false,
            keytype: 'RSA',
            keylen: 2048,
            digest_alg: 'sha256',
            lifetime: 3650,
            dn_country: 'US',
            dn_state: 'UT',
            dn_city: 'Salt Lake City',
            dn_organization: 'ACME Org',
            dn_organizationalunit: 'IT',
        );
        $this->ca->create();
        $this->ca = CertificateAuthority::query(refid: $this->ca->refid->value)->first();

        $cert = new CertificateGenerate(
            descr: 'testcert',
            caref: $this->ca->refid->value,
            keytype: 'RSA',
            keylen: 2048,
            digest_alg: 'sha256',
            lifetime: 3650,
            type: 'user',
            dn_commonname: 'testcert.example.com',
        );
        $cert->create();
        $this->cert = Certificate::query(descr: 'testcert')->first();

        $this->crl = new CertificateRevocationList(
            descr: 'test_internal_ca',
            caref: $this->ca->refid->value,
            method: 'internal',
            lifetime: 730,
            serial: 1,
        );
        $this->crl->create();
    }

    /**
     * Cleans up the test cases by deleting the CA and certificate
     */
    public function teardown(): void {
        $this->cert->delete();
        $this->ca->delete();
    }

    /*
     * Ensures revoked certificates can be created, updated and deleted
     */
    public function test_create_update_and_delete(): void {
        # Ensure we can revoke the certificate
        $revoked_cert = new CertificateRevocationListRevokedCertificate(
            parent_id: $this->crl->id,
            certref: $this->cert->refid->value,
            reason: 0,
        );
        $revoked_cert->create();

        # Ensure the revoked certificate details were populated
        $this->assert_equals($revoked_cert->descr->value, $this->cert->descr->value);
        $this->assert_is_not_empty($revoked_cert->revoke_time->value);

        # Reload the CRL and keep track of this CRL iteration
        $this->crl->from_internal();
        $crl_serial = $this->crl->serial->value;
        $crl_text = $this->crl->text->value;

        # Update the revoked certificate's reason and ensure the CRL is updated
        $revoked_cert->reason->value = 1;
        $revoked_cert->update();
        $this->crl->from_internal();
        $this->assert_not_equals($crl_serial, $this->crl->serial->value);
        $this->assert_not_equals($crl_text, $this->crl->text->value);

        # Delete the revoked certificate
        $revoked_cert->delete();
    }

    /**
     * Ensures revoked certificates can't be created for external CRLs
     */
    public function test_external_crl_rejection(): void {
        $this->assert_throws_response(
            response_id: 'CERTIFICATE_REVOCATION_LIST_REVOKED_CERTIFICATE_PARENT_NOT_INTERNAL',
            code: 406,
            callable: function () {
                $revoked_cert = new CertificateRevocationListRevokedCertificate(
                    parent_id: $this->crl->id,
                    certref: $this->cert->refid->value,
                    reason: 0,
                );
                $revoked_cert->parent_model->method->value = 'external';
                $revoked_cert->validate_extra();
            },
        );
    }

    /**
     * Ensures revoked certificates with non-numeric reasons are converted to -1 (unspecified) when reading #864
     */
    public function test_non_numeric_reason_conversion(): void {
        # Mock a revoked certificate with a non-numeric reason
        Model::set_config("crl/{$this->crl->id}/cert/0", [
            'certref' => $this->cert->refid->value,
            'reason' => 'some-non-numeric-reason',
            'revoke_time' => time(),
        ]);

        # Read the CRL and ensure the reason was converted to -1
        $crl_revoked_cert = CertificateRevocationListRevokedCertificate::query(
            parent_id: $this->crl->id,
            id: 0,
        )->first();
        $this->assert_equals(-1, $crl_revoked_cert->reason->value);
    }
}
