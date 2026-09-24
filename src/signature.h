#pragma once
// Firmware signature check: ECDSA P-256 over SHA-256 of the image, DER signature,
// public key in include/signing_key.h. Feed the image in chunks while it is written.

#include <stddef.h>
#include <stdint.h>

namespace owl::signature {

class Verifier {
public:
    Verifier();
    ~Verifier();
    void update(const uint8_t* data, size_t len);
    // True if sig (DER) is a valid signature of everything passed to update().
    bool finish(const uint8_t* sig, size_t sigLen);

private:
    void* sha_;  // mbedtls_sha256_context (kept out of this header)
};

}  // namespace owl::signature
