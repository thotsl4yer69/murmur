#include "../MURMUR_C3/Config.h"
#include <cassert>
#include <cstdio>
int main() {
#if MURMUR_TFT
#if MURMUR_RF
  int pins[]={cfg::sck,cfg::mosi,cfg::miso,cfg::cs,cfg::dc,cfg::radioCs,cfg::radioGdo,cfg::buttons[0],cfg::buttons[1],cfg::buttons[2],cfg::buttons[3]};
  static_assert(cfg::rst==-1 && cfg::bl==-1,"RF harness frees reset/backlight GPIO");
#else
  int pins[]={cfg::sck,cfg::mosi,cfg::cs,cfg::dc,cfg::rst,cfg::bl,cfg::buttons[0],cfg::buttons[1],cfg::buttons[2],cfg::buttons[3]};
#endif
#else
  int pins[]={cfg::sda,cfg::scl,cfg::sck,cfg::mosi,cfg::miso,cfg::radioCs,cfg::radioGdo,cfg::buttons[0],cfg::buttons[1],cfg::buttons[2],cfg::buttons[3]};
#endif
  for(unsigned i=0;i<sizeof(pins)/sizeof(pins[0]);++i) {
    assert(pins[i]>=0 && pins[i]<=21);
    assert(pins[i]<12 || pins[i]>19); // No flash or native USB pins.
    for(unsigned j=i+1;j<sizeof(pins)/sizeof(pins[0]);++j) assert(pins[i]!=pins[j]);
  }
  for(int pin:cfg::buttons) assert(pin!=2 && pin!=8 && pin!=9);
  assert(cfg::bandLow<=cfg::center && cfg::center<=cfg::bandHigh);
  assert(cfg::bins>=2 && cfg::maxPacket<=48);
  std::printf("PASS profile %d / band %d: unique GPIO, buttons avoid boot straps, USB/flash reserved, RF range valid\n",cfg::profile,MURMUR_RF_BAND);
}
