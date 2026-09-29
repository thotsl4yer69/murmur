"""
MURMUR SYSTEM 001 — BLACK E-B0 bench firmware R0.2
Target: Raspberry Pi Pico 2 (RP2350, non-wireless), MicroPython.
Scope: G3 E-B0 bench evidence only.

This firmware configures only the DG03 passive BLACK inputs and the GP26
service jumper. It does not drive sensors, radios, haptics, switched rails,
storage, or RED-domain hardware.

DG03 inputs:
  MODE    GP14 / physical pin 19
  UP      GP15 / physical pin 20
  DOWN    GP16 / physical pin 21
  ACTION  GP22 / physical pin 29
  TAMPER  GP9  / physical pin 12, NC to BLACK GND
  SERVICE GP26 / physical pin 31, optional WDT proof jumper to BLACK GND
"""
from machine import Pin, WDT, reset_cause
import os
import sys
import time

FW_ID = "MUR-S001-BLACK-EB0-R0.2"
WDT_TIMEOUT_MS = 4000
DEBOUNCE_MS = 25
HEARTBEAT_MS = 5000
SOAK_TARGET_MS = 60 * 60 * 1000

INPUTS = (
    ("MODE", 14),
    ("UP", 15),
    ("DOWN", 16),
    ("ACTION", 22),
    ("TAMPER", 9),
)

pins = {name: Pin(gpio, Pin.IN, Pin.PULL_UP) for name, gpio in INPUTS}
service = Pin(26, Pin.IN, Pin.PULL_UP)

def safe_uname():
    try:
        u = os.uname()
        return "{}|{}|{}|{}|{}".format(u.sysname, u.nodename, u.release, u.version, u.machine)
    except Exception as exc:
        return "unavailable:{}".format(exc)

def print_state(prefix, started_ms, stable, event_seq):
    parts = []
    for name, _ in INPUTS:
        parts.append("{}={}".format(name, stable[name]))
    print("{} uptime_ms={} event_seq={} {}".format(
        prefix,
        time.ticks_diff(time.ticks_ms(), started_ms),
        event_seq,
        " ".join(parts),
    ))

print("MURMUR SYSTEM 001 / BLACK E-B0")
print("fw_id={}".format(FW_ID))
print("reset_cause={}".format(reset_cause()))
print("wdt_timeout_ms={}".format(WDT_TIMEOUT_MS))
print("soak_target_ms={}".format(SOAK_TARGET_MS))
print("service_gp26={}".format(service.value()))
print("uname={}".format(safe_uname()))
try:
    print("implementation={}".format(sys.implementation))
except Exception:
    pass

# WDT proof is deliberately separate from the 60-minute soak.
# Fit GP26 -> BLACK GND before boot. The firmware arms the watchdog and then
# intentionally stops feeding it. The host capture should observe a reboot.
wdt = WDT(timeout=WDT_TIMEOUT_MS)
if service.value() == 0:
    print("WDT_PROOF_ARMED: no feed; expect watchdog reset.")
    while True:
        time.sleep_ms(100)

stable = {name: pins[name].value() for name, _ in INPUTS}
candidate = dict(stable)
candidate_since = {name: time.ticks_ms() for name, _ in INPUTS}
event_counts = {name: 0 for name, _ in INPUTS}
event_seq = 0

started = time.ticks_ms()
last_heartbeat = started
soak_announced = False

print_state("BOOT_STATE", started, stable, event_seq)

while True:
    now = time.ticks_ms()

    for name, _ in INPUTS:
        value = pins[name].value()

        if value != candidate[name]:
            candidate[name] = value
            candidate_since[name] = now
            continue

        if value != stable[name] and time.ticks_diff(now, candidate_since[name]) >= DEBOUNCE_MS:
            stable[name] = value
            event_counts[name] += 1
            event_seq += 1

            if name == "TAMPER":
                meaning = "NORMAL_CLOSED" if value == 0 else "TAMPER_OPEN"
            else:
                meaning = "PRESSED" if value == 0 else "RELEASED"

            print(
                "EVENT uptime_ms={} seq={} name={} value={} meaning={} count={}".format(
                    time.ticks_diff(now, started),
                    event_seq,
                    name,
                    value,
                    meaning,
                    event_counts[name],
                )
            )

    if time.ticks_diff(now, last_heartbeat) >= HEARTBEAT_MS:
        last_heartbeat = now
        print_state("HEARTBEAT", started, stable, event_seq)

    if (not soak_announced) and time.ticks_diff(now, started) >= SOAK_TARGET_MS:
        soak_announced = True
        print_state("SOAK_COMPLETE", started, stable, event_seq)

    # Feed only after the input scan and reporting path completed.
    wdt.feed()
    time.sleep_ms(5)
