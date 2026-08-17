#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

namespace Pins { constexpr uint8_t SDA=4,SCL=5,SD_MISO=16,SD_CS=17,SD_SCK=18,SD_MOSI=19,HAPTIC=20,USER_TEST=21,STATUS=25,LOAD_EN=6,INTERLOCK=7,FAULT=8,TIMER_A=9,TIMER_B=10,LOAD_FB=11; }
enum class State:uint8_t{BOOT,SELF_TEST,SAFE_IDLE,ARMED,ACTIVE,COOLDOWN,FAULT};
State state=State::BOOT; uint32_t stateSince=0;
constexpr uint32_t ACTIVE_LIMIT_MS=1000, COOLDOWN_MS=1500;
void loadOff(){digitalWrite(Pins::LOAD_EN,LOW);} void safeOutputs(){loadOff();digitalWrite(Pins::HAPTIC,LOW);}
void setState(State s){state=s;stateSince=millis();if(s!=State::ACTIVE)loadOff();Serial.printf("STATE=%u\n",(unsigned)s);}
bool proofsOK(){return digitalRead(Pins::INTERLOCK)&&digitalRead(Pins::TIMER_A)&&digitalRead(Pins::TIMER_B)&&digitalRead(Pins::FAULT);}
void scanI2C(){Serial.println("I2C scan");for(uint8_t a=1;a<127;a++){Wire.beginTransmission(a);if(Wire.endTransmission()==0)Serial.printf("0x%02X\n",a);}}
void diagnostics(){Serial.printf("FAULT=%d INT=%d TA=%d TB=%d FB=%d EN=%d\n",digitalRead(Pins::FAULT)==LOW,digitalRead(Pins::INTERLOCK),digitalRead(Pins::TIMER_A),digitalRead(Pins::TIMER_B),digitalRead(Pins::LOAD_FB),digitalRead(Pins::LOAD_EN));}
void setup(){
 pinMode(Pins::LOAD_EN,OUTPUT);digitalWrite(Pins::LOAD_EN,LOW);pinMode(Pins::HAPTIC,OUTPUT);digitalWrite(Pins::HAPTIC,LOW);pinMode(Pins::STATUS,OUTPUT);digitalWrite(Pins::STATUS,LOW);
 pinMode(Pins::USER_TEST,INPUT_PULLUP);pinMode(Pins::INTERLOCK,INPUT_PULLDOWN);pinMode(Pins::FAULT,INPUT_PULLUP);pinMode(Pins::TIMER_A,INPUT_PULLDOWN);pinMode(Pins::TIMER_B,INPUT_PULLDOWN);pinMode(Pins::LOAD_FB,INPUT_PULLDOWN);
 pinMode(Pins::SD_CS,OUTPUT);digitalWrite(Pins::SD_CS,HIGH);Wire.setSDA(Pins::SDA);Wire.setSCL(Pins::SCL);Wire.begin();Serial.begin(115200);delay(50);Serial.println("MURMUR P01-A / RP2350");setState(State::SELF_TEST);
}
void loop(){uint32_t now=millis();
 if(Serial.available()){char c=tolower(Serial.read());if(c=='i')scanI2C();else if(c=='f')diagnostics();else if(c=='r'){safeOutputs();digitalWrite(Pins::STATUS,LOW);setState(State::SAFE_IDLE);}else if(c=='a'&&state==State::SAFE_IDLE&&proofsOK())setState(State::ARMED);else if(c=='t'&&(state==State::ARMED||state==State::SAFE_IDLE)&&proofsOK())setState(State::ACTIVE);}
 if(state==State::SELF_TEST&&now-stateSince>100){if(digitalRead(Pins::FAULT)==HIGH)setState(State::SAFE_IDLE);else setState(State::FAULT);}
 if(state==State::SAFE_IDLE){loadOff();if(digitalRead(Pins::USER_TEST)==LOW&&proofsOK())setState(State::ACTIVE);else if(proofsOK())setState(State::ARMED);}
 if(state==State::ARMED){loadOff();if(!proofsOK())setState(State::SAFE_IDLE);else if(digitalRead(Pins::USER_TEST)==LOW)setState(State::ACTIVE);}
 if(state==State::ACTIVE){if(!proofsOK()||now-stateSince>=ACTIVE_LIMIT_MS){loadOff();setState(State::COOLDOWN);}else{digitalWrite(Pins::LOAD_EN,HIGH);digitalWrite(Pins::STATUS,HIGH);if(digitalRead(Pins::LOAD_FB)!=HIGH){loadOff();setState(State::FAULT);}}}
 if(state==State::COOLDOWN){loadOff();if(now-stateSince>=COOLDOWN_MS){digitalWrite(Pins::STATUS,LOW);setState(State::SAFE_IDLE);}}
 if(state==State::FAULT){safeOutputs();digitalWrite(Pins::STATUS,HIGH);}
 delay(1);
}
