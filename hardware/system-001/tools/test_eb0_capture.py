import contextlib
import io
import json
import sys
import tempfile
import unittest
from pathlib import Path
from types import SimpleNamespace
from unittest.mock import patch

from eb0_capture import (BOOT_MARKER, EXPECTED_FW, INPUT_NAMES, PROOF_MARKER,
                         main, parse_log_lines)


def boot(fw=EXPECTED_FW, watchdog=0, cause=987):
    # Deliberately arbitrary raw cause: the host must not interpret port enums.
    return [BOOT_MARKER, f"fw_id={fw}", f"reset_cause={cause}",
            f"reset_is_watchdog={watchdog}", "BOOT_IDENTITY_COMPLETE"]


def events(cycles=1):
    result = []
    seq = 0
    for name in INPUT_NAMES:
        for count in range(1, cycles * 2 + 1):
            seq += 1
            value = count % 2
            meaning = (("NORMAL_CLOSED", "TAMPER_OPEN") if name == "TAMPER"
                       else ("PRESSED", "RELEASED"))[value]
            result.append(f"EVENT uptime_ms={seq} seq={seq} name={name} "
                          f"value={value} meaning={meaning} count={count}")
    return result


def soak(uptime=3600000, seq=10):
    return (f"SOAK_COMPLETE uptime_ms={uptime} event_seq={seq} "
            "MODE=1 UP=1 DOWN=1 ACTION=1 TAMPER=0")


class ParseEvidenceTests(unittest.TestCase):
    def test_clean_soak(self):
        result = parse_log_lines(boot() + events() + [soak()], expected_cycles=1)
        self.assertTrue(result["soak_complete_seen"])
        self.assertTrue(result["controls_match_expected"])
        self.assertTrue(result["firmware_id_match"])
        self.assertFalse(result["parse_errors"])

    def test_watchdog_proof(self):
        result = parse_log_lines(boot() + [PROOF_MARKER] + boot(watchdog=1))
        self.assertTrue(result["wdt_reset_observed"])
        self.assertEqual(result["boot_count"], 2)

    def test_truncated_watchdog_boot(self):
        for length in range(1, 5):
            with self.subTest(length=length):
                self.assertFalse(parse_log_lines(
                    boot() + [PROOF_MARKER] + boot(watchdog=1)[:length]
                )["wdt_reset_observed"])

    def test_watchdog_identity_and_cause_required(self):
        good = boot(watchdog=1)
        bad_headers = [boot(fw="wrong", watchdog=1), boot(watchdog=0),
                       boot(watchdog="True"), boot(cause="unknown", watchdog=1),
                       good[:1] + good[2:], good[:2] + good[3:],
                       good[:3] + good[4:], good[:2] + ["fw_id=wrong"] + good[2:],
                       good[:4] + ["reset_is_watchdog=0"] + good[4:],
                       good[:2] + ["reset_cause=123"] + good[2:]]
        for header in bad_headers:
            with self.subTest(header=header):
                self.assertFalse(parse_log_lines(
                    boot() + [PROOF_MARKER] + header)["wdt_reset_observed"])

    def test_proof_must_be_from_matching_first_boot(self):
        for prefix in ([PROOF_MARKER], boot(fw="wrong") + [PROOF_MARKER],
                       [BOOT_MARKER, PROOF_MARKER], boot()):
            with self.subTest(prefix=prefix):
                self.assertFalse(parse_log_lines(prefix + boot(watchdog=1))[
                    "wdt_reset_observed"])

    def test_reset_fields_cannot_cross_boots(self):
        self.assertFalse(parse_log_lines(
            boot(watchdog=1) + [PROOF_MARKER, BOOT_MARKER,
                                  f"fw_id={EXPECTED_FW}", "BOOT_IDENTITY_COMPLETE"]
        )["wdt_reset_observed"])
        self.assertFalse(parse_log_lines(
            boot() + [PROOF_MARKER, BOOT_MARKER] + boot(watchdog=1)
        )["wdt_reset_observed"])

    def test_reboot_fails_soak_assessment(self):
        result = parse_log_lines(boot() + boot() + [soak(seq=0)])
        self.assertTrue(result["unexpected_reboot"])
        self.assertFalse(result["soak_complete_seen"])

    def test_duplicate_counter_repro(self):
        duplicated = []
        for line in events()[::2]:
            duplicated.extend([line] * 40)
        result = parse_log_lines(boot() + duplicated, expected_cycles=20)
        self.assertFalse(result["controls_match_expected"])
        self.assertTrue(result["parse_errors"])

    def test_prescribed_twenty_cycles(self):
        result = parse_log_lines(boot() + events(20) + [soak(seq=200)], 20)
        self.assertTrue(result["controls_match_expected"])
        self.assertTrue(result["soak_complete_seen"])

    def test_invalid_event_sequences(self):
        original = events()
        for replacement in (
            original[1].replace("seq=2", "seq=1"),
            original[1].replace("count=2", "count=1"),
            original[1].replace("uptime_ms=2", "uptime_ms=0"),
            original[1].replace("value=0", "value=1"),
            original[1].replace("meaning=PRESSED", "meaning=RELEASED"),
            "EVENT malformed",
        ):
            with self.subTest(replacement=replacement):
                result = parse_log_lines(boot() + [original[0], replacement] +
                                         original[2:] + [soak()], expected_cycles=1)
                self.assertFalse(result["controls_match_expected"])
                self.assertFalse(result["soak_complete_seen"])

    def test_soak_requires_duration_and_complete_record(self):
        for marker in ("SOAK_COMPLETE uptime_ms=1", soak(uptime=1),
                       soak(uptime=3599999), soak(seq=11),
                       soak().replace(" TAMPER=0", ""), soak() + " garbage",
                       soak().replace("MODE=1", "MODE=2")):
            with self.subTest(marker=marker):
                self.assertFalse(parse_log_lines(boot() + events() + [marker])[
                    "soak_complete_seen"])

    def test_soak_requires_matching_identity(self):
        for header in ([BOOT_MARKER], boot(fw="wrong"), []):
            self.assertFalse(parse_log_lines(header + events() + [soak()])[
                "soak_complete_seen"])


