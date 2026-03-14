<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\Command;
use RESTAPI\Core\Model;
use RESTAPI\Core\TestCase;
use RESTAPI\Models\CertificateAuthority;

class APIModelsCertificateAuthorityTestCase extends TestCase {
    const EXAMPLE_CRT = "-----BEGIN CERTIFICATE-----
MIIDnzCCAoegAwIBAgIUd9R9uoNKE2GoxgJPwzRn67WV4bgwDQYJKoZIhvcNAQEL
BQAwXzELMAkGA1UEBhMCVVMxDTALBgNVBAgMBFV0YWgxFzAVBgNVBAcMDlNhbHQg
TGFrZSBDaXR5MRIwEAYDVQQKDAlBY21lIEluYy4xFDASBgNVBAMMC2V4YW1wbGUu
Y29tMB4XDTIzMTIyMzAxNTIzMFoXDTMzMTIyMDAxNTIzMFowXzELMAkGA1UEBhMC
VVMxDTALBgNVBAgMBFV0YWgxFzAVBgNVBAcMDlNhbHQgTGFrZSBDaXR5MRIwEAYD
VQQKDAlBY21lIEluYy4xFDASBgNVBAMMC2V4YW1wbGUuY29tMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAsOO3ABmSowO9LxYb4nN6QSaj2wrEJLyyXW0s
w3wPYkskQ+LFSCK1nBfJJXzPL3JqeBYksIppjyVCBWHSjbDN4DCy4zWheBWiNX0N
MMEyRgCefcmyJhw0GCcG3LQdB4lwlNOQiNQ7cd7H7Tl4ufIWaq4GZeJlF0VDvwb7
suDvhEB1UgtXIaAy2+JAFp+hV/x5PcaKnppu2OgWMEGXY0pMC3cRq/cfPvDolrQI
rlOxE1+XTlL7xylu/SVg/aHCHjgRKa9G4mpMQuDMnvn6TaJZRwF0K+jmCpjk6qTY
Uqxuc+Ua1OMCqUHO3fVEY0s+ULkVL5nFItaaUGdKcrPn8hEvOwIDAQABo1MwUTAd
BgNVHQ4EFgQUR5oK7sWvKs8QOsNnnRXvVGxvLkgwHwYDVR0jBBgwFoAUR5oK7sWv
Ks8QOsNnnRXvVGxvLkgwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOC
AQEAYCfLDfnHTKWipng64tNWAvwgiIj2IsdfPibypgJiuWAY1kDL6Vz328N6Jr2T
Ljs2M8HlP+Xo8CxSx2I2NHSWhad8eFKOX4AhOjTk0fDMDBe1AlRuLTNlyBDdWAMI
7B5RbeCuIlHqwy7DefTtzVkHHoG8vB/hY7jukOIlxnyjmwTbFNBHmeo7+BmWhxQ2
5JPfqJelyprR6OA+CpZhu4bLjwS1J6iOaVMOgnvzGVHLD5o7cFv+bLoc/VyfTn2v
cU/K9lj+3VxX+W3Vj7L7xSY9UnGfF2trcokXIzNirVHEPDLUQDWZ1YyqQBHPBYVV
BL1l1UjlU4Vb8ubjQ1/ISq95dw==
-----END CERTIFICATE-----";
    const EXAMPLE_PRV = "-----BEGIN PRIVATE KEY-----
MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQCw47cAGZKjA70v
Fhvic3pBJqPbCsQkvLJdbSzDfA9iSyRD4sVIIrWcF8klfM8vcmp4FiSwimmPJUIF
YdKNsM3gMLLjNaF4FaI1fQ0wwTJGAJ59ybImHDQYJwbctB0HiXCU05CI1Dtx3sft
OXi58hZqrgZl4mUXRUO/Bvuy4O+EQHVSC1choDLb4kAWn6FX/Hk9xoqemm7Y6BYw
QZdjSkwLdxGr9x8+8OiWtAiuU7ETX5dOUvvHKW79JWD9ocIeOBEpr0biakxC4Mye
+fpNollHAXQr6OYKmOTqpNhSrG5z5RrU4wKpQc7d9URjSz5QuRUvmcUi1ppQZ0py
s+fyES87AgMBAAECggEAAeQ3wDxO6VDnRsNDQlr+wgbf9mKU31mxe22NpXwNn3zz
NUCgKZCpczLqxTXHO/OT5gSwT+PQuG7GfL0++Q+cktGifcYyQFD5MLUWr4T96o62
IyZXxGazO56Y7a/zE817pTPCOtMwb0B+PcuBLFWOlM68ATVTm0Lhj6RLQpMNmlMN
2w/AKGTzK4e4mzsgH/8N2b9uwgCtrDeB1HWQ8nKC9ZGeL0ZYmNvVKo5Umlsh+Y/1
PRwmaGciaap9Rrki4fHM+tHU33roIC3B6MRLdma0sLE4UoJ6IGgApL2ox4gX+qJy
qvitGzu9Oq3f0ZVuXM/Tk9zAIiZhFiY3ISNRvDppgQKBgQDfji4HO8/4geKhLf8K
UZxfpkVxTLj4J8b6T0olPsfe4Cj3vJu3PCjn3bT8+n6Px72Ub6HMbomSRZAnJUW2
i5VNWRT6NQhcLN566n9O4HJLoT7OkXoJ1SRAT6X3hsxcyZtjDNGgIhsOJ64dHce/
uu161aB1cqj0Jk1yoJiTfcEOuwKBgQDKj75jzlenbaJduCp3dbpOyjRHUdNjNmZR
Ja6GC/iO54NRKUXLt18gEzpFdPCxrQl9t7/lHi+6m6WU1Sfss3+Xxx8fIp8Pwggh
w1R2uZrzHzRhtAaEG9ZRTjJ7pEk/I/YBQ2OTdaWoUZKdl+B1O7sgWWJj16cUHFFf
MYWnVcGZgQKBgQCqFD2oezTqV8bX4WqbqyNREerbHAfNpFWZsiEy3OSrB9ejQvcl
HLvyS5aasOkA8nGlMJjkM2SuyAW4An9bNi7Mve1bvmUx22EYGrfohpHuQCrMdZln
SaVe2Dnoa8GnBisV3HbPYaM7zayFslZ54/kw7r45HJa1yT466zupWo86aQKBgQC7
t2ZbG9gDYwh7DbbdKg7UAXNdPKlHYhyDywXnV6Sq2kXJqxPxbCgNk4QG5g5Aufj3
tpdw/v6AUsmqGbd8FgSCZZejxPPfKJNcpqXZuLut3xcuDCWVvC5q/qquBluS1gKe
vRLEjtxMFfxna14phoctvWf+2tUNQAjwzPXtUv2tAQKBgAUsNWjqUE4WnI/Gu0II
bSIrTCXwoMS91HTKG5P1OhjHP7WH5hhKOlWc8SFPMn1ivu760n7F1qBkj4BOiDvy
4qYNpDCL82eUEfRtCiFmfsERhOnXRcaud8zQj/fNIOcvx5qADI1ErAJBzgp5ZGJp
E0InZI+5q1RzueYy8+j0FX/o
-----END PRIVATE KEY-----";
    const EXAMPLE_ECPRV = "-----BEGIN EC PRIVATE KEY-----
MHcCAQEEIM52UGyF/b3kF1wcPGeEf0/npV7YpgUJBhU4zsOfRowvoAoGCCqGSM49
AwEHoUQDQgAE1RtITpFIce3jjv2AkBqYaVRU0swODWO+DREC/xsYnaNSyVGT5hy6
R02Pul8ulWQ8Kl3Q3pou8As7W1mMzA2DxQ==
-----END EC PRIVATE KEY-----";

