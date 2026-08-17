# MURMUR P01-A — wiring table v3

## U1 Pico 2 / RP2350
| Pico pin | Net | Connect to |
|---|---|---|
| GP4 | I2C_SDA | U4 SDA, U5 SDA, U6 SDA, 4.7k pull-up |
| GP5 | I2C_SCL | U4 SCL, U5 SCL, U6 SCL, 4.7k pull-up |
| GP6 | LOAD_EN | permission logic input; never directly to load |
| GP7 | INTERLOCK_PROOF | physical interlock proof input |
| GP8 | FAULT_IN | independent active-low fault input, pull-up |
| GP9 | TIMER_A_OK | U8 LTC6993 proof output |
| GP10 | TIMER_B_OK | U9 LTC6993 proof output |
| GP11 | LOAD_FB | U13 feedback conditioner |
| GP16 | SD_MISO | U7 SD DO/MISO |
| GP17 | SD_CS | U7 SD CS |
| GP18 | SD_SCK | U7 SD CLK |
| GP19 | SD_MOSI | U7 SD DI/MOSI |
| GP20 | HAPTIC_CTRL | U6 DRV2605L control/interface |
| GP21 | USER_TEST | momentary button to GND, internal pull-up |
| GP25 | STATUS | LED + 330R |

## U2 Bruce ESP32-S3
| Net | Destination |
|---|---|
| BRUCE_TX | U1 telemetry RX interface |
| BRUCE_RX | U1 telemetry TX interface |
| BRUCE_3V3 | dedicated 3.3V supply |
| BRUCE_GND | interface ground |

Bruce has NO connection to LOAD_EN, interlock proof, timer proofs, fault input or load switch gate.

## U4 LIS2DU12/LIS3DH
SDA -> I2C_SDA; SCL -> I2C_SCL; VCC -> LOGIC_3V3; GND -> GND; address strap per selected device.

## U5 AS7343
SDA -> I2C_SDA; SCL -> I2C_SCL; VCC -> LOGIC_3V3; GND -> GND; address 0x39.

## U6 DRV2605L
SDA -> I2C_SDA; SCL -> I2C_SCL; logic VCC -> LOGIC_3V3; GND -> GND; actuator on driver output. Never drive the motor directly from RP2350.

## U7 microSD
MISO -> GP16; CS -> GP17; SCK -> GP18; MOSI -> GP19; VCC -> LOGIC_3V3; GND -> GND.

## U8/U9 LTC6993
Each is an independent hardware timing/proof channel. Output U8 -> TIMER_A_OK; U9 -> TIMER_B_OK. Exact timing RC values are selected only after the final required interval and exact purchased part are fixed.

## U10 final low-voltage load switch
Control is the ANDed hardware permission boundary. Input -> low-voltage test supply; output -> TEST_LOAD_VSW. Do not use an MCU GPIO as the load supply.

## U11 visible light
Low-voltage RGB LED/driver. Supply -> OPTICAL_TEST_VSW; driver enable controlled only through the validated low-voltage control boundary; current limiting required.

## U12 IR test engine
One or more low-power 850/940nm indicator/test LEDs, series resistor and transistor/MOSFET driver. Supply -> OPTICAL_TEST_VSW. Default OFF. This is a bench test emitter, not a production high-power optical stage.

## U13 load feedback
Sense TEST_LOAD_VSW through an appropriate resistor/indicator/conditioning network. Output -> LOAD_FB. It must represent actual load state rather than simply duplicating LOAD_EN.

## Grounding
All low-voltage logic references use GND. Keep switched-load current return paths physically separate from sensitive I2C/sensor routing on the eventual PCB/layout.

## Test points
Add TP_3V3, TP_GND, TP_SDA, TP_SCL, TP_LOAD_EN, TP_INTERLOCK, TP_FAULT, TP_TIMER_A, TP_TIMER_B, TP_LOAD_FB, TP_BRUCE_TX, TP_BRUCE_RX, TP_BRUCE_PWR, TP_VISIBLE, TP_IR and TP_TEST_LOAD.
