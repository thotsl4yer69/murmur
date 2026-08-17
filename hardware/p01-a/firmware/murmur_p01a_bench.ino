/*
 * MURMUR P01-A — bench firmware
 * Target: Raspberry Pi Pico 2 (RP2350, non-wireless)
 *
 * Bench/simulation firmware only. No optical/high-power actuator is enabled here.
 * It exercises the controller state machine, I2C bus, haptic output, status LED,
 * SD-card SPI pins, serial diagnostics, and a simulated motion input.
 *
 * Pin plan (P01-A):
 *   I2C SDA  GP4
 *   I2C SCL  GP5
 *   SD MISO  GP16
 *   SD CS    GP17
 *   SD SCK   GP18
 *   SD MOSI  GP19
 *   HAPTIC   GP20  -> DRV2605L INT/trigger test line (bench LED/buzzer substitute)
 *   TEST IN  GP21  -> momentary bench input, active LOW
 *   STATUS   GP25  -> onboard LED
 *
 * I2C devices on GP4/GP5:
 *   LIS3DH  0x18 (or 0x19)
 *   AS7343  0x39
 *   DRV2605L 0x5A
 *
 * The first implementation deliberately uses only Wire + GPIO so it can compile
 * in browser simulators without third-party library dependencies. Real sensor
 * drivers are added in the hardware integration branch after bus verification.
 */

#include <Arduino.h>
#include <Wire.h>

namespace Pins {
  constexpr uint8_t I2C_SDA = 4;
  constexpr uint8_t I2C_SCL = 5;
  constexpr uint8_t SD_MISO = 16;
  constexpr uint8_t SD_CS = 17;
  constexpr uint8_t SD_SCK = 18;
  constexpr uint8_t SD_MOSI = 19;
  constexpr uint8_t HAPTIC = 20;
  constexpr uint8_t TEST_IN = 21;
  constexpr uint8_t STATUS = LED_BUILTIN;
}

namespace Addr {
  constexpr uint8_t LIS3DH_A = 0x18;
  constexpr uint8_t LIS3DH_B = 0x19;
  constexpr uint8_t AS7343 = 0x39;
  constexpr uint8_t DRV2605L = 0x5A;
}

enum class State : uint8_t {
  BOOT,
  IDLE,
  ACTIVE,
  FAULT
};

State state = State::BOOT;
uint32_t bootMs = 0;
uint32_t lastReportMs = 0;
uint32_t lastBlinkMs = 0;
uint32_t activeUntilMs = 0;
bool lastButton = HIGH;
bool i2cFault = false;

const char* stateName(State s) {
  switch (s) {
    case State::BOOT: return "BOOT";
    case State::IDLE: return "IDLE";
    case State::ACTIVE: return "ACTIVE";
    case State::FAULT: return "FAULT";
  }
  return "UNKNOWN";
}

bool probeI2C(uint8_t address) {
  Wire.beginTransmission(address);
  return Wire.endTransmission() == 0;
}

void scanI2C() {
  Serial.println(F("I2C scan:"));
  uint8_t found = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    if (probeI2C(address)) {
      Serial.print(F("  0x"));
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      ++found;
    }
  }
  Serial.print(F("I2C devices found: "));
  Serial.println(found);
}

void evaluateExpectedDevices() {
  const bool lis = probeI2C(Addr::LIS3DH_A) || probeI2C(Addr::LIS3DH_B);
  const bool as = probeI2C(Addr::AS7343);
  const bool drv = probeI2C(Addr::DRV2605L);

  Serial.print(F("LIS3DH: ")); Serial.println(lis ? F("FOUND") : F("not detected (sim OK)"));
  Serial.print(F("AS7343: ")); Serial.println(as ? F("FOUND") : F("not detected (sim OK)"));
  Serial.print(F("DRV2605L: ")); Serial.println(drv ? F("FOUND") : F("not detected (sim OK)"));

  // Absence is not a fault during the browser/simulator bench stage.
  i2cFault = false;
}

