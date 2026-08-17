# MURMUR P01-A — final Cirkit build/upload guide

This directory is the source-of-truth package for constructing the P01-A validation project in Cirkit Designer.

## Build
1. Create one blank Cirkit project.
2. Use `CIRKIT_AI_BUILD_SPECIFICATION.md` as the authoritative construction prompt.
3. Preserve every RP2350 GPIO and named net in `PHYSICAL_NETLIST.md`.
4. Build U1 first: Pico 2/RP2350.
5. Add LIS2DU12/LIS3DH, AS7343, DRV2605L/haptic actuator, 3.3-V microSD, interlock, FAULT_IN, timer A, timer B, load switch, low-voltage resistor/LED test load, feedback and test points.
6. Add U2 Bruce as a separate ESP32-S3 block. Connect only BRUCE_TX, BRUCE_RX, BRUCE_3V3 and the required reference ground.
7. Never connect Bruce to LOAD_EN, INTERLOCK_PROOF, TIMER_A_OK, TIMER_B_OK, FAULT_IN or the final load-switch authority.

## Firmware
- U1: `murmur_p01a_controller.ino`
- U2: `bruce_murmur_node.ino` when the selected ESP32-S3 simulation target is supported.

## Validation
Exercise:
- reset/power-up
- self-test
- safe idle
- arm
- active test-load state
- automatic timeout
- cooldown
- interlock removal
- Timer A proof removal
- Timer B proof removal
- FAULT_IN assertion
- LOAD_FB mismatch
- USER_TEST release
- Bruce reset
- Bruce disconnect

Every permission failure must force LOAD_EN LOW.

## Physical upload
Select the exact connected board and port. Do not infer the final Bruce UART pins from the generic block; assign them only after the physical Bruce ESP32-S3 board/module is fixed and its pinout verified.

## Production gate
Cirkit validates logical topology and low-voltage behaviour. It does not prove production optical power, current regulation, thermal behaviour, PCB clearance, connector ratings or enclosure performance.
