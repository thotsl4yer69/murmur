# MURMUR P01-A — Physical/reference netlist v2

This netlist is authoritative when a Cirkit library component is unavailable. A substitute must preserve pin labels, polarity and logical behaviour.

## U1 — RP2350 / Pico 2 non-wireless
3V3, GND.
GP4=I2C_SDA; GP5=I2C_SCL.
GP6=LOAD_EN; GP7=INTERLOCK_PROOF; GP8=FAULT_IN; GP9=TIMER_A_OK; GP10=TIMER_B_OK; GP11=LOAD_FB.
GP16=SD_MISO; GP17=SD_CS; GP18=SD_SCK; GP19=SD_MOSI.
GP20=HAPTIC; GP21=USER_TEST; GP25=STATUS.

All load-control outputs default low at reset and before peripheral initialisation.

## U2 — Bruce / ESP32-S3 communications node
Represent as a separate MCU block. It is not a safety controller.

Logical connections only until the exact ESP32-S3 board/module is selected:
- BRUCE_TX -> U1 telemetry RX
- BRUCE_RX <- U1 telemetry TX
- BRUCE_PWR -> BRUCE_3V3
- BRUCE_GND -> logic ground only where the selected interface requires a common reference

Bruce must not connect directly to LOAD_EN, INTERLOCK_PROOF, TIMER_A_OK, TIMER_B_OK or the final load switch control.

## U3 — LIS2DU12 preferred / LIS3DH fallback
VDD=LOGIC_3V3, GND=GND, SDA=I2C_SDA, SCL=I2C_SCL. Address 0x18/0x19 as configured. Local 100nF bypass.

## U4 — AS7343
VDD=LOGIC_3V3, GND=GND, SDA=I2C_SDA, SCL=I2C_SCL, address 0x39. Local 100nF bypass. Optical window must remain mechanically unobstructed.

## U5 — DRV2605L
Logic/motor supply must follow the selected validated reference module. SDA=I2C_SDA, SCL=I2C_SCL, address 0x5A. Haptic actuator connects only to driver output.

## U6 — microSD
3.3-V-compatible VCC/GND. DO=MISO_GP16, CS=GP17, SCK=GP18, DI=MOSI_GP19. Local bypass. Storage is non-authoritative.

## U7/U8 — LTC6993 timer/proof channels A/B
Independent hardware timing channels. Each has its own timing network and proof output.
U7 output=TIMER_A_OK.
U8 output=TIMER_B_OK.
Exact RC values remain an engineering parameter until the final chosen LTC6993 variant, target interval and measured tolerances are fixed.

## U9 — TPS22919B / validated equivalent
Final low-voltage test-load switch. It receives only the hardware permission result. It is not a GPIO-powered load.

## Interlock chain
1. Physical enable/interlock.
2. Timer A proof.
3. Timer B proof.
4. RP2350 firmware permission request.
5. Final switched test-load stage.
6. Independent load feedback.

A fault or missing proof removes permission.

## Power domains
- LOGIC_3V3: RP2350 + sensors + SD + driver logic
- BRUCE_3V3: ESP32-S3/Bruce communications domain
- TEST_LOAD_VSW: switched low-voltage test-load domain

Do not route switched-load current through sensitive sensor traces. Use explicit rail labels in Cirkit.

## Test points
TP_3V3, TP_GND, TP_SDA, TP_SCL, TP_LOAD_EN, TP_INTERLOCK, TP_TIMER_A, TP_TIMER_B, TP_FAULT, TP_LOAD_FB, TP_BRUCE_TX, TP_BRUCE_RX, TP_BRUCE_PWR, TP_TEST_LOAD.
