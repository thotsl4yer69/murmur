// MURMUR P01-A Bruce node
// ESP32-S3 communications/operator interface.
// Bruce is non-authoritative: it never directly controls the P01-A load path.

#include <Arduino.h>

#ifndef BRUCE_RX_PIN
#define BRUCE_RX_PIN 44
#endif
#ifndef BRUCE_TX_PIN
#define BRUCE_TX_PIN 43
#endif

HardwareSerial UPLINK(1);

String line;

void sendStatusRequest() {
  UPLINK.println("STATUS?");
}

void setup() {
  Serial.begin(115200);
  delay(50);
  UPLINK.begin(115200, SERIAL_8N1, BRUCE_RX_PIN, BRUCE_TX_PIN);
  Serial.println("MURMUR BRUCE / ESP32-S3");
  Serial.println("NON-AUTHORITATIVE COMMUNICATION NODE");
  sendStatusRequest();
}

void loop() {
  while (UPLINK.available()) {
    char c = (char)UPLINK.read();
    Serial.write(c);
  }

  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r') {
      if (line.length()) {
        // Forward only advisory/operator messages. U1 remains authoritative.
        UPLINK.println(line);
        line = "";
      }
    } else if (line.length() < 96) {
      line += c;
    }
  }

  delay(2);
}
