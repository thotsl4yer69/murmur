# MURMUR P01-A — CIRKIT MASTER BUILD v1.0

## Scope

Single coherent Cirkit Designer bench/simulation design for the complete P01-A control system. The simulation validates controller behaviour, sensor interfaces, haptics, storage interface, power-state logic, independent interlock proof, fault/kill behaviour, watchdog recovery, and a bounded low-power simulated load.

The physical optical/high-power actuator is intentionally NOT represented as an executable high-power circuit. Cirkit's load output terminates at a low-power LED/resistor surrogate. The purpose is to validate logic and connectivity without energising a hazardous physical load.

## Design hierarchy

1. USB/5V input
2. 3V3 regulated logic rail
3. RP2350/Pico 2 non-wireless controller
4. I2C perception bus
   - LIS3DH/LIS2DU12-class motion sensor
   - AS7343 spectral/ambient sensor
   - DRV2605L haptic controller + vibration motor surrogate
5. SPI logging bus
   - microSD module / simulation-ready SPI storage substitute
6. Independent control/interlock section
   - physical-enable simulation
   - hardware-permission simulation
   - timeout/disable logic represented as isolated digital stages
   - MCU command
   - interlock proof
   - fault/kill
7. Low-power simulated load indicator
8. Status and diagnostics
9. Test points

## Canonical GPIO map

| GPIO | Signal | Electrical role |
|---|---|---|
| GP4 | I2C_SDA | bidirectional I2C |
| GP5 | I2C_SCL | I2C clock |
| GP16 | SD_MISO | SPI input |
| GP17 | SD_CS | SPI chip select |
| GP18 | SD_SCK | SPI clock |
| GP19 | SD_MOSI | SPI output |
| GP20 | HAPTIC_ENABLE | haptic control |
| GP21 | TEST_INPUT | active-low test input |
| GP22 | LOAD_CMD | controller permission request |
| GP23 | INTERLOCK_PROOF | active-high healthy proof |
| GP24 | FAULT_KILL | active-low kill |
| GP25 | STATUS_LED | status indicator |

## Power domains

### LOGIC_3V3

Powers RP2350, I2C sensors, haptic controller logic and SD logic. Every module gets local 100 nF bypass; modules with bulk requirements get an additional 4.7–47 uF local capacitor appropriate to the actual module.

### LOAD_SIM

Low-power simulated load only. GP22 does not directly represent a real actuator drive stage; it is a logic-state indicator in the simulation.

## Bus rules

- All I2C devices share GP4/GP5.
- Only one effective pair of I2C pull-ups should be present in the assembled simulation unless a module's pull-ups are explicitly disabled.
- SPI uses GP16–GP19.
- SD CS must remain HIGH while the SD device is not selected.
- Never connect a 5 V signal directly to an RP2350 GPIO.

## Input rules

TEST_INPUT: GP21 internal pull-up, button to GND.

FAULT_KILL: GP24 internal pull-up, fault switch to GND. LOW is unsafe/faulted.

INTERLOCK_PROOF: GP23 must be driven HIGH by the simulated healthy-interlock source. Loss of proof is treated as unsafe.

## Control truth table

| Interlock | Fault | Test | LOAD_CMD |
|---|---|---|---|
| 0 | X | X | 0 |
| X | 0 | 0 | 0 |
| 1 | 1 | X | 0 |
| 1 | 0 | 1 | 1 only during bounded ACTIVE interval |
| 1 | 0 | 0 | 0 |

At boot, reset, watchdog recovery and fault recovery LOAD_CMD must default LOW.

## State machine

BOOT -> SELFTEST -> IDLE -> ARMED -> ACTIVE -> COOLDOWN -> IDLE

Any state -> FAULT if FAULT_KILL is asserted or INTERLOCK_PROOF is lost.

FAULT -> SELFTEST only after fault clears. There is no automatic return to ACTIVE.

ACTIVE is bounded to one second in simulation. The timer is deliberately conservative and exists only to validate timeout behaviour.

## Test points

TP1 = 5V input
TP2 = 3V3 logic rail
TP3 = GND
TP4 = I2C SDA
TP5 = I2C SCL
TP6 = SPI SCK
TP7 = LOAD_CMD
TP8 = INTERLOCK_PROOF
TP9 = FAULT_KILL
TP10 = HAPTIC_ENABLE

## Required Cirkit presentation

Arrange the design left-to-right:

POWER -> CONTROLLER -> SENSORS/STORAGE -> INTERLOCK/CONTROL -> LOAD SIMULATION

Use explicit net labels instead of relying solely on visual wire crossings. Use GND and 3V3 power symbols. Keep the interlock/control section visually separated from the sensor and logging sections.

## Exact-part handling

Where Cirkit has an exact simulation-ready component, use it. Where an exact P01-A component is diagram-only, place the exact part as the documentation component and a clearly labelled simulation substitute alongside it. Do not silently substitute a different electrical interface.

## Acceptance criteria

- No floating power pins.
- No unlabelled cross-domain signal.
- No 5 V GPIO exposure.
- No accidental GPIO contention.
- LOAD_CMD is LOW at boot.
- LOAD_CMD cannot become HIGH without healthy interlock and clear fault.
- Fault assertion forces LOAD_CMD LOW immediately.
- Clearing a fault does not automatically reactivate.
- Interlock loss forces LOAD_CMD LOW.
- Watchdog/reset leaves LOAD_CMD LOW.
- Test activation automatically terminates.
- Every external interface has a documented pin assignment.
- The simulation contains visible test controls and indicators.
