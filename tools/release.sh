#!/usr/bin/env bash
# Builds everything a GitHub release carries into dist/release/:
#   owl-firmware.bin (+ .sig)   OTA image for the owl's updater, signed (tools/sign-firmware.py)
#   owl-s3zero-merged.bin       full flash image for USB / the web flasher (address 0x0)
#   owl-app.apk                 release-signed Android app
# Local: keys/ + secrets.ini. CI: the same via env (see .github/workflows/ci.yml).
set -euo pipefail
cd "$(dirname "$0")/.."
out=dist/release
rm -rf "$out" && mkdir -p "$out"

pio test -e native
pio run -e s3zero
b=.pio/build/s3zero
cp "$b/firmware.bin" "$out/owl-firmware.bin"
tools/sign-firmware.py "$out/owl-firmware.bin" > /dev/null
boot_app0="${PLATFORMIO_CORE_DIR:-$HOME/.platformio}/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
pio pkg exec --package tool-esptoolpy -- esptool.py --chip esp32s3 merge_bin -o "$out/owl-s3zero-merged.bin" \
    --flash_mode dio --flash_freq 80m --flash_size 4MB \
    0x0 "$b/bootloader.bin" 0x8000 "$b/partitions.bin" 0xe000 "$boot_app0" 0x10000 "$b/firmware.bin" > /dev/null

tools/build-app.sh release > /dev/null
cp dist/owl-app-release.apk "$out/owl-app.apk"

python3 tools/version.py > "$out/VERSION"
ls -l "$out"
