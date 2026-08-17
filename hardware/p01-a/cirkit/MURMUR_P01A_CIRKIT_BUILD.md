# MURMUR P01-A — Cirkit Designer complete build

## Purpose

This is the complete **bench/simulation** system definition for the P01-A controller. It is deliberately arranged so the sensor, logging, haptic, state-machine and multi-stage power-control logic can be validated in Cirkit before the physical prototype is assembled.

The high-power optical load is represented by a low-power indicator/load substitute in simulation. Do not connect a real high-power optical emitter to the simulator/test outputs.

## Controller

**Raspberry Pi Pico 2 / RP2350, non-wireless**

### GPIO allocation

| GPIO | Function | Direction |
|---|---|---|
| GP4 | I2C SDA | I/O |
| GP5 | I2C SCL | I/O |
| GP16 | SPI MISO / microSD | input |
| GP17 | microSD CS | output |
| GP18 | SPI SCK / microSD | output |
| GP19 | SPI MOSI / microSD | output |
| GP20 | DRV2605L haptic enable/test | output |
| GP21 | bench/test input | input, active LOW |
| GP22 | optical/load enable command | output |
| GP23 | interlock proof input | input |
| GP24 | fault/kill input | input |
| GP25 | status LED | output |

## I2C devices

| Device | Address | Purpose |
|---|---:|---|
| LIS3DH | 0x18/0x19 | motion |
| AS7343 | 0x39 | ambient spectral sensing |
| DRV2605L | 0x5A | haptic feedback |

All three devices use the same 3.3 V I2C bus with common SDA/SCL and appropriate pull-ups.

## Power architecture

For simulation, model these as logical stages rather than connecting a real high-power load:

`5V/USB -> 3V3 controller rail`

`external load rail -> master enable -> redundant timeout stage -> firmware enable -> simulated load`

The firmware must never regard the load as enabled unless the interlock proof input is asserted.

### Logical interlock

1. Hardware/master enable is OFF at boot.
2. Firmware output GP22 remains LOW until all preconditions pass.
3. GP23 must indicate the external interlock chain is healthy.
4. GP24 is an active fault/kill input and immediately forces GP22 LOW.
5. Any reset, watchdog event or fault returns GP22 LOW.
6. Cirkit simulation uses an LED/resistor or low-power indicator as the load surrogate.

## User/test controls

### TEST button

- One side -> GP21
- Other side -> GND
- Use internal pull-up.
- Pressing the button runs a bounded 1-second controller test.

### Fault switch

- One side -> GP24
- Other side -> GND
- Use internal pull-up.
- LOW means FAULT/kill asserted.

### Interlock switch

- One side -> GP23
- Other side -> 3V3
- Use a pull-down or equivalent logic source.
- HIGH means the simulated hardware interlock is proven healthy.

## Indicators

### Status LED

`GP25 -> 330R resistor -> LED anode`

`LED cathode -> GND`

### Simulated load

`GP22 -> 1k resistor -> LED anode`

`LED cathode -> GND`

This LED is only a **load-state indicator** in simulation.

## microSD

SPI bus:

- GP16 -> MISO
- GP17 -> CS
- GP18 -> SCK
- GP19 -> MOSI
- 3V3 -> VCC
- GND -> GND

Logging format should be line-oriented CSV/JSONL in the physical firmware. Simulation prints equivalent records to Serial.

## State machine

`BOOT -> SELFTEST -> IDLE -> ARMED -> ACTIVE -> COOLDOWN -> IDLE`

Any state may transition to `FAULT` when GP24 is LOW.

`FAULT -> IDLE` only after the fault input clears and the controller performs a fresh self-test.

The simulated ACTIVE state is strictly bounded to 1 second.

## Cirkit component strategy

Use Cirkit's **Simulation Ready** filter wherever an exact part is available. Cirkit supports a large component library but simulation support is per-part, so an exact sensor/module may be diagram-only. In that case, use a simulation-ready functional substitute and retain the exact P01-A part as a documentation component.

Recommended simulation set:

- Raspberry Pi Pico/Pico-compatible programmable controller
- pushbutton
- slide/toggle switches
- LEDs
- 330 ohm resistor
- 1 kohm resistor
- I2C functional sensor substitutes where required
- SPI/microSD functional substitute where available
- buzzer/piezo as haptic surrogate
- power rails and GND

## Validation sequence

1. Verify firmware compiles.
2. Run simulation with interlock healthy and fault clear.
3. Confirm boot enters SELFTEST then IDLE.
4. Press TEST and confirm bounded ACTIVE -> COOLDOWN -> IDLE.
5. Confirm GP22/load indicator remains OFF when GP23 is not proven.
6. Assert GP24 and confirm GP22 immediately goes LOW.
7. Clear GP24 and confirm the system does not automatically reactivate.
8. Exercise I2C scan.
9. Exercise haptic output surrogate.
10. Exercise SD/logging path if the selected simulator part supports it.
11. Verify watchdog/fault paths.

## Physical integration boundary

Cirkit is the controller and low-power functional-validation environment. The physical prototype should be assembled subsystem-by-subsystem after the simulated state machine and interlock logic pass.
