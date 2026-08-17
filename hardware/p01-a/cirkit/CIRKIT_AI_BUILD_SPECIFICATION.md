# CIRKIT AI — MURMUR P01-A COMPLETE SYSTEM BUILD v3

Build the ENTIRE MURMUR P01-A as ONE coherent connected Cirkit project. Do not create disconnected examples. Preserve the named nets and RP2350 GPIOs exactly.

This is a low-voltage prototype/validation model. The visible LED and IR sections must use benign low-power test emitters. Do not model or drive a high-power optical emitter from the simulator.

# A. CONTROLLERS

## U1 — Pico 2 / RP2350 non-wireless
Authoritative deterministic controller.

GPIO map:
- GP4 -> I2C_SDA
- GP5 -> I2C_SCL
- GP6 -> LOAD_EN
- GP7 <- INTERLOCK_PROOF
- GP8 <- FAULT_IN (active low)
- GP9 <- TIMER_A_OK
- GP10 <- TIMER_B_OK
- GP11 <- LOAD_FB
- GP16 <- SD_MISO
- GP17 -> SD_CS
- GP18 -> SD_SCK
- GP19 -> SD_MOSI
- GP20 -> HAPTIC_CTRL
- GP21 <- USER_TEST (active-low button)
- GP25 -> STATUS_LED

U1 owns the state machine, sensing, logging, diagnostics and local permission request. It must remain fully functional when all wireless/ESP32 devices are disconnected.

## U2 — BRUCE / ESP32-S3
Operator/communications node.

Use an ESP32-S3 block where available. Keep its exact physical UART pins abstract until the actual purchased board is fixed.

Logical nets:
- BRUCE_TX -> U1 telemetry RX
- BRUCE_RX <- U1 telemetry TX
- BRUCE_3V3 -> Bruce supply
- BRUCE_GND -> interface reference

Bruce is NON-AUTHORITATIVE. It must not connect directly to LOAD_EN, INTERLOCK_PROOF, TIMER_A_OK, TIMER_B_OK, FAULT_IN or the final load-switch control.

Bruce reset/disconnect/brownout must leave U1 safe and operational locally.

## U3 — optional isolated ESP32 auxiliary/test node
If the Cirkit library supports a second ESP32, add an ESP32-C3/S3 auxiliary node labelled `ESP_AUX` for non-authoritative sensor/bench telemetry. Keep it outside the permission path exactly like Bruce. If unavailable, leave a labelled connector block `ESP_AUX_HEADER` rather than inventing unsupported wiring.

# B. SENSORS / I2C

Create one shared I2C bus:
- GP4 = I2C_SDA
- GP5 = I2C_SCL

Devices:
- U4 LIS2DU12 preferred, or LIS3DH fallback: 0x18/0x19
- U5 AS7343: 0x39
- U6 DRV2605L: 0x5A

Use one logical 4.7k pull-up pair to 3V3 unless validated breakout boards already contain pull-ups. Do not accidentally stack duplicate pull-ups.

Give every I2C device local decoupling.

# C. STORAGE

U7 = 3.3-V-compatible microSD.

- GP16 <- SD_MISO
- GP17 -> SD_CS
- GP18 -> SD_SCK
- GP19 -> SD_MOSI

Storage is non-authoritative. SD failure cannot prevent shutdown.

# D. HAPTICS

U6 DRV2605L controls a vibration actuator.

The RP2350 must not drive the motor directly from GPIO. Represent the driver and actuator explicitly.

# E. VISIBLE LIGHT ENGINE

Create a separate low-voltage visible-light section labelled `VISIBLE_LIGHT_ENGINE`.

Use benign 3.3-V/5-V-rated LEDs or a small addressable RGB LED for simulation.

Recommended logical nets:
- VIS_EN
- VIS_PWM_R
- VIS_PWM_G
- VIS_PWM_B
- VIS_GND

If exact RGB PWM GPIOs are unavailable on the current reserved map, use an I2C LED-driver functional block or a labelled auxiliary LED-driver block rather than stealing safety GPIOs.

Visible light must have current limiting, a transistor/driver stage where required, local decoupling, and explicit supply and ground. Do not connect LED current directly to an RP2350 pin where the load exceeds GPIO limits.

# F. IR TEST EMITTER ENGINE

Create `IR_TEST_ENGINE` as a separate low-power bench subsystem.

Use one or more low-power 850 nm or 940 nm IR LEDs suitable for indicator/bench testing, with current limiting and a transistor/MOSFET driver if needed.

Logical nets:
- IR_EN
- IR_PWM
- IR_SENSE_OPTIONAL
- IR_3V3/IR_5V depending on the selected benign LED
- IR_GND