class CaptureTests(unittest.TestCase):
    def capture(self, lines, phase, cycles=0):
        class FakeSerial:
            def __init__(self, *args, **kwargs):
                self.lines = iter(lines)
                self.read_count = 0
            def __enter__(self):
                return self
            def __exit__(self, *args):
                return False
            def readline(self):
                try:
                    line = next(self.lines)
                except StopIteration:
                    raise KeyboardInterrupt
                self.read_count += 1
                return (line + "\n").encode()

        serial = FakeSerial()
        with tempfile.TemporaryDirectory() as directory:
            argv = ["eb0_capture.py", "--port", "MOCK", "--phase", phase,
                    "--sample", "HOST-TEST", "--board-rev", "SIMULATED",
                    "--operator", "unittest", "--output-dir", directory,
                    "--expected-cycles", str(cycles)]
            with patch.object(sys, "argv", argv), patch.dict(sys.modules, {
                "serial": SimpleNamespace(Serial=lambda *a, **kw: serial)
            }), contextlib.redirect_stdout(io.StringIO()):
                code = main()
            summary = json.loads(next(Path(directory).glob("*.json")).read_text())
            log = next(Path(directory).glob("*.log")).read_text()
            self.assertTrue(summary["does_not_set_hardware_pass"])
            return code, summary, log, serial.read_count

    def test_serial_waits_for_complete_reset_header(self):
        lines = boot() + [PROOF_MARKER] + boot(watchdog=1)
        code, summary, log, count = self.capture(lines + ["AFTER"], "wdt-proof")
        self.assertEqual(code, 0)
        self.assertEqual(count, len(lines))
        self.assertEqual(summary["stop_reason"], "wdt_reset_observed")
        self.assertIn("reset_is_watchdog=1", log)
        self.assertIn("BOOT_IDENTITY_COMPLETE", log)
        self.assertNotIn("AFTER", log)

    def test_serial_rejects_incomplete_and_non_watchdog_boot(self):
        for header in ([BOOT_MARKER], boot(watchdog=0), boot(fw="wrong", watchdog=1)):
            code, summary, _, _ = self.capture(boot() + [PROOF_MARKER] + header, "wdt-proof")
            self.assertEqual(code, 2)
            self.assertFalse(summary["capture"]["wdt_reset_observed"])

    def test_serial_soak_success_and_invalid_marker(self):
        code, summary, _, _ = self.capture(boot() + events() + [soak()], "soak", 1)
        self.assertEqual(code, 0)
        self.assertEqual(summary["stop_reason"], "soak_complete_seen")
        code, _, _, _ = self.capture(boot() + events() + ["SOAK_COMPLETE uptime_ms=1"], "soak", 1)
        self.assertEqual(code, 3)


if __name__ == "__main__":
    unittest.main()
