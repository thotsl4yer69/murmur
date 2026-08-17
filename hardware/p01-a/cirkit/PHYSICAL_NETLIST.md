# MURMUR P01-A — physical/reference netlist

This is the reference architecture to keep the Cirkit schematic honest when a library component is unavailable. A simulation substitute must preserve pin names, polarity and logic behaviour.

## U1 — RP2350 / Pico 2 non-wireless
3V3, GND. I2C GP4/GP5. SPI GP16/17/18/19. HAPTIC GP20. USER_TEST GP21. STATUS GP25. LOAD_EN GP6. INTERLOCK_PROOF GP7. FAULT_IN GP8. TIMER_A_OK GP9. TIMER_B_OK GP10. LOAD_FB GP11.

## U2 — LIS2DU12 (preferred) / LIS3DH fallback
VDD=3V3, GND=GND, SDA=I2C_SDA, SCL=I2C_SCL. Address configured to 0x18 where applicable. Add local 100nF bypass. If LIS3DH is used, preserve its address/configuration and interrupt pins only if required by the firmware revision.

## U3 — AS7343
VDD=3V3, GND=GND, SDA=I2C_SDA, SCL=I2C_SCL, address 0x39. Add local 100nF bypass. Optical window must remain mechanically clear in the physical enclosure.

## U4 — DRV2605L
Logic supply and motor supply per the selected validated breakout/reference design. SDA=I2C_SDA, SCL=I2C_SCL, address 0x5A. Haptic actuator connects only to the driver output, never directly to a Pico GPIO. Place local bypass capacitors as required by the selected module/reference design.

## U5 — microSD
VCC=3V3 for a 3.3-V-compatible module/device. GND=GND. DO=MISO_GP16, CS=GP17, SCK=GP18, DI=MOSI_GP19. Do not use a 5-V-only SD interface. Add local bypass.

## U6/U7 — LTC6993 timer stages A/B
Two independent timeout/proof channels. Each stage has its own configured timing network and produces a permission/proof signal. The MCU does not replace these hardware timers. Cirkit may use labelled functional timer blocks if an exact LTC6993 simulation model is unavailable. Outputs map to TIMER_A_OK and TIMER_B_OK.

## U8 — load switch / permission stage
TPS22919B or validated equivalent for the low-voltage TEST_LOAD rail. EN is not driven by software alone: it is the output of the hardware permission chain. The simulation may represent this as a logic-controlled switch if the exact device is unavailable.

## Interlock chain
1. Physical enable/interlock input.
2. Independent timer A permission.
3. Independent timer B permission.
4. RP2350 firmware permission.
5. Final controlled-load switch.
6. Load feedback.

Any missing proof removes permission. The fault input is independent and active-low.

## Communications boundary
If an ESP32-S3 wireless/communications module is fitted in the production variant, it is outside the safety/control authority of U1. Its interface must not be required for the hardware permission chain. Represent the boundary as a separate block in Cirkit. The non-wireless RP2350 remains the deterministic controller.

## Test load
For Cirkit and initial bench work, use a low-voltage resistor/LED or other non-hazardous test load after U8. The production optical subsystem is represented only as a connector/block boundary here; its emitter-current, thermal and optical engineering must be validated independently.

## Grounding / decoupling
All low-voltage logic grounds common. Keep switched-load return current out of sensitive sensor routing. Use local 100nF bypass at ICs and suitable bulk capacitance at rail entry/load transition points. Exact bulk values are selected against the final regulator, wiring length and load transient measurements rather than assumed from simulation.

## Required labelled test points
TP_3V3, TP_GND, TP_SDA, TP_SCL, TP_LOAD_EN, TP_INTERLOCK, TP_TIMER_A, TP_TIMER_B, TP_FAULT, TP_LOAD_FB.
