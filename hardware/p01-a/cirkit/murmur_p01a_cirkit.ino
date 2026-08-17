#include <Wire.h>
#include <SPI.h>

// MURMUR P01-A Cirkit bench firmware
// Simulation-safe: GP22 only drives a low-power indicator/load surrogate.

namespace Pin {
  constexpr uint8_t SDA_PIN = 4;
  constexpr uint8_t SCL_PIN = 5;
  constexpr uint8_t SD_MISO = 16;
  constexpr uint8_t SD_CS   = 17;
  constexpr uint8_t SD_SCK  = 18;
  constexpr uint8_t SD_MOSI = 19;
  constexpr uint8_t HAPTIC  = 20;
  constexpr uint8_t TEST    = 21;
  constexpr uint8_t LOAD_EN = 22;
  constexpr uint8_t INTERLOCK = 23;
  constexpr uint8_t FAULT   = 24;
  constexpr uint8_t STATUS  = 25;
}

enum class State : uint8_t {
  BOOT,
  SELFTEST,
  IDLE,
  ARMED,
  ACTIVE,
  COOLDOWN,
  FAULT
};

State state = State::BOOT;
unsigned long stateStarted = 0;
unsigned long lastReport = 0;

const char* stateName(State s) {
  switch (s) {
    case State::BOOT: return "BOOT";
    case State::SELFTEST: return "SELFTEST";
    case State::IDLE: return "IDLE";
    case State::ARMED: return "ARMED";
    case State::ACTIVE: return "ACTIVE";
    case State::COOLDOWN: return "COOLDOWN";
    case State::FAULT: return "FAULT";
  }
  return "UNKNOWN";
}

bool faultAsserted() {
  return digitalRead(Pin::FAULT) == LOW;
}

bool interlockHealthy() {
  return digitalRead(Pin::INTERLOCK) == HIGH;
}

void forceLoadOff() {
  digitalWrite(Pin::LOAD_EN, LOW);
}

void enter(State next) {
  state = next;
  stateStarted = millis();
  Serial.print("STATE -> ");
  Serial.println(stateName(state));
}

void hapticTest() {
  // Low-power bench surrogate: brief digital pulse.
  digitalWrite(Pin::HAPTIC, HIGH);
  delay(120);
  digitalWrite(Pin::HAPTIC, LOW);
}

void i2cScan() {
  Serial.println("I2C SCAN");
  uint8_t found = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("  0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      found++;
    }
  }
  Serial.print("I2C devices: ");
  Serial.println(found);
}

void selfTest() {
  forceLoadOff();
  digitalWrite(Pin::STATUS, LOW);

  Serial.println("SELFTEST: outputs safe");
  Serial.print("INTERLOCK: ");
  Serial.println(interlockHealthy() ? "HEALTHY" : "NOT PROVEN");
  Serial.print("FAULT: ");
  Serial.println(faultAsserted() ? "ASSERTED" : "CLEAR");

  i2cScan();
  digitalWrite(Pin::STATUS, HIGH);
}

void activateBenchTest() {
  // Never activate unless the simulated interlock is proven and no fault exists.
  if (faultAsserted() || !interlockHealthy()) {
    Serial.println("ACTIVE denied: interlock/fault condition");
    return;
  }

  enter(State::ACTIVE);
  digitalWrite(Pin::LOAD_EN, HIGH);
  hapticTest();
}

void handleCommand(char c) {
  if (c == 'I' || c == 'i') {
    i2cScan();
  } else if (c == 'H' || c == 'h') {
    hapticTest();
  } else if (c == 'A' || c == 'a') {
    activateBenchTest();
  } else if (c == 'R' || c == 'r') {
    forceLoadOff();
    enter(State::IDLE);
  } else if (c == '?') {
    Serial.println("I=I2C scan H=haptic A=bounded active R=reset-to-idle");
  }
}

void setup() {
  pinMode(Pin::HAPTIC, OUTPUT);
  pinMode(Pin::LOAD_EN, OUTPUT);
  pinMode(Pin::STATUS, OUTPUT);
  pinMode(Pin::TEST, INPUT_PULLUP);
  pinMode(Pin::FAULT, INPUT_PULLUP);
  pinMode(Pin::INTERLOCK, INPUT_PULLDOWN);

  forceLoadOff();
  digitalWrite(Pin::HAPTIC, LOW);
  digitalWrite(Pin::STATUS, LOW);

  Serial.begin(115200);
  Wire.setSDA(Pin::SDA_PIN);
  Wire.setSCL(Pin::SCL_PIN);
  Wire.begin();

  Serial.println();
  Serial.println("=== MURMUR P01-A CIRKIT BENCH ===");
  Serial.println("Simulation-safe controller validation");
  enter(State::SELFTEST);
}

void loop() {
  if (faultAsserted()) {
    forceLoadOff();
    if (state != State::FAULT) enter(State::FAULT);
  }

  switch (state) {
    case State::BOOT:
      forceLoadOff();
      enter(State::SELFTEST);
      break;

    case State::SELFTEST:
      if (millis() - stateStarted >= 500) {
        selfTest();
        enter(State::IDLE);
      }
      break;

    case State::IDLE:
      forceLoadOff();
      if (digitalRead(Pin::TEST) == LOW) {
        delay(25);
        if (digitalRead(Pin::TEST) == LOW) activateBenchTest();
      }
      break;

    case State::ARMED:
      forceLoadOff();
      if (!interlockHealthy()) enter(State::IDLE);
      break;

    case State::ACTIVE:
      if (faultAsserted() || !interlockHealthy()) {
        forceLoadOff();
        enter(State::FAULT);
      } else if (millis() - stateStarted >= 1000) {
        forceLoadOff();
        enter(State::COOLDOWN);
      }
      break;

    case State::COOLDOWN:
      forceLoadOff();
      if (millis() - stateStarted >= 750) enter(State::IDLE);
      break;

    case State::FAULT:
      forceLoadOff();
      digitalWrite(Pin::STATUS, (millis() / 250) & 1);
      if (!faultAsserted()) {
        digitalWrite(Pin::STATUS, HIGH);
        enter(State::SELFTEST);
      }
      break;
  }

  if (Serial.available()) {
    handleCommand(static_cast<char>(Serial.read()));
  }

  if (millis() - lastReport >= 1000) {
    lastReport = millis();
    Serial.print("STATUS state=");
    Serial.print(stateName(state));
    Serial.print(" interlock=");
    Serial.print(interlockHealthy() ? 1 : 0);
    Serial.print(" fault=");
    Serial.print(faultAsserted() ? 1 : 0);
    Serial.print(" load=");
    Serial.println(digitalRead(Pin::LOAD_EN));
  }
}
