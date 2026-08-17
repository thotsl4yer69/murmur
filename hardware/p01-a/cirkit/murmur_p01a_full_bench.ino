/*
 * MURMUR P01-A — FULL BENCH/SIMULATION CONTROLLER
 * Target: Pico 2 / RP2350 (non-wireless)
 *
 * This firmware validates the complete low-voltage controller architecture.
 * The physical/high-power actuator is NOT driven by this sketch; GP22 is a
 * permission/state signal connected only to a low-power simulation indicator.
 */

#include <Wire.h>
#include <SPI.h>

static constexpr uint8_t PIN_SDA = 4;
static constexpr uint8_t PIN_SCL = 5;
static constexpr uint8_t PIN_SD_MISO = 16;
static constexpr uint8_t PIN_SD_CS = 17;
static constexpr uint8_t PIN_SD_SCK = 18;
static constexpr uint8_t PIN_SD_MOSI = 19;
static constexpr uint8_t PIN_HAPTIC = 20;
static constexpr uint8_t PIN_TEST = 21;
static constexpr uint8_t PIN_LOAD = 22;
static constexpr uint8_t PIN_INTERLOCK = 23;
static constexpr uint8_t PIN_FAULT = 24;
static constexpr uint8_t PIN_STATUS = 25;

static constexpr uint32_t ACTIVE_MS = 1000;
static constexpr uint32_t COOLDOWN_MS = 1000;
static constexpr uint32_t WATCHDOG_PERIOD_MS = 4000;

enum class State : uint8_t { BOOT, SELFTEST, IDLE, ARMED, ACTIVE, COOLDOWN, FAULT };
State state = State::BOOT;
uint32_t stateSince = 0;
uint32_t lastHeartbeat = 0;
uint32_t eventCounter = 0;

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

void logEvent(const char* event) {
  Serial.print("EVENT,");
  Serial.print(++eventCounter);
  Serial.print(',');
  Serial.print(millis());
  Serial.print(',');
  Serial.print(stateName(state));
  Serial.print(',');
  Serial.print(event);
  Serial.print(",interlock=");
  Serial.print(digitalRead(PIN_INTERLOCK));
  Serial.print(",fault=");
  Serial.print(digitalRead(PIN_FAULT));
  Serial.print(",load=");
  Serial.println(digitalRead(PIN_LOAD));
}

void hardOff() {
  digitalWrite(PIN_LOAD, LOW);
  digitalWrite(PIN_HAPTIC, LOW);
}

void enterState(State next, const char* reason) {
  hardOff();
  state = next;
  stateSince = millis();
  logEvent(reason);
}

bool interlockHealthy() {
  return digitalRead(PIN_INTERLOCK) == HIGH;
}

bool faultAsserted() {
  return digitalRead(PIN_FAULT) == LOW;
}

bool preconditionsOK() {
  return interlockHealthy() && !faultAsserted();
}

void i2cScan() {
  Serial.println("I2C_SCAN_BEGIN");
  uint8_t found = 0;
  for (uint8_t address = 1; address < 127; ++address) {
    Wire.beginTransmission(address);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C_FOUND,0x");
      if (address < 16) Serial.print('0');
      Serial.println(address, HEX);
      found++;
    }
  }
  Serial.print("I2C_SCAN_END,count=");
  Serial.println(found);
}

bool selfTest() {
  hardOff();
  bool ok = true;
  if (faultAsserted()) ok = false;
  if (!interlockHealthy()) ok = false;

  Wire.beginTransmission(0x5A);
  const bool hapticPresent = (Wire.endTransmission() == 0);
  (void)hapticPresent; // A simulation may not expose the exact DRV2605L.

  Serial.print("SELFTEST,interlock=");
  Serial.print(interlockHealthy());
  Serial.print(",fault=");
  Serial.print(faultAsserted());
  Serial.print(",result=");
  Serial.println(ok ? "PASS" : "BLOCKED");
  return ok;
}

void hapticTest() {
  if (!preconditionsOK()) {
    logEvent("HAPTIC_BLOCKED");
    return;
  }
  digitalWrite(PIN_HAPTIC, HIGH);
  delay(100);
  digitalWrite(PIN_HAPTIC, LOW);
  logEvent("HAPTIC_TEST");
}

void requestActivation() {
  if (!preconditionsOK()) {
    hardOff();
    enterState(State::FAULT, "ACTIVATION_BLOCKED");
    return;
  }
  enterState(State::ACTIVE, "ACTIVATION_START");
  digitalWrite(PIN_LOAD, HIGH);
  digitalWrite(PIN_HAPTIC, HIGH);
}

