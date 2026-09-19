#pragma once
// 1: existing SportsDeck colour wiring, no CC1101
// 2: colour TFT + CC1101 (RST/BL rewired to 3V3; see README)
// 3: SSD1306 128x64 OLED + optional CC1101
// 4: SH1106 128x64 OLED + optional CC1101
#ifndef MURMUR_PROFILE
#define MURMUR_PROFILE 1
#endif
#ifndef MURMUR_RF_BAND
#define MURMUR_RF_BAND 433
#endif
#if MURMUR_PROFILE < 1 || MURMUR_PROFILE > 4
#error "MURMUR_PROFILE must be 1, 2, 3 or 4"
#endif
#if !defined(CONFIG_IDF_TARGET_ESP32C3)
#error "Select ESP32C3 Dev Module; this pin map is not an S3 pin map."
#endif
#if !defined(ARDUINO_USB_CDC_ON_BOOT) || !ARDUINO_USB_CDC_ON_BOOT
#error "Enable USB CDC On Boot. UART0 pins are used by this hardware profile."
#endif
#define MURMUR_TFT (MURMUR_PROFILE <= 2)
#define MURMUR_RF (MURMUR_PROFILE != 1)
namespace cfg {
constexpr const char* version = "1.0.0";
constexpr int profile = MURMUR_PROFILE;
#if MURMUR_TFT
constexpr int sck=4, mosi=6, cs=7, dc=10;
constexpr int buttons[4] = {0,20,5,21}; // UP/LEFT, DOWN/RIGHT, OK, BACK
#if MURMUR_PROFILE == 1
constexpr int miso=-1, rst=3, bl=1;
#else
constexpr int miso=3, rst=-1, bl=-1, radioCs=2, radioGdo=1;
#endif
constexpr int rotation=1;
constexpr bool inverted=true;
constexpr unsigned long spiHz=20000000UL;
#else
constexpr int sda=3, scl=4;
constexpr int buttons[4]={5,6,7,10}; // K1, K2, K3, K4
constexpr int sck=0, mosi=1, miso=20, radioCs=2, radioGdo=21;
#endif
#if MURMUR_RF_BAND == 433
constexpr float center=433.92f, bandLow=433.05f, bandHigh=434.79f;
#elif MURMUR_RF_BAND == 315
constexpr float center=315.0f, bandLow=314.5f, bandHigh=315.5f;
#elif MURMUR_RF_BAND == 868
constexpr float center=868.3f, bandLow=863.0f, bandHigh=870.0f;
#elif MURMUR_RF_BAND == 915
constexpr float center=915.0f, bandLow=915.0f, bandHigh=928.0f;
#else
#error "Supported RF band profiles: 315, 433, 868, 915"
#endif
constexpr unsigned scanMs=3500, wifiTimeoutMs=15000, bins=48;
constexpr unsigned maxWifi=24, maxBle=24, maxPacket=48;
constexpr unsigned debounceMs=25, holdMs=900;
constexpr int8_t labDbm=-20;
} // namespace cfg
