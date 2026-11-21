<?php

namespace RESTAPI\Tests;

use RESTAPI\Core\TestCase;
use RESTAPI\Models\Certificate;
use RESTAPI\Models\CertificatePKCS12Export;

class APIModelsCertificatePKCS12ExportTestCase extends TestCase {
    /**
     * Ensure the get_internal() method returns an empty array. This model has no internal representation.
     */
    public function test_get_internal(): void {
        $model = new CertificatePKCS12Export();
        $this->assert_equals([], $model->get_internal());
    }

    /**
     * Ensure the _create() method correctly converts the certificate to a PKCS12 archive.
     */
    public function test_create_with_pass(): void {
        # Obtain the default certificate
        $cert = new Certificate(id: 0);

        # Create a PKCS12 archive from the certificate
        $model = new CertificatePKCS12Export(certref: $cert->refid->value, encryption: 'high', passphrase: 'testpass');
        $model->create();

        # Ensure we can load the PKCS12 archive
        $pkcs12 = openssl_pkcs12_read($model->binary_data->value, $certs, 'testpass');
        $this->assert_not_equals(false, $pkcs12);
    }

    /**
     * Ensure the `passphrase` field is hidden from representations by default.
     */
    public function test_passphrase_is_sensitive(): void {
        $model = new CertificatePKCS12Export(passphrase: 'testpass');
        $this->assert_is_false(array_key_exists('passphrase', $model->to_representation()));
    }
}
