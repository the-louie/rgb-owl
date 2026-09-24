#!/usr/bin/env python3
"""Signs a firmware image for owl updates: ECDSA P-256 over SHA-256, DER signature.

Usage: tools/sign-firmware.py <firmware.bin> [key.pem]   -> writes <firmware.bin>.sig
Key: argument, $OWL_SIGNING_KEY (PEM text, as in CI), or keys/owl-signing.pem.
The signature is checked against include/signing_key.h before the script succeeds.
"""
import os
import re
import subprocess
import sys
import tempfile

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


def public_key_pem():
    text = open(os.path.join(ROOT, "include", "signing_key.h")).read()
    return "\n".join(re.findall(r'"([^"]*)\\n"', text)) + "\n"


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    image = sys.argv[1]
    sig = image + ".sig"
    with tempfile.TemporaryDirectory() as tmp:
        key = sys.argv[2] if len(sys.argv) > 2 else None
        if not key and os.environ.get("OWL_SIGNING_KEY"):
            key = os.path.join(tmp, "key.pem")
            with open(key, "w") as f:
                f.write(os.environ["OWL_SIGNING_KEY"])
        key = key or os.path.join(ROOT, "keys", "owl-signing.pem")
        subprocess.run(["openssl", "dgst", "-sha256", "-sign", key, "-out", sig, image], check=True)

        pub = os.path.join(tmp, "pub.pem")
        with open(pub, "w") as f:
            f.write(public_key_pem())
        ok = subprocess.run(["openssl", "dgst", "-sha256", "-verify", pub, "-signature", sig, image],
                            capture_output=True, text=True)
        if ok.returncode != 0:
            os.remove(sig)
            sys.exit("signature does not match include/signing_key.h (wrong key?)")
    print(sig)


if __name__ == "__main__":
    main()
