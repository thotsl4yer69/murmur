#pragma once
#include <stdint.h>
#include <stddef.h>
#include <string.h>
namespace core {
inline bool elapsed(uint32_t now,uint32_t since,uint32_t duration) {
  return uint32_t(now-since)>=duration;
}
inline int wrap(int index,int count) {
  if(count<=0) return 0;
  index%=count; return index<0 ? index+count:index;
}
struct Button {
  bool raw=false,down=false,held=false,tap=false,longPress=false;
  uint32_t changeAt=0,downAt=0;
  void update(bool pressed,uint32_t now,uint32_t debounce=25,uint32_t hold=900) {
    tap=false; longPress=false;
    if(pressed!=raw) { raw=pressed; changeAt=now; }
    if(raw!=down && elapsed(now,changeAt,debounce)) {
      down=raw;
      if(down) { downAt=now; held=false; }
      else { tap=!held; }
    }
    if(down && !held && elapsed(now,downAt,hold)) { held=true; longPress=true; }
  }
};
template<class T,size_t N> struct Ring {
  static_assert(N>0,"Ring must have storage");
  T data[N]{}; size_t head=0,count=0;
  void push(const T& value) { data[head]=value; head=(head+1)%N; if(count<N) ++count; }
  const T* newest(size_t age=0) const { return age<count ? &data[(head+N-1-age)%N]:nullptr; }
};
inline void clean(char* dst,size_t cap,const char* src) {
  if(!cap) return;
  size_t n=0;
  while(src && *src && n+1<cap) { const unsigned char c=static_cast<unsigned char>(*src++); dst[n++]=(c>=32 && c<=126)?char(c):'?'; }
  dst[n]=0;
}
inline uint16_t crc16(const uint8_t* data,size_t length) {
  uint16_t crc=0xffff;
  for(size_t i=0;i<length;++i) { crc^=uint16_t(data[i])<<8; for(int b=0;b<8;++b) crc=(crc&0x8000)?uint16_t((crc<<1)^0x1021):uint16_t(crc<<1); }
  return crc;
}
inline void put32(uint8_t* p,uint32_t n) { for(int i=0;i<4;++i) p[i]=uint8_t(n>>(8*i)); }
inline void makeLabFrame(uint8_t (&out)[20],uint32_t node,uint32_t seq,uint32_t seconds) {
  memcpy(out,"MUR3",4); out[4]=1; out[5]=1;
  put32(out+6,node); put32(out+10,seq); put32(out+14,seconds);
  const uint16_t c=crc16(out,18); out[18]=uint8_t(c>>8); out[19]=uint8_t(c);
}
inline bool isLabFrame(const uint8_t* data,size_t len) {
  return data && len==20 && memcmp(data,"MUR3",4)==0 && data[4]==1 && data[5]==1 && crc16(data,18)==uint16_t((uint16_t(data[18])<<8)|data[19]);
}
inline void hex(char* dst,size_t cap,const uint8_t* bytes,size_t len) {
  if(!cap) return;
  const char* digits="0123456789ABCDEF"; size_t n=0;
  for(size_t i=0;i<len && n+2<cap;++i) { dst[n++]=digits[bytes[i]>>4]; dst[n++]=digits[bytes[i]&15]; }
  dst[n]=0;
}
} // namespace core