void setState(State next) {
  if (next == state) return;
  state = next;
  Serial.print(F("STATE -> "));
  Serial.println(stateName(state));
}

void hapticPulse(uint16_t ms = 80) {
  digitalWrite(Pins::HAPTIC, HIGH);
  delay(ms);
  digitalWrite(Pins::HAPTIC, LOW);
}

void handleSerial() {
  while (Serial.available()) {
    const char c = Serial.read();

    if (c == 'i' || c == 'I') {
      scanI2C();
      evaluateExpectedDevices();
    } else if (c == 'h' || c == 'H') {
      Serial.println(F("HAPTIC TEST"));
      hapticPulse();
    } else if (c == 'a' || c == 'A') {
      Serial.println(F("BENCH ACTIVE TEST: 1 second"));
      activeUntilMs = millis() + 1000;
      setState(State::ACTIVE);
    } else if (c == 'r' || c == 'R') {
      Serial.println(F("Controller reset to IDLE"));
      setState(State::IDLE);
    } else if (c == '?') {
      Serial.println(F("Commands: I=I2C scan H=haptic A=active test R=reset"));
    }
  }
}

void updateButton() {
  const bool now = digitalRead(Pins::TEST_IN);
  if (lastButton == HIGH && now == LOW) {
    Serial.println(F("TEST INPUT -> ACTIVE"));
    activeUntilMs = millis() + 1000;
    setState(State::ACTIVE);
  }
  lastButton = now;
}

void updateState() {
  const uint32_t now = millis();
  if (state == State::ACTIVE && now >= activeUntilMs) {
    hapticPulse(50);
    setState(State::IDLE);
  }
}

void updateStatusLed() {
  const uint32_t now = millis();
  const uint32_t interval = (state == State::FAULT) ? 100 : (state == State::ACTIVE ? 80 : 500);
  if (now - lastBlinkMs >= interval) {
    lastBlinkMs = now;
    digitalWrite(Pins::STATUS, !digitalRead(Pins::STATUS));
  }
}

void report() {
  const uint32_t now = millis();
  if (now - lastReportMs < 2000) return;
  lastReportMs = now;

  // Deterministic simulated motion values make the simulator useful even without
  // a physical LIS3DH/AS7343 attached.
  const float t = now / 1000.0f;
  const float simX = 0.08f * sin(t);
  const float simY = 0.05f * cos(t * 0.7f);
  const float simZ = 1.00f + 0.02f * sin(t * 0.4f);

  Serial.print(F("MURMUR | state="));
  Serial.print(stateName(state));
  Serial.print(F(" | motion=["));
  Serial.print(simX, 3); Serial.print(F(", "));
  Serial.print(simY, 3); Serial.print(F(", "));
  Serial.print(simZ, 3); Serial.println(F("]g"));
}

void setup() {
  pinMode(Pins::STATUS, OUTPUT);
  pinMode(Pins::HAPTIC, OUTPUT);
  pinMode(Pins::TEST_IN, INPUT_PULLUP);

  digitalWrite(Pins::STATUS, LOW);
  digitalWrite(Pins::HAPTIC, LOW);

  Serial.begin(115200);
  delay(250);

  Wire.setSDA(Pins::I2C_SDA);
  Wire.setSCL(Pins::I2C_SCL);
  Wire.begin();
  Wire.setClock(100000);

  bootMs = millis();
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("MURMUR P01-A BENCH CONTROLLER"));
  Serial.println(F("RP2350 / Pico 2 non-wireless"));
  Serial.println(F("SIMULATION-SAFE OUTPUT PROFILE"));
  Serial.println(F("========================================"));

  scanI2C();
  evaluateExpectedDevices();
  Serial.println(F("Ready. ? for commands."));

  setState(State::IDLE);
}

void loop() {
  handleSerial();
  updateButton();
  updateState();
  updateStatusLed();
  report();
  delay(5);
}
