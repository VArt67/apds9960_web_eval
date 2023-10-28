//-----------------------------------------------------------------------------
// 2023 Ahoy, https://www.mikrocontroller.net/topic/525778
// Creative Commons - http://creativecommons.org/licenses/by-nc-sa/4.0/deed
//-----------------------------------------------------------------------------

#ifndef __DBG_H__
#define __DBG_H__

#ifdef ARDUINO
#include "Arduino.h"
#endif

#if defined(F) && defined(ESP32)
  #undef F
  #define F(sl) (sl)
#endif

#include <functional>
//-----------------------------------------------------------------------------
// available levels
#define ESPHOME_LOG_LEVEL_NONE 0
#define ESPHOME_LOG_LEVEL_ERROR 1
#define ESPHOME_LOG_LEVEL_WARN 2
#define ESPHOME_LOG_LEVEL_INFO 3
#define ESPHOME_LOG_LEVEL_DEBUG 5
#define ESPHOME_LOG_LEVEL_VERBOSE 6

//#define LOG_MAX_MSG_LEN 100

// globally used level
#ifndef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESPHOME_LOG_LEVEL_INFO
#endif

#ifdef NDEBUG
#undef LOG_LOCAL_LEVEL
#define LOG_LOCAL_LEVEL ESPHOME_LOG_LEVEL_NONE
#endif // NDEBUG


#if LOG_LOCAL_LEVEL >= ESPHOME_LOG_LEVEL_ERROR
#define ESP_LOGE(tag, fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__); Serial.println()
#define ESP_LOGE_N(tag, fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define ESP_LOGE(tag, fmt, ...)
#define ESP_LOGE_N(tag, fmt, ...) 
#endif

#if LOG_LOCAL_LEVEL >= ESPHOME_LOG_LEVEL_WARN
#define ESP_LOGW(tag, fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__); Serial.println()
#define ESP_LOGW_N(tag, fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define ESP_LOGW(tag, fmt, ...)
#define ESP_LOGW_N(tag, fmt, ...) 
#endif

#if LOG_LOCAL_LEVEL >= ESPHOME_LOG_LEVEL_INFO
#define ESP_LOGI(tag, fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__); Serial.println()
#define ESP_LOGI_N(tag, fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define ESP_LOGI(tag, fmt, ...)
#define ESP_LOGI_N(tag, fmt, ...) 
#endif

#if LOG_LOCAL_LEVEL >= ESPHOME_LOG_LEVEL_DEBUG
#define ESP_LOGD(tag, fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__); Serial.println()
#define ESP_LOGD_N(tag, fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define ESP_LOGD(tag, fmt, ...)
#define ESP_LOGD_N(tag, fmt, ...) 
#endif

#if LOG_LOCAL_LEVEL >= ESPHOME_LOG_LEVEL_VERBOSE
#define ESP_LOGV(tag, fmt, ...)   Serial.printf(fmt, ##__VA_ARGS__); Serial.println()
#define ESP_LOGV_N(tag, fmt, ...) Serial.printf(fmt, ##__VA_ARGS__)
#else
#define ESP_LOGV(tag, fmt, ...)
#define ESP_LOGV_N(tag, fmt, ...) 
#endif



//-----------------------------------------------------------------------------


#endif /*__DBG_H__*/
