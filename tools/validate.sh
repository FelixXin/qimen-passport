#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
mode="${1:---all}"
case "$mode" in --all|--static|--firmware) ;; *) echo 'Usage: validate.sh [--all|--static|--firmware]' >&2;exit 2;; esac
if [[ "$mode" != --firmware ]]; then
 python3 tools/check_project.py
 PYTHONDONTWRITEBYTECODE=1 python3 tests/test_project.py
 node tests/test_web.js
 echo 'Host tests: PASS'
fi
if [[ "$mode" != --static ]]; then
 command -v idf.py >/dev/null
 stage=$(mktemp -d /tmp/qimen-firmware.XXXXXX)
 trap 'case "$stage" in /tmp/qimen-firmware.*) rm -rf -- "$stage";; esac' EXIT
 idf.py -B "$stage" -D "SDKCONFIG=$stage/sdkconfig" build
 idf.py -B "$stage" merge-bin -o "$stage/FoloToy-AI-Passport-full.bin"
 python3 tools/verify_firmware.py "$stage"
 python3 tools/archive_firmware.py create "$stage" --archive-root "$PWD/build/firmware"
 mkdir -p build
 cp "$stage/FoloToy-AI-Passport-full.bin" build/FoloToy-AI-Passport-full.bin
 echo 'Firmware build: PASS'
fi
