#include "../MURMUR_C3/Core.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
int main() {
  assert(core::wrap(-1,8)==7); assert(core::wrap(9,8)==1); assert(core::wrap(1,0)==0);
  assert(core::elapsed(10,0xfffffff0,26)); assert(!core::elapsed(10,0xfffffff0,27));
  core::Button b;
  b.update(true,100); b.update(false,105); b.update(true,110); b.update(true,136);
  assert(b.down && !b.tap && !b.longPress);
  b.update(false,170); b.update(false,196); assert(b.tap && !b.down);
  b.update(false,200); assert(!b.tap);
  b.update(true,300); b.update(true,326); b.update(true,1226); assert(b.longPress);
  b.update(true,1400); assert(!b.longPress); b.update(false,1401); b.update(false,1427); assert(!b.tap);
  core::Ring<int,3> r; assert(!r.newest()); r.push(1);r.push(2);r.push(3);r.push(4);
  assert(r.count==3 && *r.newest()==4 && *r.newest(2)==2 && !r.newest(3));
  char name[5]; core::clean(name,sizeof(name),"ab\ncd"); assert(strcmp(name,"ab?c")==0);
  char sentinel='x'; core::clean(&sentinel,0,"abc"); assert(sentinel=='x');
  const uint8_t vector[]={'1','2','3','4','5','6','7','8','9'};
  assert(core::crc16(vector,9)==0x29b1);
  uint8_t frame[20]; core::makeLabFrame(frame,0x12345678,42,60); assert(core::isLabFrame(frame,20));
  assert(!core::isLabFrame(frame,19)); assert(!core::isLabFrame(nullptr,20));
  for(int i=0;i<20;++i) { frame[i]^=1; assert(!core::isLabFrame(frame,20)); frame[i]^=1; }
  char h[5]; core::hex(h,sizeof(h),frame,20); assert(strcmp(h,"4D55")==0);
  for(int i=-100;i<=100;++i) for(int n=1;n<25;++n) { int x=core::wrap(i,n); assert(x>=0 && x<n); }
  puts("PASS: debounce, tap/hold exclusion, millis rollover, ring bounds, sanitization, CRC, lab-frame integrity, hex bounds, cursor wrap");
}
