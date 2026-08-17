# MURMUR P01-A — build coverage matrix

| Subsystem | Included | Cirkit representation | Physical validation still required |
|---|---|---|---|
| RP2350/Pico 2 controller | YES | MCU | Exact board pinout/assembly |
| Bruce ESP32-S3 | YES | Separate MCU block | Exact selected module pinout/power |
| I2C sensor bus | YES | Shared labelled bus | Sensor breakout pull-ups/decoupling |
| LIS2DU12/LIS3DH | YES | Sensor block/substitute | Exact part/configuration |
| AS7343 | YES | Sensor block/substitute | Optical mechanical clearance |
| DRV2605L | YES | Driver block | Motor/reference-board supply network |
| microSD | YES | SPI device | Exact module voltage/pull-ups |
| Hardware timer A | YES | Functional block | Exact LTC6993 RC network |
| Hardware timer B | YES | Functional block | Exact LTC6993 RC network |
| Physical interlock | YES | Input/proof block | Physical switch/interlock hardware |
| Independent fault input | YES | Active-low inhibit | Final fault source implementation |
| Firmware permission | YES | RP2350 GPIO/state machine | Bench firmware test |
| Final load switch | YES | TPS22919B/functional switch | Exact part/conditions |
| Load feedback | YES | Feedback net | Actual sensing method |
| Low-voltage test load | YES | Resistor/LED | Bench current/thermal test |
| Production optical/high-power load | BOUNDARY ONLY | Connector/block | Separate electrical/thermal/optical design |
| Regulator values | BOUNDARY | Named power domain | Final regulator selection and derating |
| Connector ratings | BOUNDARY | Named connectors/nets | Exact connector choice |
| PCB geometry | NO | Not simulated | Stack-up, clearance, return paths |
| Enclosure/thermal | NO | Not simulated | Physical validation |
| Wireless protocol | YES | Bruce interface contract | Final firmware/protocol implementation |

## Review conclusion

The Cirkit package covers the complete **architecture** and low-voltage validation path. It does not falsely claim that browser simulation proves production high-power, thermal, PCB or connector behaviour.
