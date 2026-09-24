#include "signature.h"

#include <mbedtls/pk.h>
#include <mbedtls/sha256.h>
#include <string.h>

#include "signing_key.h"

namespace owl::signature {

Verifier::Verifier() {
    auto* ctx = new mbedtls_sha256_context;
    mbedtls_sha256_init(ctx);
    mbedtls_sha256_starts(ctx, 0);  // 0 = SHA-256 (not 224)
    sha_ = ctx;
}

Verifier::~Verifier() {
    auto* ctx = static_cast<mbedtls_sha256_context*>(sha_);
    mbedtls_sha256_free(ctx);
    delete ctx;
}

void Verifier::update(const uint8_t* data, size_t len) {
    mbedtls_sha256_update(static_cast<mbedtls_sha256_context*>(sha_), data, len);
}

bool Verifier::finish(const uint8_t* sig, size_t sigLen) {
    uint8_t hash[32];
    mbedtls_sha256_finish(static_cast<mbedtls_sha256_context*>(sha_), hash);
    mbedtls_pk_context pk;
    mbedtls_pk_init(&pk);
    const char* pem = config::SIGNING_PUBLIC_KEY;
    bool ok = mbedtls_pk_parse_public_key(&pk, reinterpret_cast<const unsigned char*>(pem), strlen(pem) + 1) == 0 &&
              mbedtls_pk_can_do(&pk, MBEDTLS_PK_ECDSA) &&
              mbedtls_pk_verify(&pk, MBEDTLS_MD_SHA256, hash, sizeof(hash), sig, sigLen) == 0;
    mbedtls_pk_free(&pk);
    return ok;
}

}  // namespace owl::signature
