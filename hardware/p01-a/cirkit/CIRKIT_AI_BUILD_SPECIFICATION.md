# CIRKIT AI — MURMUR P01-A COMPLETE BUILD

Create ONE coherent connected schematic/project. Preserve the exact GPIO/net names below.

## Controller
Pico 2/RP2350. If only Pico/RP2040 is simulation-supported, use it as a symbol but do not renumber GPIO.

## Wiring
GP4 SDA -> shared I2C SDA -> LIS2DU12/LIS3DH SDA, AS7343 SDA, DRV2605L SDA.
GP5 SCL -> shared I2C SCL -> LIS2DU12/LIS3DH SCL, AS7343 SCL, DRV2605L SCL.
4.7k pull-up from SDA to 3V3 and SCL to 3V3; avoid duplicate module pull-ups where possible.
GP16 -> SD MISO/DO; GP17 -> SD CS; GP18 -> SD SCK; GP19 -> SD MOSI/DI.
GP20 -> HAPTIC control/DRV2605L representation.
GP21 -> USER_TEST pushbutton to GND; internal pull-up.
GP25 -> STATUS LED + series resistor to GND.
GP6 -> LOAD_EN.
GP7 <- INTERLOCK_PROOF.
GP8 <- FAULT_IN, active LOW and pull-up.
GP9 <- TIMER_A_OK, default LOW/not permitted.
GP10 <- TIMER_B_OK, default LOW/not permitted.
GP11 <- LOAD_FB, default LOW/not permitted.

## Power and grounding
3V3 supplies the controller and logic modules. Common low-voltage GND. 100nF local decoupling per IC plus 10uF bulk. The test-load supply is a separate labelled net and must not be represented as a GPIO-powered load.

## Complete control topology
USER_TEST/firmware request -> LOAD_EN -> INTERLOCK_PROOF -> TIMER_A_OK AND TIMER_B_OK -> controlled test-load switch -> LOW-VOLTAGE TEST LOAD -> LOAD_FB.
FAULT_IN is an independent inhibit. Any inhibit opens the permission path and forces LOAD_EN low.

## Timing
Represent LTC6993 timer A and timer B as explicit labelled functional blocks if exact simulation models are unavailable. Do not replace the two proof signals with one software timer. The firmware must see both TIMER_A_OK and TIMER_B_OK.

## Test instrumentation
Add TP_3V3, TP_GND, TP_SDA, TP_SCL, TP_LOAD_EN, TP_INTERLOCK, TP_FAULT, TP_TIMER_A, TP_TIMER_B, TP_LOAD_FB. Label every connector pin and every active-low signal.

## Simulation load
Use resistor + LED or equivalent low-voltage load only. The real optical/high-power section is a separate physical engineering stage and is not electrically validated by this simulation.

## Finish criteria
No unexplained dangling pins. No floating inputs. Load control defaults OFF. All grounds connected. All nets labelled. Schematic readable. Simulation wiring must match firmware GPIO definitions exactly.
