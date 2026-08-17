# MURMUR P01-A — second-pass design review

Review performed against the complete Cirkit package after construction.

## Findings addressed

1. **Controller boundary:** RP2350/Pico 2 remains the deterministic controller. Wireless communications are not part of the permission chain.
2. **Sensor bus:** one shared I2C bus is explicitly defined, with addresses documented and pull-ups treated as a single bus resource.
3. **Storage:** SPI pins are explicitly mapped and SD power is constrained to a 3.3-V-compatible interface.
4. **Haptics:** DRV2605L, not the MCU GPIO, owns the motor output.
5. **Independent hardware timing:** two timer-proof signals remain independent; firmware cannot satisfy them in software.
6. **Fail-closed inputs:** interlock/timer permissions default to not-permitted; FAULT_IN is active-low.
7. **Load control:** LOAD_EN defaults LOW and the simulated load is separated from logic power.
8. **Feedback:** LOAD_FB is independent of LOAD_EN so a commanded-vs-observed mismatch becomes diagnosable.
9. **Testability:** required rails, buses and permission signals have explicit test points.
10. **Simulation boundary:** high-power optical/current/thermal behaviour is deliberately not claimed as simulated validation.

## Firmware review

The controller performs safe output initialization before peripheral initialization. ACTIVE is bounded, proof conditions are continuously checked during the active path, and timeout/proof loss drives LOAD_EN LOW. The FAULT state holds the load disabled.

## Remaining engineering gates

These are intentionally not guessed because they depend on the exact purchased hardware: regulator part/derating, LTC6993 timing resistor/capacitor values, TPS22919B operating conditions, connector ratings, SD module circuitry, haptic motor characteristics, PCB stack-up/clearance, emitter electrical characteristics and enclosure thermal behaviour.

## Acceptance sequence

1. Run Cirkit topology/simulation with the low-voltage test load.
2. Confirm every permission path independently removes LOAD_EN.
3. Confirm reset starts with LOAD_EN LOW.
4. Confirm sensor bus addresses and SD SPI wiring.
5. Build the controller-only PCB/bench harness.
6. Validate hardware timer and interlock stages independently.
7. Validate the low-voltage switched load.
8. Only after those gates pass, design and validate the separate production optical/load stage.
