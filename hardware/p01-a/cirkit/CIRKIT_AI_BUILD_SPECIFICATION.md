# CIRKIT AI — MURMUR P01-A COMPLETE BUILD v2

Create ONE coherent connected project. Preserve all exact RP2350 GPIO/net names below. Add Bruce as a separate ESP32-S3 communications node and do not connect it to the hardware permission authority.

## Controller U1
Pico 2 / RP2350 non-wireless.

GPIO/net mapping:
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
- GP20 -> HAPTIC control representation
- GP21 <- USER_TEST pushbutton to GND, internal pull-up
- GP25 -> STATUS LED through series resistor

## Communications U2 — Bruce
Use an ESP32-S3 MCU block where available.
Logical nets only until the exact module is selected:
- BRUCE_TX -> U1 telemetry RX
- BRUCE_RX <- U1 telemetry TX
- BRUCE_3V3 -> Bruce power
- BRUCE_GND -> selected interface reference

Bruce is telemetry/operator UI only. It must NOT connect directly to LOAD_EN, INTERLOCK_PROOF, TIMER_A_OK, TIMER_B_OK, FAULT_IN or the final load switch control. Bruce disconnect/reset must leave U1 safe and operational locally.

## I2C bus
GP4/GP5 shared to:
- LIS2DU12 preferred / LIS3DH fallback (0x18/0x19)
- AS7343 (0x39)
- DRV2605L (0x5A)

Use one logical 4.7k pull-up pair to 3V3 unless a selected validated module already supplies the necessary pull-ups. Avoid parallel duplicate pull-ups.

## microSD
GP16 MISO, GP17 CS, GP18 SCK, GP19 MOSI. 3.3-V-compatible interface only.

## Hardware timing U7/U8
Represent two independent LTC6993 timer/proof stages as separate functional blocks if exact simulator components are unavailable.
- U7 -> TIMER_A_OK
- U8 -> TIMER_B_OK

Do not replace these with one software timer.

## Interlock/fault
Represent the physical interlock as an explicit input. Represent FAULT_IN as active-low independent inhibit. Missing interlock or either timer proof must remove load permission.

## Final low-voltage load stage U9
Represent TPS22919B or validated equivalent as a controlled switch. The test load is a resistor + LED or equivalent low-voltage load. Never power the test load from a Pico GPIO.

## Feedback
LOAD_FB is independent of LOAD_EN. Show the feedback net from the switched load boundary back to GP11.

## Power domains
- LOGIC_3V3
- BRUCE_3V3
- TEST_LOAD_VSW

Show common low-voltage references where required by the actual interface. Keep switched load return current out of sensor routing.

## Test points
Add:
TP_3V3, TP_GND, TP_SDA, TP_SCL, TP_LOAD_EN, TP_INTERLOCK, TP_FAULT, TP_TIMER_A, TP_TIMER_B, TP_LOAD_FB, TP_BRUCE_TX, TP_BRUCE_RX, TP_BRUCE_PWR, TP_TEST_LOAD.

## Simulation behaviour
Run the RP2350 firmware through BOOT -> SELF_TEST -> SAFE_IDLE -> ARMED -> ACTIVE -> COOLDOWN/FAULT.

Exercise:
- interlock open
- timer A proof low
- timer B proof low
- fault asserted
- load feedback mismatch
- reset
- user test
- Bruce disconnected/reset

In every failure case LOAD_EN must return low.

## Finish criteria
- One connected schematic/project
- No unexplained dangling active pins
- No floating required inputs
- All critical nets labelled
- All grounds connected as appropriate to the selected interface
- Bruce isolated from the safety authority
- Exact RP2350 GPIOs preserved
- Simulation uses only a benign low-voltage test load
- Production optical/high-power stage shown only as a labelled boundary
