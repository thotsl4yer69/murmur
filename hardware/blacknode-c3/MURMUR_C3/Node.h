#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <WiFi.h>
#include <Preferences.h>
#include <NimBLEDevice.h>
#include <esp_wifi.h>
#include <esp_system.h>
#include <math.h>
#include <algorithm>
#include "Config.h"
#include "Core.h"
#if MURMUR_TFT
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#else
#include <Wire.h>
#include <U8g2lib.h>
#endif
#if MURMUR_RF
#include <RadioLib.h>
#endif

namespace node {
#if MURMUR_TFT
Adafruit_ST7789 display(&SPI,cfg::cs,cfg::dc,cfg::rst);
constexpr int visibleRows=6;
#else
#if MURMUR_PROFILE == 3
U8G2_SSD1306_128X64_NONAME_F_HW_I2C display(U8G2_R0,U8X8_PIN_NONE,cfg::scl,cfg::sda);
#else
U8G2_SH1106_128X64_NONAME_F_HW_I2C display(U8G2_R0,U8X8_PIN_NONE,cfg::scl,cfg::sda);
#endif
constexpr int visibleRows=4;
#endif
#if MURMUR_RF
CC1101 radio = new Module(cfg::radioCs,cfg::radioGdo,RADIOLIB_NC,RADIOLIB_NC,SPI);
#endif
Preferences prefs;
core::Button keys[4];
enum class Page : uint8_t { Home, Wifi, Ble, Channels, Scope, Rx, Lab, Events, System, Keys };
enum class Job : uint8_t { Idle, Wifi, Ble, Scope, Rx, Tx, Gap };
Page page=Page::Home;
Job job=Job::Idle;
const char* menu[]={"WI-FI SURVEY","BLE SURVEY","WI-FI CHANNELS","SUB-GHz SCOPE","PACKET RX","LAB LINK TEST","EVENT LOG","SYSTEM"};
constexpr int menuCount=sizeof(menu)/sizeof(menu[0]);
int selected=0,cursor=0,systemItem=0,labMode=0;
uint32_t bootId=0,jobAt=0,drawAt=0,wifiAt=0,bleAt=0,txSequence=0;
uint8_t brightness=180;
bool flipped=false,recordEvents=true,bleUp=false,radioReady=false,radioChecked=false,peakView=false,prefsReady=false;
String status="IDLE";
struct WifiRow { char name[33]{},address[18]{}; int16_t rssi=0; uint8_t channel=0,auth=0; };
struct BleRow { char name[25]{},address[18]{}; int16_t rssi=0; };
WifiRow wifiRows[cfg::maxWifi];
BleRow bleRows[cfg::maxBle];
int wifiCount=0,wifiTotal=-1,bleCount=0;
bool bleHasResult=false;
uint16_t channels[14]{};
NimBLEScan* bleScanner=nullptr;
struct Event { uint32_t at=0; char text[65]{}; };
struct Packet { uint32_t at=0; uint8_t len=0; int16_t rssi=0; uint8_t bytes[cfg::maxPacket]{}; };
core::Ring<Event,32> events;
core::Ring<Packet,8> packets;
uint8_t lastLab[20]{},txData[20]{};
bool haveLab=false;
volatile bool radioIrq=false;
float levels[cfg::bins],peaks[cfg::bins];
bool valid[cfg::bins]{};
unsigned bin=0,txRemaining=0,txSent=0;
bool waitingRssi=false;
uint32_t rfAt=0;
int16_t radioError=0;
String previousTitle,previousMeta,previousFooter,previousRows[visibleRows];
bool previousHighlight[visibleRows]{};
int previousPage=-1;

void logEvent(const String& message) {
  if(recordEvents) { Event e; e.at=millis(); core::clean(e.text,sizeof(e.text),message.c_str()); events.push(e); }
  if(Serial) { Serial.print("# "); Serial.print(millis()); Serial.print(' '); Serial.println(message); }
}
const char* jobName() {
  switch(job) { case Job::Wifi:return "WIFI RX"; case Job::Ble:return "BLE RX"; case Job::Scope:return "RF SWEEP"; case Job::Rx:return "PACKET RX"; case Job::Tx:return "LAB TX"; case Job::Gap:return "TX GAP"; default:return "IDLE"; }
}
void setLight() {
#if MURMUR_TFT
  if(cfg::bl>=0) ledcWrite(cfg::bl,brightness);
#else
  display.setContrast(brightness);
#endif
}
void saveSettings() {
  if(!prefsReady) return;
  prefs.putUChar("light",brightness); prefs.putBool("flip",flipped); prefs.putBool("events",recordEvents);
}
void resetView() {
  previousTitle="\1"; previousMeta="\1"; previousFooter="\1";
  for(int i=0;i<visibleRows;++i) previousRows[i]="\1";
#if MURMUR_TFT
  display.fillScreen(0x0000);
#endif
}
String clipped(const String& s,unsigned n) { return s.length()>n ? s.substring(0,n-1)+"~":s; }
void uiHeader(const String& title,const String& meta) {
#if MURMUR_TFT
  if(previousTitle!=title) { display.fillRect(0,0,280,30,0x0000); display.setTextColor(0xFFFF); display.setTextSize(2); display.setCursor(12,8); display.print(clipped(title,21)); previousTitle=title; }
  if(previousMeta!=meta) { display.fillRect(0,31,280,15,0x0000); display.setTextColor(0xAD55); display.setTextSize(1); display.setCursor(12,33); display.print(clipped(meta,42)); previousMeta=meta; }
  display.drawFastHLine(12,45,256,0x4208);
#else
  display.clearBuffer(); display.setFont(u8g2_font_5x7_tf); display.drawStr(0,7,clipped(((millis()/3000)%2)?meta:title,25).c_str()); display.drawHLine(0,9,128);
#endif
}
void uiRow(int row,const String& text,bool highlight=false) {
  if(row<0 || row>=visibleRows) return;
#if MURMUR_TFT
  if(previousRows[row]==text && previousHighlight[row]==highlight) return;
  const uint16_t bg=highlight?0xEF5D:0x0000,fg=highlight?0x0000:0xFFFF;
  display.fillRect(10,50+row*25,260,24,bg); display.setTextSize(2); display.setTextColor(fg,bg);
  display.setCursor(14,54+row*25); display.print(clipped(text,21));
  previousRows[row]=text; previousHighlight[row]=highlight;
#else
  const int y=13+row*10; display.setFont(u8g2_font_6x10_tf);
  if(highlight) { display.setDrawColor(1); display.drawBox(0,y-1,128,10); display.setDrawColor(0); }
  display.drawStr(1,y+7,clipped(text,21).c_str()); display.setDrawColor(1);
#endif
}
void uiFooter(const String& footer) {
#if MURMUR_TFT
  if(previousFooter!=footer) { display.fillRect(0,212,280,28,0x0000); display.drawFastHLine(12,212,256,0x4208); display.setTextSize(1); display.setTextColor(0xAD55); display.setCursor(12,222); display.print(clipped(footer,42)); previousFooter=footer; }
#else
  display.setFont(u8g2_font_5x7_tf); display.drawHLine(0,54,128); display.drawStr(0,63,clipped(footer,25).c_str()); display.sendBuffer();
#endif
}
void uiGraph() {
#if MURMUR_TFT
  constexpr int x=15,y=132,w=249,h=64;
  display.fillRect(x,y,w,h,0); display.drawRect(x,y,w,h,0x4208);
#else
  constexpr int x=0,y=35,w=127,h=18;
  display.setDrawColor(0); display.drawBox(x,y,w,h); display.setDrawColor(1); display.drawFrame(x,y,w,h);
#endif
  for(unsigned i=0;i<cfg::bins;++i) if(valid[i]) {
    const float value=peakView?peaks[i]:levels[i];
    int a=int((value+120.0f)*float(h-3)/100.0f); a=std::max(1,std::min(h-3,a));
    int px=x+1+int(i*(w-2)/cfg::bins);
#if MURMUR_TFT
    display.drawFastVLine(px,y+h-1-a,a,0xD6B9);
#else
    display.drawVLine(px,y+h-1-a,a);
#endif
  }
}
void IRAM_ATTR onRadio() { radioIrq=true; }
bool takeRadioIrq() { noInterrupts(); bool hit=radioIrq; radioIrq=false; interrupts(); return hit; }
void stopJobs(bool announce=false) {
  const bool active=job!=Job::Idle;
  if(job==Job::Wifi) esp_wifi_scan_stop();
  WiFi.scanDelete(); WiFi.mode(WIFI_OFF);
  if(bleUp) { if(bleScanner) bleScanner->stop(); NimBLEDevice::deinit(true); bleScanner=nullptr; bleUp=false; }
#if MURMUR_RF
  if(radioReady) { radio.clearPacketReceivedAction(); radio.clearPacketSentAction(); radio.standby(); }
#endif
  takeRadioIrq(); job=Job::Idle; txRemaining=0; status="IDLE";
  if(announce && active) logEvent("All radio jobs stopped");
}
void fault(const String& message,int code=0) {
  stopJobs(); status=message; if(code) status+=" "+String(code); logEvent(status);
}
bool configureRadio() {
#if MURMUR_RF
  radioChecked=true;
  radioError=radio.begin(cfg::center,4.8f,5.0f,58.0f,cfg::labDbm,16);
  if(radioError==RADIOLIB_ERR_NONE) radioError=radio.setSyncWord(uint8_t(0x4D),uint8_t(0x52));
  if(radioError==RADIOLIB_ERR_NONE) radioError=radio.variablePacketLengthMode(cfg::maxPacket);
  if(radioError==RADIOLIB_ERR_NONE) radioError=radio.setCrcFiltering(true);
  radioReady=radioError==RADIOLIB_ERR_NONE;
  if(!radioReady) { fault("CC1101 init error",radioError); return false; }
  return true;
#else
  fault("Use RF wiring profile"); return false;
#endif
}
void startWifi() {
  stopJobs(); wifiCount=0; wifiTotal=-1; memset(channels,0,sizeof(channels));
  WiFi.persistent(false); WiFi.setAutoReconnect(false);
  if(!WiFi.mode(WIFI_STA)) { fault("Wi-Fi start failed"); return; }
  WiFi.disconnect(false,false);
  // Receive-only Australian channel plan: no AP, associations or probe requests.
  wifi_country_t country{}; memcpy(country.cc,"AU",3); country.schan=1; country.nchan=13; country.policy=WIFI_COUNTRY_POLICY_MANUAL;
  if(esp_wifi_set_country(&country)!=ESP_OK) { fault("Wi-Fi country error"); return; }
  const int result=WiFi.scanNetworks(true,true,true,150,0);
  if(result==WIFI_SCAN_FAILED) { fault("Wi-Fi scan failed"); return; }
  job=Job::Wifi; jobAt=millis(); cursor=0; status="Passive scan...";
}
void pollWifi() {
  int n=WiFi.scanComplete();
  if(n==WIFI_SCAN_RUNNING) { if(core::elapsed(millis(),jobAt,cfg::wifiTimeoutMs)) fault("Wi-Fi scan timeout"); return; }
  if(n<0) { fault("Wi-Fi scan failed",n); return; }
  wifiTotal=n;
  for(int i=0;i<n;++i) {
    WifiRow row; core::clean(row.name,sizeof(row.name),WiFi.SSID(i).c_str());
    if(!row.name[0]) core::clean(row.name,sizeof(row.name),"<hidden>");
    core::clean(row.address,sizeof(row.address),WiFi.BSSIDstr(i).c_str());
    row.rssi=WiFi.RSSI(i); row.channel=WiFi.channel(i); row.auth=uint8_t(WiFi.encryptionType(i));
    if(row.channel>=1 && row.channel<=13) ++channels[row.channel];
    int at=wifiCount;
    if(at<int(cfg::maxWifi)) ++wifiCount; else if(row.rssi>wifiRows[at-1].rssi) --at; else continue;
    while(at>0 && row.rssi>wifiRows[at-1].rssi) { if(at<int(cfg::maxWifi)) wifiRows[at]=wifiRows[at-1]; --at; }
    wifiRows[at]=row;
  }
  WiFi.scanDelete(); WiFi.mode(WIFI_OFF); job=Job::Idle; wifiAt=millis();
  status=String(wifiTotal)+" APs / strongest "+String(wifiCount); logEvent(status);
}
void startBle() {
  stopJobs(); bleCount=0; bleHasResult=false; cursor=0;
  NimBLEDevice::init(""); bleUp=true; bleScanner=NimBLEDevice::getScan();
  if(!bleScanner) { fault("BLE allocation failed"); return; }
  bleScanner->setActiveScan(false); bleScanner->setInterval(100); bleScanner->setWindow(80);
  bleScanner->setDuplicateFilter(true); bleScanner->setMaxResults(cfg::maxBle);
  if(!bleScanner->start(cfg::scanMs,false,true)) { fault("BLE scan failed"); return; }
  job=Job::Ble; jobAt=millis(); status="Passive BLE scan...";
}
void pollBle() {
  if(!bleScanner) { fault("BLE scanner missing"); return; }
  if(bleScanner->isScanning()) { if(core::elapsed(millis(),jobAt,cfg::scanMs+5000)) fault("BLE scan timeout"); return; }
  { // Results contain library-owned pointers: copy before clear/deinit.
    NimBLEScanResults results=bleScanner->getResults();
    bleCount=std::min(results.getCount(),int(cfg::maxBle));
    for(int i=0;i<bleCount;++i) {
      const NimBLEAdvertisedDevice* d=results.getDevice(i);
      if(!d) { bleRows[i]=BleRow{}; continue; }
      core::clean(bleRows[i].name,sizeof(bleRows[i].name),d->haveName()?d->getName().c_str():"<unnamed>");
      core::clean(bleRows[i].address,sizeof(bleRows[i].address),d->getAddress().toString().c_str());
      bleRows[i].rssi=d->getRSSI();
    }
  }
  std::sort(bleRows,bleRows+bleCount,[](const BleRow& a,const BleRow& b){ return a.rssi>b.rssi; });
  stopJobs(); bleAt=millis(); bleHasResult=true;
  status=String(bleCount)+" BLE results"+(bleCount==int(cfg::maxBle)?" (cap)":""); logEvent(status);
}
void startScope() {
  stopJobs(); if(!configureRadio()) return;
#if MURMUR_RF
  for(unsigned i=0;i<cfg::bins;++i) { levels[i]=-127; peaks[i]=-127; valid[i]=false; }
  radio.clearPacketReceivedAction(); bin=0; waitingRssi=false; rfAt=millis(); job=Job::Scope; status="RSSI sweep / uncalibrated";
#endif
}
void startRx() {
  stopJobs(); if(!configureRadio()) return;
#if MURMUR_RF
  radio.setPacketReceivedAction(onRadio); takeRadioIrq();
  int16_t err=radio.startReceive(); if(err) { fault("RF RX error",err); return; }
  job=Job::Rx; status="FSK 4.8k / sync 4D52";
#endif
}
void pollScope() {
#if MURMUR_RF
  if(!core::elapsed(millis(),rfAt,8)) return;
  rfAt=millis();
  if(!waitingRssi) {
    int16_t err=radio.setFrequency(cfg::bandLow+(cfg::bandHigh-cfg::bandLow)*float(bin)/float(cfg::bins-1));
    if(!err) err=radio.receiveDirect();
    if(err) { fault("RF sweep error",err); return; } waitingRssi=true;
  } else {
    const float rssi=radio.getRSSI();
    if(isfinite(rssi)) { levels[bin]=rssi; peaks[bin]=valid[bin]?std::max(peaks[bin],rssi):rssi; valid[bin]=true; }
    int16_t err=radio.standby(); if(err) { fault("RF standby error",err); return; }
    bin=(bin+1)%cfg::bins; waitingRssi=false;
  }
#endif
}
void pollRx() {
#if MURMUR_RF
  if(!takeRadioIrq()) return;
  const size_t len=radio.getPacketLength();
  if(len>0 && len<=cfg::maxPacket) {
    Packet packet; packet.len=uint8_t(len); packet.at=millis();
    int16_t err=radio.readData(packet.bytes,len);
    if(!err) {
      packet.rssi=int16_t(radio.getRSSI()); packets.push(packet);
      if(core::isLabFrame(packet.bytes,len)) { memcpy(lastLab,packet.bytes,20); haveLab=true; }
      logEvent("RF RX "+String(len)+" bytes / "+String(packet.rssi)+" dBm");
    } else logEvent("RF read error "+String(err));
  } else { radio.standby(); logEvent("RF length rejected "+String(unsigned(len))); }
  int16_t err=radio.startReceive(); if(err) fault("RF restart error",err);
#endif
}
void beginTxPacket() {
#if MURMUR_RF
  if(labMode==0) core::makeLabFrame(txData,bootId,++txSequence,millis()/1000);
  takeRadioIrq(); int16_t err=radio.startTransmit(txData,sizeof(txData));
  if(err) { fault("LAB TX error",err); return; }
  job=Job::Tx; rfAt=millis(); status="LAB TX "+String(txSent+1);
#endif
}
void startLab() {
  if(job==Job::Tx || job==Job::Gap) return;
  if(labMode==1 && !haveLab) { status="Receive a MUR3 frame first"; return; }
  stopJobs(); if(!configureRadio()) return;
#if MURMUR_RF
  if(labMode==1) memcpy(txData,lastLab,20);
  radio.clearPacketReceivedAction(); radio.setPacketSentAction(onRadio);
  txRemaining=labMode==0?3:1; txSent=0; logEvent(labMode==0?"LAB: 3 test frames":"LAB: one same-profile frame replay"); beginTxPacket();
#endif
}
void pollTx() {
#if MURMUR_RF
  if(job==Job::Gap) { if(core::elapsed(millis(),rfAt,1000)) beginTxPacket(); return; }
  const bool done=takeRadioIrq();
  if(!done && !core::elapsed(millis(),rfAt,250)) return;
  int16_t err=radio.finishTransmit();
  if(!done || err) { fault(!done?"LAB TX timeout":"LAB finish error",err); return; }
  ++txSent; if(txRemaining) --txRemaining;
  if(!txRemaining) { stopJobs(); status="LAB complete: "+String(txSent); logEvent(status); }
  else { job=Job::Gap; rfAt=millis(); status="LAB gap / BACK stops"; }
#endif
}
void enter(Page next) {
  stopJobs(); page=next; cursor=0; status="IDLE";
  switch(page) { case Page::Wifi: case Page::Channels:startWifi();break; case Page::Ble:startBle();break; case Page::Scope:startScope();break; case Page::Rx:startRx();break; default:break; }
}
void handleKeys() {
  for(int i=0;i<4;++i) keys[i].update(digitalRead(cfg::buttons[i])==LOW,millis());
  if(keys[3].longPress) { stopJobs(true); page=Page::Home; status="All jobs stopped"; return; }
  if(page==Page::Keys) return;
  if(keys[3].tap) { stopJobs(true); page=Page::Home; return; }
  int direction=(keys[1].tap?1:0)-(keys[0].tap?1:0);
  if(page==Page::Home) {
    selected=core::wrap(selected+direction,menuCount);
    if(keys[2].tap) enter(static_cast<Page>(selected+1));
    return;
  }
  switch(page) {
    case Page::Wifi: cursor=core::wrap(cursor+direction,wifiCount); if(keys[2].tap) startWifi(); break;
    case Page::Ble: cursor=core::wrap(cursor+direction,bleCount); if(keys[2].tap) startBle(); break;
    case Page::Channels: cursor=core::wrap(cursor+direction,13); if(keys[2].tap) startWifi(); break;
    case Page::Scope:
      if(direction) peakView=!peakView;
      if(keys[2].tap) { if(job==Job::Scope) { stopJobs(); status="Sweep paused"; } else startScope(); } break;
    case Page::Rx:
      cursor=core::wrap(cursor+direction,int(packets.count));
      if(keys[2].tap) { if(job==Job::Rx) { stopJobs(); status="RX paused"; } else startRx(); } break;
    case Page::Lab:
      if(job==Job::Idle && direction) labMode=core::wrap(labMode+direction,2);
      if(keys[2].longPress) startLab(); else if(keys[2].tap) status="Hold OK 0.9s to send"; break;
    case Page::Events: cursor=core::wrap(cursor+direction,int(events.count)); break;
    case Page::System:
      systemItem=core::wrap(systemItem+direction,4);
      if(keys[2].tap) {
        if(systemItem==0) {
#if MURMUR_PROFILE != 2
          brightness=brightness<70?128:brightness<150?210:brightness<230?255:40;
#endif
        } else if(systemItem==1) {
          flipped=!flipped;
#if MURMUR_TFT
          display.setRotation(flipped?3:cfg::rotation);
#else
          display.setFlipMode(flipped?1:0);
#endif
          resetView();
        } else if(systemItem==2) recordEvents=!recordEvents;
        else { page=Page::Keys; break; }
        setLight(); saveSettings();
      } break;
    default: break;
  }
}
void csvText(const char* s) {
  Serial.print('"');
  if(s && (*s=='=' || *s=='+' || *s=='-' || *s=='@')) Serial.print('\'');
  for(;s && *s;++s) { if(*s=='"') Serial.print('"'); Serial.print(*s); }
  Serial.print('"');
}
void exportCsv(const String& what) {
  if(what=="wifi") {
    Serial.println("boot_id,scan_uptime_ms,ssid,bssid,channel,rssi_dbm,auth_enum");
    for(int i=0;i<wifiCount;++i) { const auto& r=wifiRows[i]; Serial.printf("%08lX,%lu,",(unsigned long)bootId,(unsigned long)wifiAt);csvText(r.name);Serial.print(',');csvText(r.address);Serial.printf(",%u,%d,%u\n",r.channel,r.rssi,r.auth); }
  } else if(what=="ble") {
    Serial.println("boot_id,scan_uptime_ms,name,address,rssi_dbm");
    for(int i=0;i<bleCount;++i) { const auto& r=bleRows[i]; Serial.printf("%08lX,%lu,",(unsigned long)bootId,(unsigned long)bleAt);csvText(r.name);Serial.print(',');csvText(r.address);Serial.printf(",%d\n",r.rssi); }
  } else if(what=="packets") {
    Serial.println("boot_id,uptime_ms,mhz,modulation,kbps,sync,length,rssi_dbm,hex");
    for(size_t i=packets.count;i>0;--i) { const auto* p=packets.newest(i-1); char hex[2*cfg::maxPacket+1]; core::hex(hex,sizeof(hex),p->bytes,p->len);Serial.printf("%08lX,%lu,%.3f,2FSK,4.8,4D52,%u,%d,%s\n",(unsigned long)bootId,(unsigned long)p->at,cfg::center,p->len,p->rssi,hex); }
  } else if(what=="events") {
    Serial.println("boot_id,uptime_ms,event");
    for(size_t i=events.count;i>0;--i) { const auto* e=events.newest(i-1);Serial.printf("%08lX,%lu,",(unsigned long)bootId,(unsigned long)e->at);csvText(e->text);Serial.println(); }
  } else Serial.println("# export wifi | ble | packets | events");
}
void handleSerial() {
  static char line[64]; static size_t len=0; static bool overflow=false;
  unsigned budget=128;
  while(Serial.available() && budget--) {
    char c=char(Serial.read()); if(c=='\r') continue;
    if(c!='\n') { if(len+1<sizeof(line) && !overflow) line[len++]=c; else overflow=true; continue; }
    if(overflow) { Serial.println("# command too long"); len=0; overflow=false; continue; }
    line[len]=0; String command(line); command.trim(); len=0;
    if(command=="wifi") enter(Page::Wifi);
    else if(command=="ble") enter(Page::Ble);
    else if(command=="scope") enter(Page::Scope);
    else if(command=="rx") enter(Page::Rx);
    else if(command=="stop") { stopJobs(true); page=Page::Home; }
    else if(command=="status") Serial.printf("# BLACKNODE %s profile=%d boot=%08lX job=%s heap=%u minheap=%u uptime_ms=%lu reset=%d\n",cfg::version,cfg::profile,(unsigned long)bootId,jobName(),ESP.getFreeHeap(),ESP.getMinFreeHeap(),(unsigned long)millis(),int(esp_reset_reason()));
    else if(command.startsWith("export ")) { stopJobs(); exportCsv(command.substring(7)); }
    else if(command.length()) Serial.println("# help | status | wifi | ble | scope | rx | stop | export wifi/ble/packets/events");
  }
}
void render() {
  if(previousPage!=int(page)) { resetView(); previousPage=int(page); }
  String rows[visibleRows]; bool highlight[visibleRows]{};
  String title="MURMUR // C3",meta=status,footer="UP DOWN OK BACK / hold BACK stops";
  if(page==Page::Home) {
    meta=String("BLACKNODE ")+cfg::version+" / "+jobName();
    int first=std::max(0,selected-visibleRows+1);
    for(int i=0;i<visibleRows && first+i<menuCount;++i) { rows[i]=menu[first+i]; highlight[i]=first+i==selected; }
  } else if(page==Page::Wifi || page==Page::Ble) {
    bool wifi=page==Page::Wifi; int count=wifi?wifiCount:bleCount;
    title=wifi?"WI-FI SURVEY":"BLE SURVEY";
    if(job==Job::Idle && (wifi?wifiTotal>=0:bleHasResult)) meta=String(count)+(wifi?" listed / ":" listed (cap 24) / ")+String((millis()-(wifi?wifiAt:bleAt))/1000)+"s old";
    int first=std::max(0,cursor-visibleRows+1);
    for(int i=0;i<visibleRows && first+i<count;++i) {
      int n=first+i; String name=wifi?wifiRows[n].name:bleRows[n].name; int rssi=wifi?wifiRows[n].rssi:bleRows[n].rssi;
      rows[i]=clipped(name,15)+" "+String(rssi); highlight[i]=n==cursor;
    }
    if(!count) rows[0]=(job==Job::Idle)?"No results":"Scanning...";
    footer="OK rescan / BACK exit / dBm";
  } else if(page==Page::Channels) {
    title="WI-FI CHANNELS"; meta=wifiTotal<0?status:"AP count, not RF power";
    int first=std::max(0,cursor-visibleRows+1);
    for(int i=0;i<visibleRows && first+i<13;++i) { int ch=first+i+1; rows[i]="CH "+String(ch)+(wifiTotal<0?"  --":"  APs "+String(channels[ch]));highlight[i]=ch==cursor+1; }
    footer="UP/DOWN channel / OK rescan";
  } else if(page==Page::Scope) {
    title="SUB-GHz SCOPE";
    rows[0]=String(cfg::bandLow,2)+" - "+String(cfg::bandHigh,2);
    float maximum=-999; unsigned at=0;
    for(unsigned i=0;i<cfg::bins;++i) if(valid[i]) { float v=peakView?peaks[i]:levels[i]; if(v>maximum) { maximum=v;at=i; } }
    rows[1]=maximum>-999?String(maximum,0)+"dBm @"+String(cfg::bandLow+(cfg::bandHigh-cfg::bandLow)*at/(cfg::bins-1),2):"No samples yet";
#if MURMUR_TFT
    rows[2]=peakView?"PEAK HOLD":"LIVE / relative RSSI";
#endif
    footer="OK pause/run / UP/DOWN live/peak";
  } else if(page==Page::Rx) {
    title="PACKET RX";
    const Packet* p=packets.newest(cursor);
    rows[0]=String(cfg::center,2)+" MHz / 2-FSK";
    if(p) {
      rows[1]=String(p->len)+" bytes / "+String(p->rssi)+"dBm";
      char hex[2*cfg::maxPacket+1]; core::hex(hex,sizeof(hex),p->bytes,p->len); String bytes(hex);
      for(int i=2;i<visibleRows;++i) rows[i]=bytes.substring((i-2)*20,(i-1)*20);
    } else { rows[1]="No matching packets"; rows[2]="4.8k / sync 4D52"; }
    footer="OK pause/run / UP/DOWN history";
  } else if(page==Page::Lab) {
    title="LAB LINK TEST";
    rows[0]=labMode==0?"3 MUR3 test frames":"Replay last MUR3 x1"; highlight[0]=true;
    rows[1]=String(cfg::center,2)+" MHz / -20dBm";
    rows[2]=labMode==1?(haveLab?"MUR3 frame available":"No MUR3 frame stored"):"20 bytes / 1s gaps";
    rows[3]="Hold OK to send";
#if MURMUR_TFT
    rows[4]="BACK stops sequence"; rows[5]="2-FSK / CRC checked";
#endif
    footer="UP/DOWN mode / HOLD OK 0.9s";
  } else if(page==Page::Events) {
    title="EVENT LOG"; meta=String(events.count)+" RAM entries / reset clears";
    for(int i=0;i<visibleRows;++i) { const Event* e=events.newest(cursor+i);if(e) rows[i]=String(e->at/1000)+"s "+String(e->text); }
    if(!events.count) rows[0]="No events recorded";
    footer="UP/DOWN scroll / USB export events";
  } else if(page==Page::System) {
    title="SYSTEM"; meta="Heap "+String(ESP.getFreeHeap()/1024)+"K / uptime "+String(millis()/1000)+"s";
#if MURMUR_PROFILE == 2
    rows[0]="Backlight hard-wired";
#else
    rows[0]=String(MURMUR_TFT?"Backlight ":"Contrast ")+String(brightness);
#endif
    rows[1]=String("Flip ")+(flipped?"ON":"OFF"); rows[2]=String("RAM logging ")+(recordEvents?"ON":"OFF"); rows[3]="Screen + button test"; highlight[systemItem]=true;
#if MURMUR_TFT
    rows[4]=String("Profile ")+cfg::profile+" / "+(MURMUR_RF?(radioReady?"RF found":radioChecked?"RF no response":"RF untested"):"no RF pins");
    rows[5]="Min heap "+String(ESP.getMinFreeHeap()/1024)+"K";
#endif
    footer="UP/DOWN setting / OK change";
  } else if(page==Page::Keys) {
    title="HARDWARE TEST"; meta="Press each key / hold BACK exits";
    const char* names[]={"UP/LEFT","DOWN/RIGHT","OK","BACK"};
    for(int i=0;i<4;++i) { rows[i]=String(names[i])+" G"+String(cfg::buttons[i])+" "+(keys[i].down?"DOWN":"up");highlight[i]=keys[i].down; }
#if MURMUR_TFT
    rows[4]="RED GREEN BLUE below";
#endif
    footer="No key action here / HOLD BACK";
  }
  uiHeader(title,meta); for(int i=0;i<visibleRows;++i) uiRow(i,rows[i],highlight[i]);
  if(page==Page::Scope) uiGraph();
#if MURMUR_TFT
  if(page==Page::Keys) { display.fillRect(14,182,80,18,0xF800); display.fillRect(98,182,80,18,0x07E0);display.fillRect(182,182,80,18,0x001F); }
#endif
  uiFooter(footer);
}
void begin() {
  Serial.begin(115200);
  bootId=esp_random();
  for(int pin:cfg::buttons) pinMode(pin,INPUT_PULLUP);
  prefsReady=prefs.begin("murmur-c3",false);
  if(prefsReady) { brightness=prefs.getUChar("light",180); if(brightness<20) brightness=40; flipped=prefs.getBool("flip",false); recordEvents=prefs.getBool("events",true); }
#if MURMUR_RF
  pinMode(cfg::radioCs,OUTPUT); digitalWrite(cfg::radioCs,HIGH); pinMode(cfg::radioGdo,INPUT);
#endif
#if MURMUR_TFT
  pinMode(cfg::cs,OUTPUT); digitalWrite(cfg::cs,HIGH);
  SPI.begin(cfg::sck,cfg::miso,cfg::mosi,cfg::cs);
  if(cfg::bl>=0) { pinMode(cfg::bl,OUTPUT); digitalWrite(cfg::bl,HIGH); ledcAttach(cfg::bl,5000,8); }
  display.init(240,280); display.setSPISpeed(cfg::spiHz); display.setRotation(flipped?3:cfg::rotation); display.invertDisplay(cfg::inverted);display.setTextWrap(false);
#else
  SPI.begin(cfg::sck,cfg::miso,cfg::mosi,cfg::radioCs);
  display.setBusClock(400000); display.begin(); display.setFlipMode(flipped?1:0);
#endif
  setLight(); resetView(); WiFi.mode(WIFI_OFF);
  for(unsigned i=0;i<cfg::bins;++i) levels[i]=peaks[i]=-127;
  logEvent(String("Boot BLACKNODE ")+cfg::version+" profile "+cfg::profile+" reset "+int(esp_reset_reason())); render();
}
void tick() {
  handleKeys(); handleSerial();
  switch(job) { case Job::Wifi:pollWifi();break; case Job::Ble:pollBle();break; case Job::Scope:pollScope();break; case Job::Rx:pollRx();break; case Job::Tx:case Job::Gap:pollTx();break; default:break; }
  if(core::elapsed(millis(),drawAt,page==Page::Keys?80:200)) { drawAt=millis(); render(); }
  delay(2);
}
} // namespace node
