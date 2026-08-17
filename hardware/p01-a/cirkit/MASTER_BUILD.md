# MURMUR P01-A — Cirkit Master Build v2

Authoritative integration package for the complete P01-A controller, communications, sensing, haptics, storage, interlock and low-voltage validation system.

## 1. System boundaries

### U1 — Deterministic controller
- Raspberry Pi Pico 2 / RP2350 non-wireless
- Owns sensing, state machine, logging, diagnostics and permission request
- **Only U1 can request LOAD_EN**
- U1 does not rely on wireless communications to maintain the fail-closed path

### U2 — Bruce communications node
- ESP32-S3 based communications/UI module
- Bruce is an **operator/telemetry interface only**
- Bruce may request diagnostics, display state and relay non-authoritative commands
- Bruce must never directly drive the test-load permission path
- Bruce cannot satisfy interlock, timer or fault proofs
- Loss, reboot, brownout or disconnection of Bruce must leave P01-A safe

## 2. Exact RP2350 GPIO map

| Net | GPIO | Direction | Function |
|---|---:|---|---|
| I2C_SDA | GP4 | I/O | Shared I2C |
| I2C_SCL | GP5 | I/O | Shared I2C |
| LOAD_EN | GP6 | OUT | Firmware permission request |
| INTERLOCK_PROOF | GP7 | IN | Physical interlock permission |
| FAULT_IN | GP8 | IN | Independent active-low inhibit |
| TIMER_A_OK | GP9 | IN | Hardware timeout proof A |
| TIMER_B_OK | GP10 | IN | Hardware timeout proof B |
| LOAD_FB | GP11 | IN | Switched-load feedback |
| SD_MISO | GP16 | IN | microSD SPI |
| SD_CS | GP17 | OUT | microSD chip select |
| SD_SCK | GP18 | OUT | microSD SPI clock |
| SD_MOSI | GP19 | OUT | microSD SPI data |
| HAPTIC | GP20 | OUT | DRV2605L control/driver interface |
| USER_TEST | GP21 | IN | Bench test input, active-low |
| STATUS | GP25 | OUT | Status LED |

No GPIO may be repurposed to provide a missing safety proof in software.

## 3. Shared I2C bus

- U3 LIS2DU12 preferred / LIS3DH fallback: 0x18 or 0x19 as configured
- U4 AS7343: 0x39
- U5 DRV2605L: 0x5A
- One logical pull-up pair, nominal 4.7k to 3V3 unless the selected validated modules already provide the required pull-ups
- Local 100nF bypass at each IC/module plus appropriate bulk capacitance

## 4. Storage

U6 microSD, 3.3-V-compatible interface only:
- GP16 MISO / DO
- GP17 CS
- GP18 SCK
- GP19 MOSI / DI

Storage failure must not inhibit safe shutdown or be required for the interlock chain.

## 5. Haptics

U5 DRV2605L owns the actuator output. The RP2350 does not drive a vibration motor directly. Haptic failure must not create load permission.

## 6. Hardware permission chain

The permission path is intentionally distributed:

`Firmware request -> LOAD_EN -> hardware interlock proof -> Timer A proof AND Timer B proof -> controlled low-voltage load switch -> TEST_LOAD`

`FAULT_IN` is an independent inhibit.

Any missing proof must remove permission. The firmware continuously observes all available proofs and immediately drops LOAD_EN on proof loss, timeout or feedback disagreement.

## 7. Hardware timers

U7 and U8 are two independent LTC6993 timer/proof stages.

They remain independent physical channels. Software may observe them but cannot manufacture their proof signals.

Cirkit should represent each as a labelled functional block where an exact simulation model is unavailable:
- TIMER_A_OK
- TIMER_B_OK

Exact timing resistor/capacitor values must be selected from the final target active interval and validated against the actual purchased devices; do not substitute a second software timer.

## 8. Final low-voltage load stage

U9 TPS22919B or validated equivalent is the final switched low-voltage test-load stage.

The simulator uses a resistor/LED or similarly benign load. The production optical/high-power stage is represented only as an explicitly labelled connector/block boundary and is **not** claimed to be validated by Cirkit simulation.

## 9. Bruce interface boundary

Bruce is intentionally outside the safety authority.

Logical interface:
- telemetry/status RX/TX
- diagnostic request
- operator UI state
- optional authenticated configuration transport

Required properties:
- Bruce cannot directly assert hardware permission
- Bruce cannot bypass interlock or hardware timers
- Bruce cannot clear a latched hardware fault
- removing Bruce leaves U1 functioning locally
- booting U1 with Bruce disconnected remains a valid safe state
- the interface defaults to non-authoritative/no-permission on reset

Exact UART/I2C pin assignment for Bruce is kept in the dedicated `BRUCE_INTEGRATION.md` rather than inventing pins not fixed by the selected ESP32-S3 board.

## 10. Power domains

### LOGIC_3V3
Supplies U1, sensors, storage interface and validated logic peripherals.

### BRUCE_3V3
Dedicated ESP32-S3/Bruce supply domain, independently switched/isolated at the architecture level from the permission chain.

### TEST_LOAD_VSW
Separate switched low-voltage test-load net downstream of U9.

The load must never be powered from an RP2350 GPIO.

## 11. Test points

TP_3V3
TP_GND
TP_SDA
TP_SCL
TP_LOAD_EN
TP_INTERLOCK
TP_FAULT
TP_TIMER_A
TP_TIMER_B
TP_LOAD_FB
TP_BRUCE_TX
TP_BRUCE_RX
TP_BRUCE_PWR
TP_TEST_LOAD

## 12. Required Cirkit result

Create one coherent project with:
- U1 RP2350/Pico 2
- U2 ESP32-S3/Bruce block
- U3 LIS2DU12/LIS3DH
- U4 AS7343
- U5 DRV2605L + haptic actuator
- U6 microSD
- U7 LTC6993-A functional block
- U8 LTC6993-B functional block
- U9 TPS22919B/validated equivalent
- interlock/fault inputs
- low-voltage test load
- all test points
- labels and notes

No unexplained dangling active pins, no floating required inputs, and no safety proof generated only by firmware.
