#pragma once

// The application uses the hardware profile selected in config.h.

#include <Arduino.h>
#include <ArduinoJson.h>
#include <OneWire.h>
#include <DallasTemperature.h>


#include "config.h"
#include <ei_ds18b20.h>
#include <ei_scheduler.h>
#include <ei_utilities.h>

#pragma message( \
  "\n************************************************************\n" \
  "***             SHADE AUTOMATION V4 BUILD                ***\n" \
  "***                                                      ***\n" \
  "*** CONTROLLER: " CONTROLLER_NAME "\n" \
  "*** BUILD:      " BUILD_TARGET_NAME "\n" \
  "***                                                      ***\n" \
  "*** VERIFY THIS BEFORE FLASHING THE CONTROLLER!          ***\n" \
  "************************************************************" \
)



#define COMPILE_DATE __DATE__ " " __TIME__
#ifdef BUILD_PRODUCTION
  #define APPNAME "Windshield Shade Controller"                     // the name of this application
  #define APP_SOURCE_ID "ws_sd"
  #define PAGE_HEADER "wshdShade"                                   // web page header
  #define PG_TITLE "wshdShade Ctrl"
  #define ACCESS_PT_NAME "WS_SD_Controller"                         // access point name
#elif defined BUILD_TEST
  #define APPNAME "TEST Windshield Shade Controller"                // the name of this application
  #define APP_SOURCE_ID "test_ws_sd"
  #define PAGE_HEADER "TEST wshdShade"                              // web page header
  #define PG_TITLE "TEST wshdShade Ctrl"
  #define ACCESS_PT_NAME "TEST_WS_SD_Controller"                    // access point name
#endif
#define UPLOAD_PG "UPLOAD"                                          // page type is 'upload'

constexpr uint8_t RES_NINE   = 9;
constexpr uint8_t RES_TEN    = 10;
constexpr uint8_t RES_ELEVEN = 11;
constexpr uint8_t RES_TWELVE = 12;

constexpr const char* PCB_TEMP_TOPIC     = "shade/ws/pcb/temp";
constexpr const char* PCB_TEMP_LOG_TOPIC = "shade/ws/pcb/temp/log";

class ShadeAutomationV4 {
public:

  bool startup();
  void evtLoop();
 
private:

  EiDs18b20Sensor _pcbT = { "PCB", 0, TemperatureUnit::Fahrenheit, {0x28,0xFF,0x64,0x0E,0x7B,0x5D,0x58,0x9A}, 0.3, RES_NINE};

  RunTime _readSensor;
  uint8_t _gettempInterval = 1;

  void registerEiEvtHandelers();
  void setupHeartBeat();
  void addAppMQTTSubscriptions();
  void cfgMqttLwtPolicy();
  void configureMqtt();
  void fillAppIDs();
  void setupTempSensors();
  void logRawPcbTemp();
  void sendPcbTemp();

};

extern ShadeAutomationV4 shadeAuto;

extern void writeBootBanner();
extern void appWifiConnected();
extern void appWifiDisconnected();
extern void appMqttConnected() ;
extern void appMqttDisconnected();
extern  void appHandleMsg(const JsonDocument& doc, Source source);



