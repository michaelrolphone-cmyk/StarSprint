#!/usr/bin/env bash
set -euo pipefail

repo_root="$(cd "$(dirname "$0")/.." && pwd)"
rom_path="${repo_root}/starsprint.sfc"
dist_path="${repo_root}/dist/starsprint.sfc"
backup_path="${repo_root}/starsprint.sfc.test-backup"

cleanup() {
    rm -f "${dist_path}"
    if [ -f "${backup_path}" ]; then
        mv "${backup_path}" "${rom_path}"
    else
        rm -f "${rom_path}"
    fi
}
trap cleanup EXIT

if [ -f "${rom_path}" ]; then
    cp "${rom_path}" "${backup_path}"
fi

printf "STARSPRINT-BUNDLE-TEST\n" > "${rom_path}"

make -C "${repo_root}" bundle-prebuilt

cmp "${rom_path}" "${dist_path}"

echo "test_bundle_prebuilt: ok"
