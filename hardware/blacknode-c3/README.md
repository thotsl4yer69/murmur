# MURMUR BLACKNODE C3 1.0.0

Complete ESP32-C3 SuperMini source for a four-button screen node: passive Wi-Fi/BLE surveys, Wi-Fi AP counts by channel, optional CC1101 RSSI sweep, fixed-profile packet RX, MUR3 lab link tests/replay, RAM event logs, USB CSV export and hardware diagnostics. No Wi-Fi password, API key or cloud account is required.

**Status:** portable core/profile tests pass. The hosted ESP32 build failed before runner steps executed, including a retry. The firmware has NOT been verified by an ESP32 compiler or flashed/tested on physical hardware. No verified binary is supplied. The source is complete; this is not a hardware-qualified release.

## Select the actual display

Set `MURMUR_PROFILE` in `MURMUR_C3/Config.h` for IDE builds; the scripts select it with a compiler flag.

| Profile | Screen | RF |
|---|---|---|
| 1 (default) | ST7789 240x280, landscape 280x240 | No CC1101 pins allocated |
| 2 | Same ST7789; RESET/BL rewired as below | CC1101 shared SPI |
| 3 | SSD1306 128x64 I2C OLED, address 0x3C | Optional CC1101 |
| 4 | SH1106 128x64 I2C OLED, address 0x3C | Optional CC1101 |

This is not an S3 pin map. Profile 1 preserves the recent SportsDeck colour harness. Profiles 3/4 preserve the earlier SDA3/SCL4 OLED harness while moving K4 from GPIO9 to GPIO10.

## Colour wiring

Both profiles: screen VCC to 3V3 on a compatible carrier, GND to GND; SCK=4, MOSI=6, CS=7, DC=10. Buttons are LEFT/UP=0, OK=5, RIGHT/DOWN=20, MODE/BACK=21. Each button switches to common GND; firmware uses pull-ups. The button labels describe function, not an assumed printed K-number.

**Profile 1:** TFT RESET=GPIO3; BL logic-control input=GPIO1. GPIO1 must not power a raw backlight LED; use a carrier's documented logic input/driver.

**Profile 2:** remove the old TFT RESET and BL wires from GPIO3/1. Pull TFT RESET to 3V3 through 10k; tie a documented BL logic input to 3V3, always on. Software reset is used. Then connect a standard 3.3-V SPI CC1101:

| CC1101 | C3 GPIO |
|---|---:|
| SCK | 4 |
| SI/MOSI | 6 |
| SO/MISO | 3 |
| CSn | 2, with 10k pull-up to 3V3 |
| GDO0 | 1, through 1k |
| GDO2 | Not connected |

**Do not flash profile 2 onto profile 1 wiring.** The two former TFT signals are reassigned to radio inputs.

## OLED wiring

VCC=3V3, GND=GND, SDA=3, SCL=4, K1/UP=5, K2/DOWN=6, K3/OK=7, K4/BACK=10. Do not use the old K4=GPIO9 connection.

Optional standard CC1101: SCK=0, SI/MOSI=1, SO/MISO=20, CSn=2 with 10k pull-up to 3V3, GDO0=21 through 1k, GDO2 disconnected.

For either RF harness, add 100nF and 10uF at the radio supply and use the correct band-matched antenna. Power the bench node through the C3 USB input. Power down before changing wires. GPIO18/19 are reserved for native USB. A 1k series resistor on GPIO21 signal/button wiring limits boot-time UART contention.

These power/pin tables apply to standard 3.3-V carriers. An amplified CC1101 carrier's supply, PA enable and RX/TX switching must be identified separately; this source does not invent those connections.

## Build

Keep `MURMUR_C3.ino`, `Node.h`, `Config.h` and `Core.h` together. The .ino is the entry point; the application is in Node.h. Install Arduino-ESP32 3.3.12 and:

| Library | Version |
|---|---:|
| Adafruit GFX Library | 1.12.6 |
| Adafruit ST7735 and ST7789 Library | 1.11.0 |
| NimBLE-Arduino | 2.5.1 |
| RadioLib | 7.7.1 |
| U8g2 | 2.37.1 |

Accept required library dependencies. IDE settings: ESP32C3 Dev Module; USB CDC On Boot enabled; 4MB flash; DIO; Huge APP 3MB/no OTA; no PSRAM. ArduinoDroid must retain all four files and provide the matching C3 core/libraries; its bundled core is not assumed equivalent.

With Arduino CLI installed, run from this directory:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build.ps1 -Profiles 1
arduino-cli board list
.\tools\flash.ps1 -Port COM7 -Profile 1
```

Replace COM7 with the actual enumerated port. The flash helper requires a successfully compiled application. Choose another profile only after matching its wiring. Upload replaces the board's existing firmware.

Linux/macOS:

```bash
bash tools/build.sh 1
```

With no argument, build.sh compiles all four profiles. CLI/core/library versions and compiler output are recorded by the Linux/CI script. For a board not entering the uploader, hold BOOT, tap RESET, release BOOT and select the newly enumerated port.

## Use and test

Start with SYSTEM -> Screen + button test. Verify colour bars on TFT and all four switch states. Hold BACK to exit. Then test Wi-Fi and BLE scans and repeated switching between them. No AP results or BLE advertisements is a real possible result, not a simulated reading.

UP/DOWN selects items/history; OK enters/rescans or pauses/resumes a scope/RX job; BACK exits and stops the job. Holding BACK also stops the active job. Settings persist; survey, packet and event records are RAM-only and clear on reset. OLED titles/status alternate for readability.

The default CC1101 packet profile is 433.92MHz, 2-FSK, 4.8kbps, 5kHz deviation, 58kHz bandwidth, 16-bit preamble, sync `4D52`, variable packets up to 48 bytes, hardware CRC and requested chip TX setting -20dBm. PACKET RX is not an arbitrary-waveform recorder or universal remote decoder.

LAB LINK TEST sends three 20-byte MUR3 test packets one second apart after holding OK for 0.9s. A second matching node in PACKET RX proves reception; sender TX completion alone does not. The alternate mode repeats one previously received CRC-valid MUR3 frame, not arbitrary remote traffic. TX has a 250ms completion timeout.

Scope: 48 tuning points, uncalibrated RSSI, live/peak display. Default window 433.05-434.79MHz. Other compile-time bands are 315/868/915 and need matching modules/antennas. The wider 868/915 windows are discrete samples at 58kHz bandwidth, not gap-free coverage. Wi-Fi channel counts are AP counts, not RF power or channel utilization.

USB serial: 115200 baud, newline commands `help`, `status`, `wifi`, `ble`, `scope`, `rx`, `stop`, `export wifi`, `export ble`, `export packets`, `export events`. Export stops the active job. CSV uses a boot identifier and uptime, not invented wall-clock timestamps. No serial command starts transmission.

## Verification

```bash
python3 tools/test.py
```

This runs actual portable C++ tests: debouncing/long-hold exclusion, millis rollover, bounded storage, sanitization, CRC known-answer/corruption detection, hex bounds, cursor wrap, all 16 profile/band pin-map combinations and invalid-configuration guards. It compiles Core.h/Config.h, not the Arduino application. Physical cold boots, display orientation, radio carrier operation and a 30-minute scan session remain to be demonstrated.

The companion source package contains an expanded offline build guide and the actual verification record. No optics, garment interlocks, camera concealment, jammer/deauth mode, OTA service or complete Bruce port is claimed by this release.
