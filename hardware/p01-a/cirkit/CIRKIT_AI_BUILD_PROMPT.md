# Cirkit Designer AI build prompt — MURMUR P01-A

Paste the following into Cirkit AI in a blank project.

---

Build a Raspberry Pi Pico/Pico-compatible P01-A controller circuit for the MURMUR prototype as a **simulation-safe bench system**.

Use these exact controller connections:

- GP4 = I2C SDA
- GP5 = I2C SCL
- GP16 = SPI MISO
- GP17 = microSD CS
- GP18 = SPI SCK
- GP19 = SPI MOSI
- GP20 = haptic output surrogate
- GP21 = TEST pushbutton, active LOW
- GP22 = simulated load enable output
- GP23 = interlock proof input, HIGH means healthy
- GP24 = fault/kill input, LOW means fault
- GP25 = status LED

Add these functional blocks:

1. Raspberry Pi Pico/Pico-compatible programmable controller.
2. I2C sensor block representing LIS3DH at 0x18/0x19.
3. I2C spectral sensor block representing AS7343 at 0x39.
4. I2C haptic driver block representing DRV2605L at 0x5A.
5. SPI microSD block using GP16/17/18/19.
6. TEST pushbutton from GP21 to GND.
7. FAULT toggle from GP24 to GND with pull-up logic.
8. INTERLOCK toggle driving GP23 HIGH when healthy.
9. STATUS LED on GP25 through 330 ohm resistor to GND.
10. SIMULATED LOAD LED on GP22 through 1 kohm resistor to GND.
11. Piezo/buzzer or equivalent low-power indicator as the haptic simulation surrogate.
12. 3.3 V and GND rails.

Important simulation constraint:
The load LED is only a logical representation of the controlled load. Do not model or connect a high-power optical emitter in the simulator.

Implement the controller state machine:
BOOT -> SELFTEST -> IDLE -> ACTIVE -> COOLDOWN -> IDLE
Any state -> FAULT when FAULT is asserted.
FAULT -> SELFTEST after the fault clears.

ACTIVE must be bounded to 1 second.
The load enable must remain LOW unless the interlock is healthy and no fault is asserted.
A fault must immediately force the load enable LOW.
Reset/boot must force the load enable LOW.

Use this firmware from the repository:
`hardware/p01-a/cirkit/murmur_p01a_cirkit.ino`

Use the simulation-ready equivalents when the exact LIS3DH, AS7343, DRV2605L or microSD part is not simulation-enabled. Keep the exact component names as documentation labels where possible.

After creating the circuit, verify that:

- firmware compiles;
- TEST produces a 1-second ACTIVE pulse followed by COOLDOWN;
- load indicator never activates without interlock proof;
- FAULT immediately disables the load;
- clearing FAULT requires a fresh SELFTEST;
- status LED indicates controller state;
- serial output reports state, interlock, fault and load status;
- I2C scan executes without crashing.

Arrange the circuit in functional zones: POWER, CONTROLLER, SENSORS, LOGGING, HAPTICS, INTERLOCK, and SIMULATED LOAD.

Add labels to every important net and GPIO connection.

---
