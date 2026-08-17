# MURMUR P01-A — Bruce integration

## Role

Bruce is the ESP32-S3-based communications/operator node previously discussed for MURMUR. In the P01-A architecture Bruce is a **non-authoritative communications endpoint**.

Bruce can:
- display device state;
- request diagnostics;
- send non-authoritative operator commands to U1;
- relay telemetry to an external UI when a communications link is available.

Bruce cannot:
- directly drive LOAD_EN;
- satisfy INTERLOCK_PROOF;
- satisfy TIMER_A_OK or TIMER_B_OK;
- clear an independent FAULT_IN condition;
- directly switch the test-load rail;
- become a required boot dependency.

## Interface

The logical interface is deliberately named rather than tied to an arbitrary ESP32-S3 dev-board pinout:

- BRUCE_TX -> U1 telemetry RX
- BRUCE_RX <- U1 telemetry TX
- BRUCE_PWR -> BRUCE_3V3
- BRUCE_GND -> reference/common ground where required by the selected interface

When the exact ESP32-S3 board is selected, assign physical UART pins from that board's verified pinout and update this file before physical wiring.

## Reset and failure behaviour

1. Bruce reset: U1 continues in its current locally-valid safe state.
2. Bruce disconnected: U1 continues without Bruce.
3. Bruce brownout: no effect on hardware permission proofs.
4. Invalid command: U1 rejects it.
5. Repeated communication loss: U1 remains locally deterministic.

## Cirkit representation

Create Bruce as a separate ESP32-S3 MCU block. Connect only the telemetry/operator interface nets. Do not connect Bruce to the permission chain.

Cirkit supports ESP32 simulation and running firmware in-browser, so Bruce can be represented as a real MCU in the project where the selected ESP32-S3 variant is supported. The Cirkit project must still preserve the physical RP2350 safety boundary even if simulation uses a generic ESP32-S3 symbol.

## Firmware contract

The RP2350 is authoritative for state. Bruce receives telemetry such as:

`STATE`, `FAULT`, `INTERLOCK`, `TIMER_A`, `TIMER_B`, `LOAD_FB`, `LOAD_EN`.

Operator requests are advisory messages only. U1 applies its own state-machine and proof checks before acting.

Example conceptual messages:

- `STATUS?`
- `SENSORS?`
- `LOGS?`
- `ARM_REQUEST`
- `TEST_REQUEST`
- `RESET_REQUEST`

No Bruce message is a direct hardware-enable command.
