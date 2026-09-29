import unittest

from eb0_capture import EXPECTED_FW, parse_log_lines

class ParseEvidenceTests(unittest.TestCase):
    def test_clean_soak(self):
        lines = [
            "MURMUR SYSTEM 001 / BLACK E-B0",
            f"fw_id={EXPECTED_FW}",
        ]
        for name in ("MODE", "UP", "DOWN", "ACTION", "TAMPER"):
            meaning_low = "NORMAL_CLOSED" if name == "TAMPER" else "PRESSED"
            meaning_high = "TAMPER_OPEN" if name == "TAMPER" else "RELEASED"
            lines.append(f"EVENT uptime_ms=1 seq=1 name={name} value=0 meaning={meaning_low} count=1")
            lines.append(f"EVENT uptime_ms=2 seq=2 name={name} value=1 meaning={meaning_high} count=2")
        lines.append("SOAK_COMPLETE uptime_ms=3600000 event_seq=10 MODE=1 UP=1 DOWN=1 ACTION=1 TAMPER=0")
        result = parse_log_lines(lines, expected_cycles=1)
        self.assertTrue(result["soak_complete_seen"])
        self.assertFalse(result["unexpected_reboot"])
        self.assertTrue(result["firmware_id_match"])
        self.assertTrue(result["controls_match_expected"])
        self.assertFalse(result["parse_errors"])

    def test_watchdog_proof(self):
        result = parse_log_lines([
            "MURMUR SYSTEM 001 / BLACK E-B0",
            f"fw_id={EXPECTED_FW}",
            "WDT_PROOF_ARMED: no feed; expect watchdog reset.",
            "MURMUR SYSTEM 001 / BLACK E-B0",
            f"fw_id={EXPECTED_FW}",
        ])
        self.assertTrue(result["wdt_reset_observed"])
        self.assertEqual(result["boot_count"], 2)

    def test_reboot_fails_soak_assessment(self):
        result = parse_log_lines([
            "MURMUR SYSTEM 001 / BLACK E-B0",
            f"fw_id={EXPECTED_FW}",
            "MURMUR SYSTEM 001 / BLACK E-B0",
            f"fw_id={EXPECTED_FW}",
        ])
        self.assertTrue(result["unexpected_reboot"])

if __name__ == "__main__":
    unittest.main()
