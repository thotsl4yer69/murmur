# MURMUR // PROOF LEDGER

MURMUR SYSTEM 001 uses a public evidence rule:

> A render is not evidence. A product claim earns a **PASS** only after a physical test with its threshold published before the run.

Public ledger: https://murmur.qd.je/pages/proof-ledger

## Result states

- **QUEUED** — protocol fixed; physical test not yet run.
- **RUNNING** — evidence collection in progress.
- **PASS** — published threshold met.
- **FAIL** — threshold not met; failure remains visible.
- **REWORK** — hardware/process changed after a failure; revised implementation must be tested again.

## Test series 0001

| ID | System | Test | Status |
| --- | --- | --- | --- |
| P001 | MOLT CORE | 60-second module swap | QUEUED |
| P002 | COWL + MOLT CORE | Loaded-wear behaviour | QUEUED |
| P003 | MOLT CORE | 100-cycle service test | QUEUED |
| P004 | SYSTEM 001 | Power + thermal endurance | QUEUED — threshold pending frozen bench baseline |
| P005 | COWL | Service separation | QUEUED |

## Public rules

1. The pass threshold is written before the physical run.
2. Evidence and result are published together.
3. Failed runs are not deleted from the ledger.
4. A hardware revision does not inherit an earlier revision's PASS.
5. Design-development imagery is identified separately from measured evidence.

## Community failure hunt

Find a failure mode we missed. A useful proposal defines:

- what could fail;
- how to reproduce the failure;
- what measurement or observation decides PASS vs FAIL.

Accepted proposals can become numbered Proof Ledger tests with the contributor credited by their chosen handle.

Submit via the public ledger or open a GitHub issue using the `proof-test` prefix.

## Current protocol

[P001 — 60-second MOLT CORE module swap](tests/P001-module-swap.md)
