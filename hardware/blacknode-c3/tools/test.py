#!/usr/bin/env python3
"""Portable logic/profile tests. This does not compile the Arduino firmware."""
from pathlib import Path
import shutil
import subprocess
import tempfile

root = Path(__file__).resolve().parents[1]
compiler = shutil.which('g++')
if not compiler:
    raise SystemExit('g++ is required for the portable tests.')
logs = []
with tempfile.TemporaryDirectory(prefix='murmur-core-') as temporary:
    target = str(Path(temporary) / 'test')
    base = [compiler, '-std=c++17', '-Wall', '-Wextra', '-Werror']
    subprocess.run(base + ['-fsanitize=address,undefined', str(root/'tests/core_test.cpp'), '-o', target], check=True)
    logs.append(subprocess.check_output([target], text=True).strip())
    for profile in (1, 2, 3, 4):
        for band in (315, 433, 868, 915):
            flags = ['-DCONFIG_IDF_TARGET_ESP32C3=1', '-DARDUINO_USB_CDC_ON_BOOT=1', f'-DMURMUR_PROFILE={profile}', f'-DMURMUR_RF_BAND={band}']
            subprocess.run(base + flags + [str(root/'tests/profile_test.cpp'), '-o', target], check=True)
            logs.append(subprocess.check_output([target], text=True).strip())
    for flags in ([], ['-DCONFIG_IDF_TARGET_ESP32C3=1'], ['-DCONFIG_IDF_TARGET_ESP32C3=1','-DARDUINO_USB_CDC_ON_BOOT=1','-DMURMUR_PROFILE=5'], ['-DCONFIG_IDF_TARGET_ESP32C3=1','-DARDUINO_USB_CDC_ON_BOOT=1','-DMURMUR_RF_BAND=100']):
        result = subprocess.run(base + flags + [str(root/'tests/profile_test.cpp'), '-o', target], capture_output=True, text=True)
        if result.returncode == 0:
            raise RuntimeError(f'Invalid configuration unexpectedly accepted: {flags}')
    logs.append('PASS: wrong target, disabled USB CDC, invalid profile and invalid band are rejected.')
text = '\n'.join(logs) + '\n'
(root/'build-evidence').mkdir(exist_ok=True)
(root/'build-evidence/host-tests.txt').write_text(text)
print(text, end='')
