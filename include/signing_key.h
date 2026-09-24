#pragma once
// Public half of the firmware signing key (ECDSA P-256). Not secret. The private key lives in
// gitignored keys/owl-signing.pem (and as a CI secret); tools/sign-firmware.py uses it.
// Replacing this key means every owl must first get a firmware with the new key, signed with the old one.

namespace owl::config {

constexpr const char* SIGNING_PUBLIC_KEY =
    "-----BEGIN PUBLIC KEY-----\n"
    "MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAE71CdYDkrULKV2fJaTL4HfY/VHIBU\n"
    "z/wzHyg+dxx4ng+Yyn3SAALM59NdBptEPT2nR0n3GQErjVHqqEeCiSXZ0w==\n"
    "-----END PUBLIC KEY-----\n";

}  // namespace owl::config
