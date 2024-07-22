#ifndef pfodESP32Utils_h
#define pfodESP32Utils_h

#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <WiFi.h>
//#include <esp_wifi_types.h>

class pfodESP32Utils {
  public:
  	// some constants for field sizes
  static const int MAX_LEN_SSID = 32;
  static const int MAX_LEN_PASSWORD = 64;
  static const int MAX_LEN_STATICIP = 40;
  static void analogWrite(int _ch, int count);
  static size_t strncpy_safe(char* dest, const char* src, size_t maxLen);
  static void urldecode2(char *dst, const char *src);
  static uint8_t* parseLong(uint8_t* idxPtr, long *result);
  static uint8_t isEmpty(const char* str); // Look for non-control char i.e. char > space (32)
  /**
  * trim chars <=32 from beginning and end of string
  * return zero if one found
  */
  static char* trim(char* str);

  static uint32_t ipStrToNum(const char* ipStr);
  static const char* encryptionTypeToStr(wifi_auth_mode_t type);
  static const char* scanForStrongestAP(char* result, size_t resultLen);
  
  static const char WEP[];
  static const char WPA[];
  static const char WPA2[];
  static const char WPA_WPA2_PSK[];
  static const char NONE[];
  static const char WPA2_ENTERPRISE[];
  static const char UNKNOWN_ENCRY[];

};

#endif // pfodESP32Utils_h