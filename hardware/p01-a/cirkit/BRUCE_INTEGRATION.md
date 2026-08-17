# MURMUR P01-A — Bruce integration

Bruce is the ESP32-S3 communications/operator node. It is deliberately outside the deterministic permission authority.

## Bruce responsibilities
- operator UI/telemetry
- diagnostics requests
- state display
- non-authoritative command transport

## Bruce prohibitions
Bruce must never directly:
- assert LOAD_EN
- satisfy INTERLOCK_PROOF
- satisfy TIMER_A_OK or TIMER_B_OK
- clear FAULT_IN
- switch the test-load rail
- become a boot dependency

A Bruce reset, disconnect, brownout or firmware failure must leave U1 in its existing fail-closed state.

## Logical interface
- BRUCE_TX -> U1 telemetry RX
- BRUCE_RX <- U1 telemetry TX
- BRUCE_3V3 -> Bruce supply
- BRUCE_GND -> interface reference

Exact ESP32-S3 module GPIOs are intentionally not guessed. They are selected only after the physical Bruce board/module is fixed.

## Integration rule
All authoritative state remains local to the RP2350. Bruce can request an operation; U1 independently verifies every required permission before doing anything with the load-control path.
