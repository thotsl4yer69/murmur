from http.client import HTTPConnection
from contextlib import closing
from http.server import ThreadingHTTPServer
from pathlib import Path
import json
import sqlite3
import tempfile
import threading
import unittest
from telemetry import Samples, handler


class ReceiverTests(unittest.TestCase):
    def test_http_and_persistence(self):
        with tempfile.TemporaryDirectory() as folder:
            path = Path(folder) / "capture.sqlite"
            samples = Samples(path)
            with ThreadingHTTPServer(("127.0.0.1", 0), handler(samples)) as service:
                thread = threading.Thread(target=service.serve_forever)
                thread.start()
                try:
                    def get():
                        with_connection = HTTPConnection(*service.server_address)
                        try:
                            with_connection.request("GET", "/api/status")
                            response = with_connection.getresponse()
                            return response.status, json.loads(response.read())
                        finally:
                            with_connection.close()
                    self.assertFalse(samples.accept("BOOT"))
                    self.assertEqual(get(), (503, {"available": False}))
                    self.assertTrue(samples.accept("# BLACKNODE 1.0.0 profile=1 boot=000000AF job=WIFI RX heap=24000 minheap=22000 uptime_ms=100 reset=1"))
                    status, data = get()
                    self.assertEqual(status, 200)
                    self.assertEqual(data["heap"], 24000)
                    with closing(sqlite3.connect(path)) as db:
                        self.assertEqual(json.loads(db.execute("SELECT record FROM status").fetchone()[0]), data)
                    samples.received -= 13
                    self.assertEqual(get()[0], 503)
                    samples.disconnect()
                    self.assertIsNone(samples.current())
                finally:
                    service.shutdown()
                    thread.join()


if __name__ == "__main__":
    unittest.main()
