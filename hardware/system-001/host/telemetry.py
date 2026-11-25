"""Serve BLACKNODE status over local HTTP and retain actual samples in SQLite."""
import argparse
from contextlib import closing
from datetime import datetime, timezone
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import re
import sqlite3
import threading
import time

STATUS = re.compile(
    r"# BLACKNODE (?P<firmware>[\w.-]+) profile=(?P<profile>[1-4]) "
    r"boot=(?P<boot>[0-9a-fA-F]{8}) job=(?P<job>[\w -]+) heap=(?P<heap>\d+) "
    r"minheap=(?P<minheap>\d+) uptime_ms=(?P<uptime_ms>\d+) reset=(?P<reset>\d+)"
)


class Samples:
    def __init__(self, database):
        self.database = database
        self.lock = threading.Lock()
        self.sample = None
        self.received = 0
        with closing(sqlite3.connect(database)) as db, db:
            db.execute("CREATE TABLE IF NOT EXISTS status (id INTEGER PRIMARY KEY, utc TEXT, record TEXT)")

    def accept(self, line):
        match = STATUS.fullmatch(line)
        if not match:
            return False
        sample = match.groupdict()
        for key in ("profile", "heap", "minheap", "uptime_ms", "reset"):
            sample[key] = int(sample[key])
        sample["utc"] = datetime.now(timezone.utc).isoformat()
        with closing(sqlite3.connect(self.database)) as db, db:
            db.execute("INSERT INTO status(utc,record) VALUES(?,?)",
                       (sample["utc"], json.dumps(sample)))
        with self.lock:
            self.sample, self.received = sample, time.monotonic()
        return True

    def disconnect(self):
        with self.lock:
            self.sample = None

    def current(self):
        with self.lock:
            if self.sample is None or time.monotonic() - self.received > 12:
                return None
            return dict(self.sample)


def poll(serial_module, port, samples, stop):
    while not stop.is_set():
        try:
            with serial_module.Serial(port, 115200, timeout=0.5, write_timeout=1) as device:
                next_poll = 0
                pending = bytearray()
                while not stop.is_set():
                    now = time.monotonic()
                    if now >= next_poll:
                        device.write(b"status\n")
                        next_poll = now + 5
                    data = device.read(device.in_waiting or 1)
                    pending.extend(data)
                    while b"\n" in pending:
                        record, _, remainder = pending.partition(b"\n")
                        pending = bytearray(remainder)
                        if len(record) <= 512:
                            samples.accept(record.rstrip(b"\r").decode("ascii", "replace"))
                    if len(pending) > 4096:
                        raise ValueError("Serial line exceeded buffer")
        except (OSError, ValueError, sqlite3.Error) as exc:
            samples.disconnect()
            print(f"Capture unavailable on {port}: {exc}", flush=True)
            stop.wait(3)


PAGE = b"""<!doctype html><html lang=en><meta charset=utf-8><meta name=viewport content="width=device-width">
<title>MURMUR telemetry</title><style>body{font:18px system-ui;background:#14171b;color:#e0e6ec;max-width:48rem;margin:8vh auto;padding:1rem}dt{color:#aab7bd}dd{margin:0 0 .8rem;font-family:monospace}</style>
<h1>BLACKNODE telemetry</h1><p id=state>Connecting</p><dl id=fields></dl>
<script>async function refresh(){try{const response=await fetch('/api/status',{cache:'no-store'});
if(!response.ok)throw Error();const data=await response.json();fields.replaceChildren();
for(const [name,value] of Object.entries(data)){const label=document.createElement('dt'),field=document.createElement('dd');
label.textContent=name;field.textContent=value;fields.append(label,field)}state.textContent='Device online';
}catch{fields.replaceChildren();state.textContent='Awaiting device telemetry'}finally{setTimeout(refresh,3000)}}refresh()</script></html>"""


def handler(samples):
    class Handler(BaseHTTPRequestHandler):
        def do_GET(self):
            if self.path == "/":
                status, kind, body = 200, "text/html", PAGE
            elif self.path == "/api/status":
                data = samples.current()
                status, kind = (200 if data else 503), "application/json"
                body = json.dumps(data or {"available": False}).encode()
            else:
                self.send_error(404)
                return
            self.send_response(status)
            self.send_header("Cache-Control", "no-store")
            self.send_header("Content-Type", kind + "; charset=utf-8")
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)
    return Handler


def main():
    import serial
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--port", required=True)
    parser.add_argument("--database", default="blacknode.sqlite3")
    parser.add_argument("--bind", default="127.0.0.1")
    parser.add_argument("--http-port", type=int, default=8765)
    args = parser.parse_args()
    samples = Samples(args.database)
    stop = threading.Event()
    with ThreadingHTTPServer((args.bind, args.http_port), handler(samples)) as server:
        worker = threading.Thread(target=poll, args=(serial, args.port, samples, stop), daemon=True)
        worker.start()
        try:
            server.serve_forever()
        except KeyboardInterrupt:
            pass
        finally:
            stop.set()
            worker.join(3)


if __name__ == "__main__":
    main()
