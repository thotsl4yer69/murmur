"""
MURMUR SYSTEM 001 — BLACK E-B0 bench firmware R0.1
Target: Raspberry Pi Pico 2 (RP2350, non-wireless), MicroPython.
Scope: G3 bench evidence only. No sensors, radios, haptics, switched rails, or application logic.

DG03 inputs:
  MODE   GP14 / physical pin 19
  UP     GP15 / physical pin 20
  DOWN   GP16 / physical pin 21
  ACTION GP22 / physical pin 29
  TAMPER GP9  / physical pin 12, NC to BLACK GND
  SERVICE GP26 / physical pin 31, optional WDT proof jumper to BLACK GND
"""
from machine import Pin, WDT, reset_cause
import time

FW_ID = "MUR-S001-BLACK-EB0-R0.1"
WDT_TIMEOUT_MS = 4000
DEBOUNCE_MS = 25
HEARTBEAT_MS = 5000

INPUTS = (
    ("MODE", 14),
    ("UP", 15),
    ("DOWN", 16),
    ("ACTION", 22),
    ("TAMPER", 9),
)

pins = {name: Pin(gpio, Pin.IN, Pin.PULL_UP) for name, gpio in INPUTS}
service = Pin(26, Pin.IN, Pin.PULL_UP)

print("MURMUR SYSTEM 001 / BLACK E-B0")
print("fw_id={}".format(FW_ID))
print("reset_cause={}".format(reset_cause()))
print("wdt_timeout_ms={}".format(WDT_TIMEOUT_MS))
print("service_gp26={}".format(service.value()))

# WDT proof mode is deliberate and separate from the 60-minute soak.
# Fit a jumper GP26 -> BLACK GND before boot to prove the watchdog resets the MCU.
wdt = WDT(timeout=WDT_TIMEOUT_MS)
if service.value() == 0:
    print("WDT_PROOF_ARMED: watchdog will NOT be fed; expect reset within timeout.")
    while True:
        time.sleep_ms(100)

stable = {name: pins[name].value() for name, _ in INPUTS}
candidate = dict(stable)
candidate_since = {name: time.ticks_ms() for name, _ in INPUTS}
events = {name: 0 for name, _ in INPUTS}

started = time.ticks_ms()
last_heartbeat = started

def print_state(prefix):
    parts = []
    for name, _ in INPUTS:
        parts.append("{}={}".format(name, stable[name]))
    print("{} uptime_ms={} {}".format(
        prefix, time.ticks_diff(time.ticks_ms(), started), " ".join(parts)
    ))

print_state("BOOT_STATE")

while True:
    now = time.ticks_ms()

    for name, _ in INPUTS:
        value = pins[name].value()
        if value != candidate[name]:
            candidate[name] = value
            candidate_since[name] = now
        elif value != stable[name] and time.ticks_diff(now, candidate_since[name]) >= DEBOUNCE_MS:
            stable[name] = value
            events[name] += 1
            if name == "TAMPER":
                meaning = "NORMAL_CLOSED" if value == 0 else "TAMPER_OPEN"
            else:
                meaning = "PRESSED" if value == 0 else "RELEASED"
            print("EVENT uptime_ms={} name={} value={} meaning={} count={}".format(
                time.ticks_diff(now, started), name, value, meaning, events[name]
            ))

    if time.ticks_diff(now, last_heartbeat) >= HEARTBEAT_MS:
        last_heartbeat = now
        print_state("HEARTBEAT")

    wdt.feed()
    time.sleep_ms(5)
