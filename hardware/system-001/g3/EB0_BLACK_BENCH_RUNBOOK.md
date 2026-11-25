# SYSTEM 001 / G3 / E-B0 BLACK CORE — Physical Bench Runbook R0.2

**Status:** executable bench procedure; **not a hardware PASS record**.  
**Configuration authority:** SYSTEM 001 DG03, 19 Sep 2026, plus the controlled G3 baseline.  
**Target:** Raspberry Pi Pico 2, non-wireless, RP2350.  
**Firmware:** `hardware/system-001/firmware/black_e_b0_bench.py` with `FW_ID=MUR-S001-BLACK-EB0-R0.2`.

## 1. What E-B0 proves

E-B0 is the first current physical G3 acceptance dependency. It proves only the BLACK core, passive local controls and active watchdog behaviour on the identified Pico 2. It does **not** prove switched rails, sensors, haptics, storage, optical isolation, RED-domain aggregation, RF performance, garment integration, RCM conformity or commercial readiness.

The retained DG03 source criterion is:

- 60-minute powered core run;
- deterministic controls;
- active watchdog;
- no unexplained reset;
- idle and active current recorded.

This R0.2 runbook operationalises "deterministic controls" without changing a public product claim: run the existing DG03 ten-cycle input check both before and after the soak. Each input must therefore produce exactly 20 full cycles / 40 stable transitions across the E-B0 capture, with no extra, missed or uncommanded transitions.

## 2. Controlled pin map

Raspberry Pi's current Pico documentation states that non-wireless Pico and Pico 2 boards share the same 40-pin layout. For this test use only:

| Function | GPIO | Physical pin | Connection |
|---|---:|---:|---|
| MODE | GP14 | 19 | momentary contact to BLACK GND |
| UP | GP15 | 20 | momentary contact to BLACK GND |
| DOWN | GP16 | 21 | momentary contact to BLACK GND |
| ACTION | GP22 | 29 | momentary contact to BLACK GND |
| TAMPER | GP9 | 12 | normally-closed contact to BLACK GND |
| SERVICE/WDT proof | GP26 | 31 | temporary jumper to BLACK GND only during WDT proof |
| BLACK GND | — | 18 | one verified common return option |

Do not substitute Pico physical pin numbers for GPIO numbers.

### Power for E-B0

Use the Pico 2 USB connector from a known-good 5 V USB source through a current-measurement device. Do not inject power into pin 36. Raspberry Pi defines pin 36 as `3V3(OUT)`, pin 37 as `3V3_EN`, pin 39 as `VSYS` and pin 40 as `VBUS`. The E-B0 default fixture intentionally avoids external VSYS wiring.

## 3. Minimum bench equipment

- identified Raspberry Pi Pico 2 non-wireless board;
- known-good USB data cable and 5 V USB source;
- host computer with MicroPython/serial access;
- current meter or USB power logger capable of recording at least 1 mA resolution;
- DMM for unpowered continuity checks;
- four momentary normally-open contacts;
- one normally-closed tamper contact;
- one removable GP26-to-GND service jumper;
- camera/phone for board identity and wiring photographs.

Record make/model/serial or asset identifier for the power/current instrument in the test record. E-B0 records current; it does not introduce a new numeric current ceiling.

## 4. Evidence filenames

Use one sample identifier throughout.

- `S001_G3_EB0_<SAMPLE>_preflight_<UTC>.jpg`
- `S001_G3_EB0_<SAMPLE>_wdt-proof_<UTC>.log`
- `S001_G3_EB0_<SAMPLE>_wdt-proof_<UTC>.json`
- `S001_G3_EB0_<SAMPLE>_soak_<UTC>.log`
- `S001_G3_EB0_<SAMPLE>_soak_<UTC>.json`
- `S001_G3_EB0_<SAMPLE>_current_<UTC>.csv` or instrument export
- matching `.sha256` manifests from the capture tool.

Do not rename evidence from a different hardware revision into this record.

## 5. Preflight — power disconnected

1. Photograph the board top and bottom so the exact Pico 2 identity is visible.
2. Record sample ID, board revision, MicroPython build/version and firmware ID.
3. With USB disconnected, continuity-check physical pin 18 to the chosen BLACK ground rail.
4. Confirm MODE/UP/DOWN/ACTION are open at rest and close only to BLACK GND.
5. Confirm TAMPER is closed to BLACK GND in its normal state and opens on tamper.
6. Confirm GP26 is not grounded for the normal soak.
7. Confirm there are no RED boards, sensors, haptics, switched rails, external supplies or wearable harness attached.

