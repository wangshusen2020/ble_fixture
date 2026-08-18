#!/usr/bin/env bash
# Build Nordic Blinky UF2 for nice!nano (used by CI and local).
set -euo pipefail
BOARD="${1:-nice_nano_v2}"
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

if [[ ! -d .west ]]; then
  west init -l config
  west update --narrow
  west zephyr-export
fi

west build -s app -b "$BOARD" -d "build-$BOARD" -- \
  -DBOARD_ROOT="$ROOT/config" \
  -DCONF_FILE="$ROOT/app/prj.conf" \
  -DDTC_OVERLAY_FILE="$ROOT/app/app.overlay"

echo "UF2: $ROOT/build-$BOARD/zephyr/zephyr.uf2"
