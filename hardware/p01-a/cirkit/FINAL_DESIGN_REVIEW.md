# MURMUR P01-A — third-pass integration review

Review performed after adding the Bruce ESP32-S3 boundary, coverage matrix and Cirkit upload package.

## Confirmed included

1. RP2350/Pico 2 non-wireless deterministic controller.
2. Bruce ESP32-S3 communications/operator node as a separate non-authoritative subsystem.
3. Shared I2C bus for LIS2DU12/LIS3DH, AS7343 and DRV2605L.
4. 3.3-V-compatible SPI microSD interface.
5. Haptic driver boundary; motor is not directly driven by a Pico GPIO.
6. Physical interlock proof input.
7. Independent fault input.
8. Two independent LTC6993 timer/proof channels.
9. Final switched low-voltage validation-load stage.
10. Independent load feedback.
11. Explicit power-domain labels.
12. Explicit test points including Bruce telemetry/power points.
13. Cirkit AI construction specification.
14. Bruce firmware stub implementing a non-authoritative serial/operator interface.
15. Coverage matrix identifying which parts are architecture-complete versus physical-engineering gates.
16. Upload/build instructions for reconstructing the project in Cirkit.

## Bruce audit

Bruce was previously missing from the repository. It is now explicitly represented as U2 and has its own integration contract and firmware.

Bruce is not allowed to:
- assert LOAD_EN;
- satisfy interlock proof;
- satisfy either timer proof;
- clear FAULT_IN;
- bypass the RP2350 state machine.

Loss of Bruce must not make the load permissive.

## Firmware audit

The earlier sketch was not as complete as the architecture documentation implied. In particular, hardware drivers for the selected sensors/SD card, persistent logging and a production watchdog path were not actually present.

Therefore the package now distinguishes:
- controller/state-machine firmware that is currently executable;
- device-driver integration that must be added against the exact selected libraries/modules;
- Bruce communications firmware;
- physical engineering parameters that must not be guessed.

## Wiring audit

Checked for:
- RP2350 GPIO/net-name consistency;
- I2C address duplication risk;
- SPI mapping;
- active-low fault semantics;
- default-off load control;
- independent timer proof signals;
- Bruce isolation from load permission;
- absence of GPIO-powered test load;
- explicit test points;
- floating-input risk at the logical architecture level.

## Remaining gates before calling the physical build final

These cannot honestly be completed without the exact purchased parts/datasheets:
- exact LTC6993 timing networks;
- exact regulator and protection circuitry;
- exact TPS22919B implementation/conditions;
- exact ESP32-S3/Bruce module and verified UART pinout;
- exact DRV2605L module and motor electrical requirements;
- exact SD module circuitry;
- production optical emitter electrical/current/thermal design;
- connector ratings;
- PCB stack-up, trace widths, return paths and clearance;
- enclosure thermal behaviour.

## Acceptance order

1. Construct Cirkit topology from the authoritative specification.
2. Run controller simulation.
3. Verify each fault/proof path independently disables LOAD_EN.
4. Verify reset and Brownout-safe defaults.
5. Add Bruce and verify its loss has no permissive effect.
6. Compile/run the controller firmware against the selected simulated target.
7. Validate each physical module on the bench individually.
8. Freeze exact part numbers and electrical values.
9. Produce the production PCB/netlist from the verified physical selections.
10. Validate the separate production optical/load stage independently.
