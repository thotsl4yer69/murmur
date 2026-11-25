#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
command -v arduino-cli >/dev/null || { echo 'Install Arduino CLI before running this script.' >&2; exit 1; }
INDEX=https://espressif.github.io/arduino-esp32/package_esp32_index.json
arduino-cli core update-index --additional-urls "$INDEX"
arduino-cli core install esp32:esp32@3.3.12 --additional-urls "$INDEX"
arduino-cli lib update-index
arduino-cli lib install 'Adafruit GFX Library@1.12.6' 'Adafruit ST7735 and ST7789 Library@1.11.0' 'NimBLE-Arduino@2.5.1' 'RadioLib@7.7.1' 'U8g2@2.37.1'
mkdir -p build-evidence
arduino-cli version | tee build-evidence/cli.txt
arduino-cli core list | tee build-evidence/cores.txt
arduino-cli lib list | tee build-evidence/libraries.txt
FQBN='esp32:esp32:esp32c3:CDCOnBoot=cdc,PartitionScheme=huge_app,FlashMode=dio,FlashSize=4M'
for profile in ${*:-1 2 3 4}; do
  case "$profile" in 1|2|3|4) ;; *) echo 'Profile must be 1, 2, 3 or 4.' >&2; exit 2;; esac
  mkdir -p "build/profile-$profile"
  arduino-cli compile --fqbn "$FQBN" --warnings all \
    --build-property "compiler.cpp.extra_flags=-DMURMUR_PROFILE=$profile" \
    --output-dir "build/profile-$profile" MURMUR_C3 2>&1 | tee "build-evidence/profile-$profile.log"
done
find MURMUR_C3 build -type f \( -name '*.h' -o -name '*.ino' -o -name '*.bin' \) -exec sha256sum {} \; > build-evidence/SHA256SUMS.txt
