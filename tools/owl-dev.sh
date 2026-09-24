#!/usr/bin/env bash
# Dev loop helper: OTA-flash the owl, wait for it to come back, and switch on debug mode
# inside the 3-minute boot window (otherwise WiFi turns off).
# Usage: tools/owl-dev.sh <ip> [flash|devmode]
set -euo pipefail
cd "$(dirname "$0")/.."
ip="$1"; what="${2:-flash}"
pw=$(sed -n 's/^ota_password *= *//p' secrets.ini)
built=$(python3 tools/version.py)
if [ "$what" = flash ]; then
    pio run -e s3zero-ota -t upload --upload-port "$ip" 2>&1 | grep -E "SUCCESS|FAILED|Error:" | head -3
    sleep 5
fi
for _ in $(seq 1 60); do
    if curl -sf -m 2 -d "password=$pw&on=1" "http://$ip/api/devmode" > /tmp/owl-devmode.json; then
        grep -o '"version":"[^"]*"\|"devmode":[a-z]*\|"wifi_status":"[a-z]*"' /tmp/owl-devmode.json | tr '\n' ' '; echo
        if [ "$what" = flash ] && ! grep -q "\"version\":\"$built\"" /tmp/owl-devmode.json; then
            echo "WARNING: the owl does not run the build just flashed ($built)" >&2
            exit 2
        fi
        exit 0
    fi
    sleep 2
done
echo "owl did not answer on $ip within 120 s" >&2
exit 1