The simulator must use a low-power LED representation. Do not model a high-power IR COB, high-current optical array, or unattended emitter driver as a directly validated Cirkit subsystem.

The IR test engine is a load boundary and must have a defined default-OFF state.

# G. OPTICAL LOAD BOUNDARY

Create a clearly labelled block:
`PRODUCTION_OPTICAL_STAGE — PHYSICAL ENGINEERING GATE`

Connect the prototype control boundary to a benign low-voltage test load only.

Do NOT claim Cirkit simulation validates the production optical/high-power stage, thermal design, emitter current, enclosure heat dissipation, PCB clearances or connector ratings.

# H. HARDWARE INTERLOCK / TIMERS

U8 = LTC6993 TIMER_A functional block.
U9 = LTC6993 TIMER_B functional block.

Outputs:
- U8 -> TIMER_A_OK -> U1 GP9
- U9 -> TIMER_B_OK -> U1 GP10

Keep them independent. Do not replace either with a software timer.

Create an explicit physical interlock input:
- INTERLOCK_PROOF -> U1 GP7

Create an independent active-low fault input:
- FAULT_IN -> U1 GP8

Missing any proof must remove permission.

# I. FINAL LOW-VOLTAGE LOAD SWITCH

U10 = TPS22919B or validated equivalent functional load-switch block.

Control boundary:
- LOAD_EN from U1 GP6
- hardware proof/interlock path must also be represented
- switched output = TEST_LOAD_VSW

U10 drives only a benign resistor/LED test load in simulation.

Never power the load from an RP2350 GPIO.

Create independent load feedback:
- TEST_LOAD_VSW -> LOAD_FB conditioning/indicator -> U1 GP11

LOAD_FB must not simply be a wire copy of LOAD_EN.

# J. POWER ARCHITECTURE

Show separate named low-voltage domains:

`LOGIC_3V3`
- Pico
- sensors
- SD logic
- DRV2605L logic

`BRUCE_3V3`
- Bruce ESP32-S3

`OPTICAL_TEST_VSW`
- benign visible/IR test loads

Use common low-voltage ground where required by the actual interface, but keep switched-load return currents away from the sensor/I2C routing in the schematic.

Include power-entry protection/decoupling as labelled functional blocks where exact regulator hardware is not yet fixed. Do not invent regulator part numbers or RC values without a selected physical BOM.

# K. TEST / INSTRUMENTATION

Add labelled test points:
- TP_3V3
- TP_GND
- TP_SDA
- TP_SCL
- TP_LOAD_EN
- TP_INTERLOCK
- TP_FAULT
- TP_TIMER_A
- TP_TIMER_B
- TP_LOAD_FB
- TP_BRUCE_TX
- TP_BRUCE_RX
- TP_BRUCE_PWR
- TP_VISIBLE
- TP_IR
- TP_TEST_LOAD

# L. FIRMWARE / BEHAVIOUR

Load the supplied `murmur_p01a_controller.ino` onto U1.

Load `bruce_murmur_node.ino` onto U2 where the selected Cirkit ESP32-S3 target supports it.

U1 state machine:
BOOT -> SELF_TEST -> SAFE_IDLE -> ARMED -> ACTIVE -> COOLDOWN
and any failed proof -> FAULT/SAFE state.

Test:
1. Power-on default outputs OFF.
2. Sensor/I2C discovery.
3. SD unavailable.
4. Interlock open.
5. Timer A proof missing.
6. Timer B proof missing.
7. FAULT_IN asserted.
8. Load feedback mismatch.
9. User test.
10. Active timeout.
11. Reset during ACTIVE.
12. Bruce reset.
13. Bruce disconnected.
14. ESP_AUX disconnected if present.
15. Visible-light test output.
16. Low-power IR test output.

In every fault/proof-loss case, `LOAD_EN` returns LOW and the controlled test-load output becomes OFF.

# M. CIRKIT FINISH CRITERIA

The generated project is only complete when:
- Pico 2/RP2350 is present.
- Bruce ESP32-S3 is present.
- Optional ESP_AUX is either a real supported ESP32 simulation or clearly labelled connector block.
- Sensors, haptic, SD and test inputs are present.
- Visible-light engine is present.
- Low-power IR test engine is present.
- Dual timer proof blocks are present.
- Interlock and independent fault are present.
- Final low-voltage load switch and feedback are present.
- Power domains are labelled.
- No required input is floating.
- No active safety net is left unexplained.
- Bruce/ESP_AUX cannot bypass the RP2350 authority.
- Production optical stage is clearly marked as a physical engineering gate.
- All active RP2350 GPIO assignments match this document exactly.
- The result is ONE coherent project suitable for simulation and later physical wiring review.
