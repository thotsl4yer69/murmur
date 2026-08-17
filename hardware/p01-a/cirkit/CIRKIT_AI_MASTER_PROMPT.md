# CIRKIT AI MASTER PROMPT — MURMUR P01-A

Build ONE complete, coherent Cirkit Designer project named `MURMUR P01-A — Full System Bench`.

Do not create a simplified example. Use the following architecture exactly unless Cirkit requires a simulation-only equivalent. If an exact part is not simulation-ready, retain the exact part as a documentation component and add a clearly labelled simulation substitute; do not change its electrical interface.

## CONTROLLER

Use Raspberry Pi Pico 2 / RP2350 non-wireless as the intended controller. For simulation, use the closest simulation-ready Pico/Pico-compatible controller if Pico 2/RP2350 is unavailable. Expose GPIO labels:

GP4 I2C_SDA
GP5 I2C_SCL
GP16 SD_MISO
GP17 SD_CS
GP18 SD_SCK
GP19 SD_MOSI
GP20 HAPTIC_ENABLE
GP21 TEST_INPUT
GP22 LOAD_CMD
GP23 INTERLOCK_PROOF
GP24 FAULT_KILL
GP25 STATUS_LED

## I2C BUS

Create one 3.3 V I2C bus:
- SDA = GP4
- SCL = GP5
- common GND
- appropriate pull-ups to 3V3

Devices:
- LIS3DH or exact P01-A motion sensor, address 0x18/0x19
- AS7343, address 0x39
- DRV2605L, address 0x5A

If a device is not simulation-ready, use a simulation-ready I2C substitute while retaining a labelled documentation component named with the exact P01-A device.

## HAPTIC

Connect DRV2605L logic to I2C. Represent the vibration motor with a buzzer/piezo or other simulation-ready low-power actuator. HAPTIC_ENABLE is GP20. Do not connect a high-current motor directly to a GPIO.

## STORAGE

Create SPI:
GP16 MISO
GP17 CS
GP18 SCK
GP19 MOSI
3V3 and GND

Use a simulation-ready microSD/SPI storage component where available; otherwise use a labelled SPI storage substitute. Ensure CS has a defined inactive state.

## POWER

Show USB/5V input, GND and a 3V3 regulated logic rail. Include visible power labels and appropriate decoupling. Do not imply that the simulator validates a physical high-current power supply.

## INTERLOCK / CONTROL

Create a clearly separated logic-control block with:
- MASTER_ENABLE test switch
- INTERLOCK_PROOF healthy-state source
- FAULT_KILL switch
- timeout/permission logic represented as low-voltage digital logic
- LOAD_CMD from GP22
- simulated load indicator

The load indicator must be a low-power LED/resistor or equivalent. Do NOT model or drive a real high-power optical emitter.

Truth requirements:
- LOAD_CMD LOW at boot.
- LOAD_CMD remains LOW unless INTERLOCK_PROOF is healthy, FAULT_KILL is clear and firmware requests activation.
- FAULT_KILL asserted => LOAD_CMD LOW.
- INTERLOCK_PROOF lost => LOAD_CMD LOW.
- Reset/watchdog => LOAD_CMD LOW.
- Clearing a fault does not automatically reactivate the load.

## TEST CONTROLS

TEST_INPUT: momentary pushbutton from GP21 to GND with pull-up.
FAULT_KILL: toggle switch from GP24 to GND with pull-up; LOW means fault.
INTERLOCK_PROOF: toggle/source that drives GP23 HIGH only when the simulated interlock is healthy.

## INDICATORS

STATUS_LED: GP25 through 330 ohm resistor to LED to GND.
LOAD_SIM: GP22 through 1 kohm resistor to LED to GND. Label it `SIMULATED LOAD — NOT PHYSICAL ACTUATOR`.

## TEST POINTS

Add labelled test points TP1 5V, TP2 3V3, TP3 GND, TP4 SDA, TP5 SCL, TP6 SCK, TP7 LOAD_CMD, TP8 INTERLOCK_PROOF, TP9 FAULT_KILL, TP10 HAPTIC_ENABLE.

## LAYOUT

Left-to-right engineering flow:
POWER → CONTROLLER → PERCEPTION/STORAGE → INTERLOCK/CONTROL → SIMULATED LOAD.
Keep buses visually clean. Use net labels. Avoid ambiguous crossings. Put all GND symbols on the common ground net.

## SIMULATION

Make the project runnable. Add serial/debug output if supported. Provide obvious controls for:
1. normal boot
2. test activation
3. interlock open
4. fault assertion
5. fault clear
6. sensor/interface diagnostic

The expected state machine is:
BOOT → SELFTEST → IDLE → ARMED → ACTIVE → COOLDOWN → IDLE.
Any fault/interlock loss → FAULT.
FAULT recovery → SELFTEST, never directly ACTIVE.
ACTIVE duration = 1 second for bench simulation.

## FINAL CHECK

Before finishing, inspect every connection. Verify every controller GPIO is used only for its assigned function, every powered component has GND, every bus has a defined topology, no 5 V signal enters a 3.3 V GPIO, and the simulated load cannot be active without interlock proof and clear fault.

Then provide the completed circuit and simulation-ready code in the project.
