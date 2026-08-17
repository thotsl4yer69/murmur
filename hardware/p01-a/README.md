# MURMUR P01-A hardware bring-up

This directory is the first executable hardware layer for the P01-A prototype.

## Controller

- Raspberry Pi Pico 2 / RP2350, non-wireless
- 3.3 V GPIO
- I2C: GP4 SDA / GP5 SCL
- SPI: GP16 MISO / GP18 SCK / GP19 MOSI / GP17 CS
- Haptic test output: GP20
- Bench test input: GP21, active LOW
- Status LED: GP25

## Planned I2C devices

| Device | Address | Bus |
|---|---:|---|
| LIS3DH | 0x18 or 0x19 | I2C |
| AS7343 | 0x39 | I2C |
| DRV2605L | 0x5A | I2C |

The current bench firmware intentionally does **not** drive any optical/high-power actuator. It is for controller, bus, state-machine and haptic bring-up.

## Browser test

The `wokwi/` directory contains a simulator circuit using the Pico pin layout, a pushbutton, a low-volume buzzer as the haptic substitute, and a status LED. The original Pico and Pico 2 share the relevant GPIO layout, so this is suitable for logic/wiring validation before moving the exact RP2350 target into the physical build.

Open Wokwi, create a Raspberry Pi Pico/Pico 2 project, then copy in:

- `wokwi/sketch.ino`
- `wokwi/diagram.json`

Run the simulation and use the serial monitor.

Commands:

- `I` — I2C scan
- `H` — haptic test
- `A` — 1-second active-state test
- `R` — return to idle
- `?` — command help

## Real hardware firmware

`firmware/murmur_p01a_bench.ino` is the corresponding bench firmware for the physical Pico 2. The next integration step is replacing the simulated sensor path with validated LIS3DH, AS7343 and DRV2605L drivers, then adding the remaining P01-A power/interlock hardware one subsystem at a time.
