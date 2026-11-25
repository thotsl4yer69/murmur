#!/usr/bin/env python3
"""
Capture and summarize MURMUR SYSTEM 001 BLACK E-B0 serial evidence.

This tool assists evidence collection. It never writes PASS to the release gate.
A physical E-B0 result still requires the runbook measurements, evidence files,
operator review, and release-gate record.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import time
from collections import Counter
from datetime import datetime, timezone
from pathlib import Path

EXPECTED_FW = "MUR-S001-BLACK-EB0-R0.2"
BOOT_MARKER = "MURMUR SYSTEM 001 / BLACK E-B0"
INPUT_NAMES = ("MODE", "UP", "DOWN", "ACTION", "TAMPER")

EVENT_RE = re.compile(
    r"^EVENT uptime_ms=(?P<uptime>\d+) seq=(?P<seq>\d+) "
    r"name=(?P<name>[A-Z]+) value=(?P<value>[01]) "
    r"meaning=(?P<meaning>[A-Z_]+) count=(?P<count>\d+)$"
)

def utc_now() -> str:
    return datetime.now(timezone.utc).isoformat(timespec="milliseconds").replace("+00:00", "Z")

def parse_log_lines(lines, expected_cycles=0):
    boots = 0
    proof_armed = False
    wdt_reset_observed = False
    fw_ids = []
    soak_complete_seen = False
    event_counts = Counter()
    last_device_count = {}
    parse_errors = []

    for raw in lines:
        line = raw.strip()
        if not line:
            continue

        if line == BOOT_MARKER:
            boots += 1
            if proof_armed and boots >= 2:
                wdt_reset_observed = True
            continue

        if line.startswith("fw_id="):
            fw_ids.append(line.split("=", 1)[1].strip())
            continue

        if line.startswith("WDT_PROOF_ARMED:"):
            proof_armed = True
            continue

        if line.startswith("SOAK_COMPLETE "):
            soak_complete_seen = True
            continue

        if line.startswith("EVENT "):
            m = EVENT_RE.match(line)
            if not m:
                parse_errors.append(line)
                continue
            name = m.group("name")
            if name not in INPUT_NAMES:
                parse_errors.append(line)
                continue
            event_counts[name] += 1
            last_device_count[name] = int(m.group("count"))

    expected_transitions = expected_cycles * 2 if expected_cycles else None
    controls_match_expected = None
    if expected_transitions is not None:
        controls_match_expected = all(event_counts[name] == expected_transitions for name in INPUT_NAMES)

    return {
        "boot_count": boots,
        "proof_armed_seen": proof_armed,
        "wdt_reset_observed": wdt_reset_observed,
        "firmware_ids": fw_ids,
        "firmware_id_match": bool(fw_ids) and all(x == EXPECTED_FW for x in fw_ids),
        "soak_complete_seen": soak_complete_seen,
        "unexpected_reboot": boots > 1,
        "event_transition_counts": {name: event_counts[name] for name in INPUT_NAMES},
        "last_device_event_count": last_device_count,
        "expected_cycles_per_input": expected_cycles or None,
        "expected_transitions_per_input": expected_transitions,
        "controls_match_expected": controls_match_expected,
        "parse_errors": parse_errors,
    }

def sha256_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1024 * 1024), b""):
            h.update(chunk)
    return h.hexdigest()

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--port", required=True, help="Serial port, e.g. COM5 or /dev/ttyACM0")
    ap.add_argument("--baud", type=int, default=115200)
    ap.add_argument("--phase", choices=("soak", "wdt-proof"), required=True)
    ap.add_argument("--sample", required=True)
    ap.add_argument("--board-rev", required=True)
    ap.add_argument("--operator", required=True)
    ap.add_argument("--expected-cycles", type=int, default=0,
                    help="Expected full cycles per input across this capture; each cycle is 2 transitions.")
    ap.add_argument("--output-dir", default="evidence")
    ap.add_argument("--max-minutes", type=float, default=70.0)
    args = ap.parse_args()

    try:
        import serial
    except ImportError:
        raise SystemExit("pyserial is required: python -m pip install pyserial")

    out_dir = Path(args.output_dir)
    out_dir.mkdir(parents=True, exist_ok=True)

    stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ")
    base = f"S001_G3_EB0_{args.sample}_{args.phase}_{stamp}"
    log_path = out_dir / f"{base}.log"
    json_path = out_dir / f"{base}.json"
    sha_path = out_dir / f"{base}.sha256"

    started_host = utc_now()
    captured_device_lines = []
    first_boot_seen = False
    stop_reason = "timeout"
    deadline = time.monotonic() + args.max_minutes * 60.0

    print(f"Capturing {args.phase} from {args.port} -> {log_path}")
    print("Start capture before the required power-cycle. Ctrl+C stops and still writes evidence.")

    try:
        with serial.Serial(args.port, args.baud, timeout=0.25) as ser, log_path.open("w", encoding="utf-8", newline="\n") as log:
            while time.monotonic() < deadline:
                raw = ser.readline()
                if not raw:
                    continue

                text = raw.decode("utf-8", errors="replace").rstrip("\r\n")
                host_ts = utc_now()
                log.write(f"{host_ts}\t{text}\n")
                log.flush()
                captured_device_lines.append(text)
                print(text)

                if text == BOOT_MARKER:
                    if first_boot_seen and args.phase == "soak":
                        stop_reason = "unexpected_reboot"
                        break
                    first_boot_seen = True

                parsed = parse_log_lines(captured_device_lines, args.expected_cycles)
                if args.phase == "wdt-proof" and parsed["wdt_reset_observed"]:
                    stop_reason = "wdt_reset_observed"
                    break
                if args.phase == "soak" and parsed["soak_complete_seen"]:
                    stop_reason = "soak_complete_seen"
                    break
    except KeyboardInterrupt:
        stop_reason = "operator_interrupt"

    ended_host = utc_now()
    parsed = parse_log_lines(captured_device_lines, args.expected_cycles)

    assessment = {
        "classification": "HOST_EVIDENCE_ASSIST_ONLY",
        "does_not_set_hardware_pass": True,
        "phase": args.phase,
        "sample": args.sample,
        "board_revision": args.board_rev,
        "operator": args.operator,
        "port": args.port,
        "baud": args.baud,
        "started_utc": started_host,
        "ended_utc": ended_host,
        "stop_reason": stop_reason,
        "capture": parsed,
        "hardware_measurements_required_separately": [
            "supply/current measurements required by E-B0 runbook",
            "photos of board identity and wiring",
            "continuity/preflight record",
            "operator PASS/FAIL/REWORK decision",
        ],
    }

    json_path.write_text(json.dumps(assessment, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    hashes = {
        log_path.name: sha256_file(log_path),
        json_path.name: sha256_file(json_path),
    }
    sha_path.write_text(
        "".join(f"{digest}  {name}\n" for name, digest in hashes.items()),
        encoding="utf-8",
    )

    print(f"Wrote {json_path}")
    print(f"Wrote {sha_path}")

    # Exit non-zero only for machine-detectable capture failures.
    if args.phase == "wdt-proof" and not parsed["wdt_reset_observed"]:
        return 2
    if args.phase == "soak":
        if not parsed["soak_complete_seen"] or parsed["unexpected_reboot"] or not parsed["firmware_id_match"]:
            return 3
        if args.expected_cycles and not parsed["controls_match_expected"]:
            return 4
    return 0

if __name__ == "__main__":
    sys.exit(main())