    /**
     * Checks that a CertificateAuthority cannot have `crt` and `prv` values that are not a matching key pair.
     */
    public function test_crt_and_prv_are_matching_keypair() {
        $this->assert_throws_response(
            response_id: 'CERTIFICATE_AUTHORITY_CRT_AND_PRV_NOT_KEYPAIR',
            code: 400,
            callable: function () {
                $cert = new CertificateAuthority(
                    data: [
                        'descr' => 'test',
                        'crt' => self::EXAMPLE_CRT,
                        'prv' => self::EXAMPLE_ECPRV,
                    ],
                );
                $cert->validate();
            },
        );

        $this->assert_does_not_throw(
            callable: function () {
                $cert = new CertificateAuthority(
                    data: [
                        'descr' => 'test',
                        'crt' => self::EXAMPLE_CRT,
                        'prv' => self::EXAMPLE_PRV,
                    ],
                );
                $cert->validate();
            },
        );
    }

    /**
     * Checks that the CA is found in the OS trust store when `trust` is enabled.
     */
    public function test_ca_trust() {
        # Create a CA to test with
        $ca = new CertificateAuthority(
            data: [
                'descr' => 'test',
                'crt' => self::EXAMPLE_CRT,
                'prv' => self::EXAMPLE_PRV,
                'trust' => true,
            ],
        );
        $ca->create();

        # Ensure the cert is in the trust store directory
        $truststore_dir_ls = glob('/usr/local/etc/ssl/certs/*.crt');
        $this->assert_is_not_empty($truststore_dir_ls, message: 'Trust store directory should have one trusted CA!');

        # Disable `trust`
        $ca->trust->value = false;
        $ca->update();

        # Ensure the certificate is not found in the trust store
        $truststore_dir_ls = glob('/usr/local/etc/ssl/certs/*.crt');
        $this->assert_is_empty($truststore_dir_ls, message: 'Trust store directory should have no trusted CAs!');

        # Delete the CA
        $ca->delete();
    }

    /**
     * Checks that we cannot delete a CA that is in use.
     */
    public function test_cannot_delete_ca_in_use(): void
    {
        # Create a CA to test with
        $ca = new CertificateAuthority(
            descr: 'test',
            crt: self::EXAMPLE_CRT,
            prv: self::EXAMPLE_PRV,
        );
        $ca->create();

        # Mock an OpenVPN server using this CA to be in use
        Model::set_config(path: "openvpn/openvpn-server/0/caref", value: $ca->refid->value);

        # Ensure an error is thrown if we try to delete the CA while it's in use
        $this->assert_throws_response(
            response_id: 'CERTIFICATE_AUTHORITY_CANNOT_BE_DELETED_WHILE_IN_USE',
            code: 409,
            callable: function () use ($ca) {
                $ca->delete();
            },
        );
    }
    # TODO: Need test to ensure crt must be CA capable
}
