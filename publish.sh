#!/usr/bin/env bash
# Builds and publishes a release of the owl (firmware + app) to GitHub.
#
#   ./publish.sh 1.2.0          build here, push main + tag v1.2.0, create the GitHub release
#   ./publish.sh 1.3.0-rc.1     same, published as a pre-release (owls take it only in debug mode)
#   ./publish.sh 1.2.0 --ci     only push main + tag; GitHub Actions builds and publishes
#                               (needs the repo secrets listed in .github/workflows/ci.yml)
#
# Needs: clean tree on main, secrets.ini, keys/ (signing keys), `gh auth login` (local mode).
# Assets: owl-firmware-V.bin(.sig), owl-s3zero-merged-V.bin, owl-app-V.apk (see tools/release.sh).
set -euo pipefail
cd "$(dirname "$0")"

die() { echo "publish: $*" >&2; exit 1; }

version="${1:-}"
mode="${2:-local}"
[[ "$version" =~ ^[0-9]+\.[0-9]+\.[0-9]+(-[0-9A-Za-z.]+)?$ ]] || die "usage: ./publish.sh X.Y.Z[-pre] [--ci]"
[[ "$mode" == local || "$mode" == --ci ]] || die "unknown option: $mode"
tag="v$version"
pre=""
[[ "$version" == *-* ]] && pre="--prerelease"

# --- checks -------------------------------------------------------------------------------------
[ "$(git branch --show-current)" = main ] || die "not on main"
[ -z "$(git status --porcelain --untracked-files=no)" ] || die "uncommitted changes"
git rev-parse -q --verify "refs/tags/$tag" > /dev/null && die "tag $tag already exists"
git remote get-url origin > /dev/null 2>&1 || die "no 'origin' remote"
[ -f secrets.ini ] || die "secrets.ini missing (copy secrets.ini.example)"
[ -f keys/owl-signing.pem ] || [ "$mode" = --ci ] || die "keys/owl-signing.pem missing"
[ -f keys/owl-app.properties ] || [ "$mode" = --ci ] || die "keys/owl-app.properties missing"
if [ "$mode" = local ]; then gh auth status > /dev/null 2>&1 || die "run 'gh auth login' first"; fi

# The repo is public: refuse to push history that contains a secret from secrets.ini.
for key in ota_password ble_pin; do
    val=$(sed -n "s/^$key *= *//p" secrets.ini)
    [ -n "$val" ] || continue
    if [ -n "$(git log --all -S"$val" --format=%h | head -1)" ]; then
        die "$key from secrets.ini appears in git history; scrub it before publishing, e.g.:
  git filter-branch --tree-filter \"sed -i 's/$val/REDACTED/g' platformio.ini || true\" -- --all
  (rewrites every later commit hash; then re-run ./publish.sh)"
    fi
done

# --- build --------------------------------------------------------------------------------------
git tag -a "$tag" -m "owl $tag"
trap 'git tag -d "$tag" > /dev/null 2>&1 || true' ERR  # undo the tag if anything below fails
if [ "$mode" = local ]; then
    tools/release.sh
    built=$(cat dist/release/VERSION)
    [ "$built" = "$version" ] || die "built version $built != $version"
fi

# --- publish ------------------------------------------------------------------------------------
git push origin main
git push origin "$tag"
trap - ERR
if [ "$mode" = local ]; then
    gh release create "$tag" $pre --verify-tag --generate-notes --title "owl $tag" \
        $(ls dist/release/*.bin dist/release/*.sig dist/release/*.apk)
    echo "published $tag"
else
    echo "pushed $tag; GitHub Actions builds and publishes it"
fi
