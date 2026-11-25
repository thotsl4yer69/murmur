# SYSTEM 001 / G3 / E-B0 — Physical Evidence Record

**This record is blank by design. Do not mark PASS without the attached physical evidence.**

## Configuration

- Sample ID:
- Test date/time (UTC):
- Operator:
- Pico 2 board identity/revision:
- Board photo filename:
- MicroPython build/version:
- Firmware ID:
- Firmware Git commit:
- Current-measurement instrument:
- Instrument asset/serial:
- USB power source:
- Wiring/preflight photo filename:

## Preflight

- [ ] Physical pin 18 verified as BLACK GND return
- [ ] MODE GP14 / pin 19 -> momentary contact -> BLACK GND
- [ ] UP GP15 / pin 20 -> momentary contact -> BLACK GND
- [ ] DOWN GP16 / pin 21 -> momentary contact -> BLACK GND
- [ ] ACTION GP22 / pin 29 -> momentary contact -> BLACK GND
- [ ] TAMPER GP9 / pin 12 -> NC contact -> BLACK GND
- [ ] GP26 service jumper absent for normal soak
- [ ] No RED board / sensor / haptic / switched rail / external supply / garment harness attached

Preflight result: PASS / FAIL / REWORK

## WDT proof

- Capture log:
- Capture JSON:
- SHA-256 manifest:
- `WDT_PROOF_ARMED` observed: YES / NO
- Watchdog-caused reboot observed: YES / NO
- Manual reset used as substitute: MUST BE NO

WDT proof result: PASS / FAIL / REWORK

## 60-minute soak

- Capture log:
- Capture JSON:
- SHA-256 manifest:
- Start UTC:
- End UTC:
- `SOAK_COMPLETE` observed: YES / NO
- Unexpected reboot during soak: YES / NO
- Firmware ID matched R0.2: YES / NO

### Control transitions

Required for this run: 20 full cycles / 40 stable transitions per input.

| Input | Recorded transitions | Extra/missed/uncommanded transitions | Result |
|---|---:|---|---|
| MODE | | | |
| UP | | | |
| DOWN | | | |
| ACTION | | | |
| TAMPER | | | |

Tamper semantics verified: LOW=`NORMAL_CLOSED`, HIGH=`TAMPER_OPEN`: YES / NO

### Current evidence

No new E-B0 current ceiling is introduced. Values are evidence inputs for later power budgeting.

| Measurement | Instrument file/time | Voltage | Mean current | Max current |
|---|---|---:|---:|---:|
| Idle after first control sequence, 60 s | | | | |
| Active control sequence | | | | |
| Idle during final 5 min, 60 s | | | | |

Current trace/export filename:

## Final physical decision

- Result: PASS / FAIL / REWORK
- Failure/rework notes:
- Evidence reviewer:
- Review date:
- Decision + Release Gate Log entry/reference:

**Scope reminder:** an E-B0 PASS does not pass the rest of G3 and does not establish RF/EMC/EME, garment, carrier, battery, thermal or commercial conformity.
