#!/usr/bin/env bash
# Regenerates data/x509_crt_bundle.bin (root CAs for HTTPS update downloads) from the current
# Mozilla CA list, in ESP-IDF's bundle format (tools/crt/gen_crt_bundle.py from ESP-IDF v4.4.7).
set -euo pipefail
cd "$(dirname "$0")/crt"
curl -sfL -o cacert.pem https://curl.se/ca/cacert.pem
python3 gen_crt_bundle.py --input cacert.pem
mv x509_crt_bundle ../../data/x509_crt_bundle.bin
rm cacert.pem
echo "data/x509_crt_bundle.bin"
