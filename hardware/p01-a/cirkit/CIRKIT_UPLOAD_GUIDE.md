# MURMUR P01-A — Cirkit build/upload guide

## What this package is

This directory is the complete source-of-truth package for constructing the P01-A validation project in Cirkit Designer.

Cirkit Designer currently supports circuit construction, firmware simulation, ESP32/Pico-class projects, custom parts and browser-based firmware upload. Use the master specification to build the project rather than relying on a generic AI-generated wiring result.

## Files to use

1. `MASTER_BUILD.md` — architecture and boundaries.
2. `CIRKIT_AI_BUILD_SPECIFICATION.md` — paste into Cirkit AI.
3. `PHYSICAL_NETLIST.md` — authoritative wiring/net names.
4. `BRUCE_INTEGRATION.md` — Bruce/ESP32-S3 boundary.
5. `murmur_p01a_controller.ino` — RP2350 controller firmware.
6. `bruce_murmur_node.ino` — Bruce ESP32-S3 communications firmware.
7. `BUILD_COVERAGE_MATRIX.md` — completeness/engineering-gate matrix.
8. `FINAL_DESIGN_REVIEW.md` — second-pass review and remaining physical gates.

## Cirkit construction sequence

### A. Create the project

Create one new blank Cirkit project.

### B. Build U1 first

Add Pico 2/RP2350. If the exact Pico 2 component is unavailable in simulation, use the closest supported Pico simulation symbol but preserve the exact GPIO assignments.

### C. Add all U1 peripherals

Add:
- LIS2DU12 or LIS3DH
- AS7343
- DRV2605L + haptic actuator
- 3.3-V microSD
- physical interlock input
- active-low fault input
- timer A proof block
- timer B proof block
- final low-voltage load switch block
- resistor/LED test load
- load feedback block
- test points

### D. Add Bruce

Add an ESP32-S3 simulation MCU if supported by the current Cirkit component library. Label the block `BRUCE` and connect only BRUCE_TX, BRUCE_RX, BRUCE_3V3 and the required reference ground.

Do NOT wire Bruce into the safety permission path.

### E. Run firmware

Load `murmur_p01a_controller.ino` on U1.
Load `bruce_murmur_node.ino` on U2 if the Cirkit ESP32-S3 simulation target supports it.

### F. Validate

Exercise reset, self-test, safe idle, arm, active timeout, cooldown and fault states. Then independently force each proof low and verify that LOAD_EN returns LOW.

Finally disconnect/restart Bruce and verify U1 remains locally safe.

## Important distinction

Cirkit validates topology, logical behaviour and firmware/simulation interactions. It does not prove the physical electrical, thermal, PCB, optical or high-power properties of the production emitter subsystem.
