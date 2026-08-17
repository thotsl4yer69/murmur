# MURMUR P01-A — Cirkit Master Build

Authoritative integration package for the complete low-voltage P01-A controller/test system.

## Architecture
- Raspberry Pi Pico 2 / RP2350 controller (RP2040 equivalent allowed only for simulation)
- shared I2C: LIS2DU12/LIS3DH, AS7343, DRV2605L
- SPI microSD
- haptic and status outputs
- interlock proof
- two independent timer-proof inputs
- fault input
- controlled low-voltage test-load stage
- load feedback
- test points and explicit net labels

## Exact GPIO map
| Net | GPIO |
|---|---:|
| I2C SDA | GP4 |
| I2C SCL | GP5 |
| SD MISO | GP16 |
| SD CS | GP17 |
| SD SCK | GP18 |
| SD MOSI | GP19 |
| HAPTIC | GP20 |
| USER_TEST | GP21 |
| STATUS | GP25 |
| LOAD_EN | GP6 |
| INTERLOCK_PROOF | GP7 |
| FAULT_IN | GP8 |
| TIMER_A_OK | GP9 |
| TIMER_B_OK | GP10 |
| LOAD_FB | GP11 |

## Bus devices
LIS2DU12/LIS3DH: 0x18/0x19. AS7343: 0x39. DRV2605L: 0x5A. Use one shared I2C pull-up pair, nominally 4.7k to 3V3, unless the selected module already contains pull-ups.

## Power
3V3 logic rail. Place 100nF at each IC and 10uF bulk near controller/sensor cluster. Keep the test-load rail explicitly separate from the logic rail in the schematic.

## Permission chain
Firmware request -> LOAD_EN -> interlock permission -> timer A permission AND timer B permission -> test-load switch -> low-voltage test load.

FAULT_IN, failed interlock, either timer proof low, firmware timeout, or load-feedback disagreement must force LOAD_EN low.

## Simulation boundary
The simulated load must be a resistor/LED or equivalent low-voltage load. Do not treat Cirkit simulation as validation of the real high-power optical emitter, current regulator, thermal design, PCB creepage/clearance or connector ratings.

## Test points
TP_3V3, TP_GND, TP_SDA, TP_SCL, TP_LOAD_EN, TP_INTERLOCK, TP_FAULT, TP_TIMER_A, TP_TIMER_B, TP_LOAD_FB.

## Firmware
Use `murmur_p01a_controller.ino`. It initializes all load-control outputs LOW before peripheral setup and implements BOOT -> SELF_TEST -> SAFE_IDLE -> ARMED -> ACTIVE -> COOLDOWN/FAULT with fail-closed proof checks.