Any wiring mismatch is **REWORK**. Do not power the fixture until corrected.

## 6. WDT proof — separate run

The firmware uses MicroPython `machine.WDT(timeout=4000)`. The watchdog cannot be stopped after it is armed.

1. Fit the temporary GP26-to-BLACK-GND jumper.
2. Start host capture:

   `python hardware/system-001/tools/eb0_capture.py --port <PORT> --phase wdt-proof --sample <SAMPLE> --board-rev <REV> --operator <OPERATOR>`

3. Power-cycle the Pico 2 after capture starts.
4. Confirm the log shows `WDT_PROOF_ARMED`.
5. The host capture must observe a subsequent boot marker after the watchdog expires.
6. Disconnect USB, remove the GP26 jumper, photograph the removed jumper state, then power the board normally.

If no watchdog reset is captured, E-B0 remains **FAIL/REWORK**. A manual reset is not a substitute.

## 7. 60-minute soak

Start capture before the required power cycle:

`python hardware/system-001/tools/eb0_capture.py --port <PORT> --phase soak --sample <SAMPLE> --board-rev <REV> --operator <OPERATOR> --expected-cycles 20`

Then:

1. Power-cycle once into the normal E-B0 firmware.
2. Confirm `fw_id=MUR-S001-BLACK-EB0-R0.2`, `service_gp26=1` and one boot marker.
3. During minutes 0-5, perform 10 full cycles each of MODE, UP, DOWN, ACTION and TAMPER.
4. Record a 60-second idle current sample after the first control sequence.
5. Record current while performing one complete active-control sequence; preserve the meter/logger trace.
6. Leave the fixture powered and undisturbed.
7. During minutes 55-60, repeat 10 full cycles each of MODE, UP, DOWN, ACTION and TAMPER.
8. Record a second 60-second idle current sample.
9. Continue until the device emits `SOAK_COMPLETE`.
10. Save the serial log, summary JSON, SHA-256 manifest and current trace/export.

The firmware continues running after `SOAK_COMPLETE`; the host capture ends the evidence session when that marker is seen.

## 8. E-B0 decision

**PASS is permitted only when all of the following are physically evidenced:**

- identified Pico 2 non-wireless board and fixture photographs;
- correct controlled pin map and unpowered continuity preflight;
- separate WDT proof shows a watchdog-caused reboot;
- 60-minute soak reaches `SOAK_COMPLETE`;
- no second boot marker/unexplained reset occurs during the soak;
- each of MODE/UP/DOWN/ACTION/TAMPER records exactly 40 stable transitions for the prescribed 20 cycles;
- tamper semantics are LOW=`NORMAL_CLOSED`, HIGH=`TAMPER_OPEN`;
- idle and active current evidence is present;
- serial log, JSON and hashes are retained with the same sample/revision.

Any missing condition is **FAIL/REWORK**, not PASS.

The host capture's JSON is a parsing aid only. The Configuration & Evidence Manager records the final physical result after checking the complete evidence set.

## 9. Manufacturer / regulatory references checked for this runbook

- Raspberry Pi Pico-series documentation and Pico 2 pinout: non-wireless Pico/Pico 2 share the pinout; pin 36 `3V3(OUT)`, pin 37 `3V3_EN`, pin 39 `VSYS`, pin 40 `VBUS`.
- MicroPython `machine.WDT`: RP2 supports WDT; once started it cannot be stopped or reconfigured.
- Australian RF/compliance controls are not claimed by E-B0. For later radio-enabled release configuration, re-check the then-current Radiocommunications Equipment (General) Rules 2021, Radiocommunications (Low Interference Potential Devices) Class Licence 2025, applicable EMC/EME standards and exact enabled radios/antennas.

## 10. Gate effect

A real E-B0 PASS clears only the earliest BLACK-core dependency. It does **not** clear G3 as a whole. E-B1/E-B2/E-B2A/E-B3/E-B4/E-B5/E-B6 and RED aggregation remain separately gated. G4 powered acceptance remains HOLD until the connected module envelopes, six registered interfaces, exact battery article and applicable electrical evidence are complete.
