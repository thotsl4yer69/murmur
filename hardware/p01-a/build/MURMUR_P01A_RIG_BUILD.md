# MURMUR P01-A COMPLETE RIG

Complete low-voltage bench prototype specification. Visible LEDs and a low-power 850/940nm IR LED are test loads; no production high-power optical stage is specified or validated here.

## U1 — Pico 2 / RP2350
GP4 I2C_SDA; GP5 I2C_SCL; GP6 LOAD_EN; GP7 INTERLOCK_PROOF; GP8 FAULT_IN active-low; GP9 TIMER_A_OK; GP10 TIMER_B_OK; GP11 LOAD_FB; GP16 SD_MISO; GP17 SD_CS; GP18 SD_SCK; GP19 SD_MOSI; GP20 HAPTIC_CTRL; GP21 USER_TEST active-low; GP25 STATUS_LED.

## U2 — Bruce / ESP32-S3
BRUCE_TX -> U1 telemetry RX; BRUCE_RX <- U1 telemetry TX; BRUCE_3V3; GND. Bruce is non-authoritative and must never connect directly to LOAD_EN, timer proofs, interlock, fault input or final load switching.

## U3 — ESP auxiliary
ESP32-C3/S3 optional non-authoritative telemetry node. It cannot create load permission.

## I2C devices
U4 LIS2DU12 or LIS3DH; U5 AS7343 at 0x39; U6 DRV2605L at 0x5A; U11 PCA9685 at 0x40. One verified 4.7k pull-up pair on SDA/SCL to 3V3 unless the selected modules already provide suitable pull-ups.

## Storage
U7 3.3V-compatible microSD: GP16 MISO, GP17 CS, GP18 SCK, GP19 MOSI. SD failure must not prevent shutdown.

## Lighting
U11 PCA9685 provides PWM expansion. Visible LED/RGB assembly uses appropriate current limiting or its specified constant-current driver. Never drive a larger LED assembly directly from a Pico/PCA9685 output.

## IR test
IR1 is a low-power 850nm or 940nm bench indicator. Drive it with a small-signal transistor/MOSFET or validated constant-current driver and appropriate current limiting. The MCU never powers the emitter directly.

## Interlock/timing
U8 and U9 are independent LTC6993 timer/proof blocks. Physical interlock produces INTERLOCK_PROOF. FAULT_IN is independent active-low inhibit. Conceptual permission: firmware_request AND INTERLOCK_PROOF AND TIMER_A_OK AND TIMER_B_OK AND NOT FAULT_IN. All paths default OFF on reset/brownout.

## Load stage
U10 TPS22919B or validated low-voltage equivalent controls TEST_LOAD_VSW. Use a resistor+LED benign test load. LOAD_FB is independent of LOAD_EN.

## Rails
LOGIC_3V3 for controller/sensors; BRUCE_3V3 for U2/U3; TEST_LOAD_VSW for the switched test load; common low-voltage GND. Never power motor/LED/IR load from an RP2350 GPIO.

## Build order
1 controller; 2 I2C; 3 sensors/haptic/PWM; 4 SD; 5 timer proof inputs; 6 interlock/fault; 7 load switch and benign load; 8 visible lighting; 9 low-power IR; 10 Bruce/AUX. Test each stage before proceeding.

## Acceptance
Reset/brownout OFF; interlock open OFF; either timer proof missing OFF; fault asserted OFF; feedback mismatch FAULT; timeout OFF; SD failure cannot block shutdown; Bruce/AUX disconnect cannot create permission; visible and IR tests remain bounded low-voltage outputs.

## Physical engineering gate
Do not infer production emitter current, thermal design, connector ratings, supply sizing or PCB clearances from this prototype. Exact selected emitter/module and electrical/thermal specifications are required for that stage.