void processCommand(char c) {
  switch (c) {
    case 'I': case 'i': i2cScan(); break;
    case 'H': case 'h': hapticTest(); break;
    case 'A': case 'a': requestActivation(); break;
    case 'F': case 'f':
      if (!faultAsserted()) Serial.println("FAULT_INPUT_IS_HIGH");
      else enterState(State::FAULT, "COMMAND_FAULT");
      break;
    case 'R': case 'r':
      hardOff();
      if (!faultAsserted()) enterState(State::SELFTEST, "MANUAL_RESET");
      break;
    case 'S': case 's':
      Serial.print("STATUS,state="); Serial.print(stateName(state));
      Serial.print(",interlock="); Serial.print(interlockHealthy());
      Serial.print(",fault="); Serial.print(faultAsserted());
      Serial.print(",load="); Serial.println(digitalRead(PIN_LOAD));
      break;
    case '?':
      Serial.println("COMMANDS: I=I2C H=HAPTIC A=ACTIVATE R=RESET S=STATUS");
      break;
  }
}

void setup() {
  pinMode(PIN_LOAD, OUTPUT);
  pinMode(PIN_HAPTIC, OUTPUT);
  pinMode(PIN_STATUS, OUTPUT);
  pinMode(PIN_TEST, INPUT_PULLUP);
  pinMode(PIN_FAULT, INPUT_PULLUP);
  pinMode(PIN_INTERLOCK, INPUT_PULLDOWN);
  pinMode(PIN_SD_CS, OUTPUT);

  hardOff();
  digitalWrite(PIN_SD_CS, HIGH);
  digitalWrite(PIN_STATUS, LOW);

  Serial.begin(115200);
  delay(100);
  Wire.setSDA(PIN_SDA);
  Wire.setSCL(PIN_SCL);
  Wire.begin();
  SPI.setRX(PIN_SD_MISO);
  SPI.setSCK(PIN_SD_SCK);
  SPI.setTX(PIN_SD_MOSI);
  SPI.begin();

  Serial.println("MURMUR_P01A_FULL_BENCH_START");
  enterState(State::SELFTEST, "BOOT");
}

void loop() {
  const uint32_t now = millis();

  // Highest-priority safety conditions.
  if (faultAsserted() || !interlockHealthy()) {
    if (state != State::FAULT) enterState(State::FAULT, "INTERLOCK_OR_FAULT");
    hardOff();
  }

  if (Serial.available()) processCommand(Serial.read());

  if (digitalRead(PIN_TEST) == LOW && state != State::ACTIVE && state != State::COOLDOWN && state != State::FAULT) {
    requestActivation();
    delay(30); // debounce only; activation remains bounded by state timer.
  }

  switch (state) {
    case State::SELFTEST:
      if (selfTest()) enterState(State::IDLE, "SELFTEST_PASS");
      else enterState(State::FAULT, "SELFTEST_BLOCKED");
      break;

    case State::IDLE:
      if (preconditionsOK()) enterState(State::ARMED, "ARMED");
      break;

    case State::ARMED:
      if (!preconditionsOK()) {
        enterState(State::FAULT, "ARMED_LOST_PRECONDITION");
      }
      break;

    case State::ACTIVE:
      if (!preconditionsOK() || now - stateSince >= ACTIVE_MS) {
        hardOff();
        enterState(State::COOLDOWN, "ACTIVE_END");
      }
      break;

    case State::COOLDOWN:
      if (now - stateSince >= COOLDOWN_MS) {
        enterState(State::IDLE, "COOLDOWN_END");
      }
      break;

    case State::FAULT:
      hardOff();
      digitalWrite(PIN_STATUS, (now / 250) & 1);
      if (!faultAsserted() && interlockHealthy()) {
        enterState(State::SELFTEST, "FAULT_CLEARED");
      }
      break;

    case State::BOOT:
      hardOff();
      enterState(State::SELFTEST, "BOOT_RECOVERY");
      break;
  }

  if (state != State::FAULT) {
    digitalWrite(PIN_STATUS, state == State::ACTIVE ? HIGH : ((now / 500) & 1));
  }

  // Simple application-level heartbeat. A real physical build should also
  // use the RP2350 watchdog; this heartbeat is intentionally observable in
  // simulation and does not replace a hardware safety mechanism.
  if (now - lastHeartbeat >= WATCHDOG_PERIOD_MS) {
    lastHeartbeat = now;
    Serial.println("HEARTBEAT");
  }
}
