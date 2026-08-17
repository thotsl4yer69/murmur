#include <Arduino.h>
#include <Wire.h>
#include <math.h>

constexpr uint8_t I2C_SDA = 4;
constexpr uint8_t I2C_SCL = 5;
constexpr uint8_t HAPTIC = 20;
constexpr uint8_t TEST_IN = 21;
constexpr uint8_t STATUS = LED_BUILTIN;

enum class State : uint8_t { BOOT, IDLE, ACTIVE, FAULT };
State state = State::BOOT;
uint32_t activeUntil = 0;
uint32_t lastReport = 0;
uint32_t lastBlink = 0;
bool previousButton = HIGH;

const char* stateName(State s) {
  switch (s) {
    case State::BOOT: return "BOOT";
    case State::IDLE: return "IDLE";
    case State::ACTIVE: return "ACTIVE";
    case State::FAULT: return "FAULT";
  }
  return "?";
}

void setState(State next) {
  if (next == state) return;
  state = next;
  Serial.print("STATE -> ");
  Serial.println(stateName(state));
}

void i2cScan() {
  uint8_t count = 0;
  Serial.println("I2C scan (expected real P01-A addresses: 0x18/0x19, 0x39, 0x5A)");
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("  found 0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      ++count;
    }
  }
  Serial.print("devices=");
  Serial.println(count);
}

void pulseHaptic() {
  digitalWrite(HAPTIC, HIGH);
  delay(80);
  digitalWrite(HAPTIC, LOW);
}

void setup() {
  pinMode(STATUS, OUTPUT);
  pinMode(HAPTIC, OUTPUT);
  pinMode(TEST_IN, INPUT_PULLUP);
  digitalWrite(STATUS, LOW);
  digitalWrite(HAPTIC, LOW);

  Serial.begin(115200);
  delay(250);

  Wire.setSDA(I2C_SDA);
  Wire.setSCL(I2C_SCL);
  Wire.begin();
  Wire.setClock(100000);

  Serial.println();
  Serial.println("=== MURMUR P01-A BENCH SIM ===");
  Serial.println("RP2350/Pico 2 pin-compatible simulation target");
  Serial.println("Outputs are bench-safe: GP20 is represented by a low-volume buzzer.");
  Serial.println("Commands: I=I2C scan, H=haptic, A=active, R=reset, ?=help");
  i2cScan();
  setState(State::IDLE);
}

void loop() {
  const uint32_t now = millis();

  while (Serial.available()) {
    const char c = Serial.read();
    if (c == 'i' || c == 'I') i2cScan();
    if (c == 'h' || c == 'H') pulseHaptic();
    if (c == 'a' || c == 'A') {
      activeUntil = now + 1000;
      setState(State::ACTIVE);
    }
    if (c == 'r' || c == 'R') setState(State::IDLE);
    if (c == '?') Serial.println("I=scan H=haptic A=active R=reset");
  }

  const bool button = digitalRead(TEST_IN);
  if (previousButton == HIGH && button == LOW) {
    activeUntil = now + 1000;
    setState(State::ACTIVE);
    pulseHaptic();
  }
  previousButton = button;

  if (state == State::ACTIVE && now >= activeUntil) {
    pulseHaptic();
    setState(State::IDLE);
  }

  const uint32_t blinkEvery = state == State::ACTIVE ? 80 : 500;
  if (now - lastBlink >= blinkEvery) {
    lastBlink = now;
    digitalWrite(STATUS, !digitalRead(STATUS));
  }

  if (now - lastReport >= 2000) {
    lastReport = now;
    const float t = now / 1000.0f;
    Serial.print("MURMUR | state=");
    Serial.print(stateName(state));
    Serial.print(" | simulated_motion=[");
    Serial.print(0.08f * sin(t), 3);
    Serial.print(", ");
    Serial.print(0.05f * cos(t * 0.7f), 3);
    Serial.print(", ");
    Serial.print(1.0f + 0.02f * sin(t * 0.4f), 3);
    Serial.println("]g");
  }

  delay(5);
}
