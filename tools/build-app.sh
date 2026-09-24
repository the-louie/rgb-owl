#!/usr/bin/env bash
# Builds and tests the Android app; copies the APK to dist/.
# Usage: tools/build-app.sh [debug|release]   (default debug)
set -euo pipefail
cd "$(dirname "$0")/../android"
export JAVA_HOME="${JAVA_HOME:-$HOME/.local/jdk/current}"
export ANDROID_HOME="${ANDROID_HOME:-$HOME/.local/android-sdk}"
[ -f local.properties ] || echo "sdk.dir=$ANDROID_HOME" > local.properties
variant="${1:-debug}"
V="${variant^}"
./gradlew --no-daemon -q "test${V}UnitTest" "assemble${V}"
mkdir -p ../dist
cp "app/build/outputs/apk/${variant}/app-${variant}.apk" "../dist/owl-app-${variant}.apk"
echo "dist/owl-app-${variant}.apk"
