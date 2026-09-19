#pragma once

// The application uses the hardware profile selected in config.h.

#include <Arduino.h>
#include <ArduinoJson.h>
#include <DallasTemperature.h>


#include "config.h"
#include <ei_ds18b20.h>
#include <ei_scheduler.h>
#include <ei_utilities.h>

 #include "shadeDefs.h"

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
//#define SD_CMD_ROUTE APP_SOURCE_ID "/to/nr/shadeState"

#ifdef WINDSHIELD_SHADES
  constexpr uint8_t DS18B20_DATA_PIN            = 13;
  constexpr uint8_t countOfTempSensors          =  1;
  constexpr uint8_t ONE_WIRE_BUS = 								13;		// ESP32 pin connected to the DS18B20 data wire
  #define SENSOR_NAME        "PCB"
  #define SENSOR_APP_ID      0
  #define SENSOR_TEMP_UNIT   TemperatureUnit::Fahrenheit
  #define SENSOR_ADDRESS     {0x28,0xFF,0x64,0x0E,0x7B,0x5D,0x58,0x9A}
  #define SENSOR_HYSTERESIS  0.3
  #define SENSOR_RESOLUTION  RES_NINE

#elif defined DRIVER_SHADES
  constexpr uint8_t DS18B20_DATA_PIN            = 13;
  constexpr uint8_t countOfTempSensors          =  1;
  constexpr uint8_t ONE_WIRE_BUS = 								13;		// ESP32 pin connected to the DS18B20 data wire
  #define SENSOR_NAME        "PCB"
  #define SENSOR_APP_ID      0
  #define SENSOR_TEMP_UNIT   TemperatureUnit::Fahrenheit
  #define SENSOR_ADDRESS     {0x28,0xA1,0x05,0x0B,0x00,0x00,0x00,0x33}
  #define SENSOR_HYSTERESIS  0.3
  #define SENSOR_RESOLUTION  RES_NINE

#elif defined PASSENGER_SHADES
  #define SENSOR_NAME        "PCB"
  #define DHTPIN 33                       // Digital pin connected to the DHT sensor
  #define DHTTYPE DHT22                   // DHT 22 (AM2302)
  #define SENSOR_HYSTERESIS  0.3
  #define DHT22_READ_INTERVAL 5000        // ms between sensors reads

#else 
	#error "A shade controller must be defined.  Please do this in the 'shadeDefs.h' file befor before contnuing."
#endif



class ShadeAutomationV4 {
public:
  bool setup();
  void evtLoop();
  void mqttConnected();
  void setupTempSensors();
  void sendPcbTemp();
  bool hdlAppInfoRequest();
 
private:

  void logRawPcbTemp();

  void registerEiEvtHandelers();
  void setupHeartBeat();
  void addAppMQTTSubscriptions();
  void cfgMqttLwtPolicy();
  void configureMqtt();
  void fillAppIDs();
  void setupMqttTopics();
  void sensorSetup();

};

extern ShadeAutomationV4 shadeAuto;

extern void writeBootBanner();
extern void appWifiConnected();
extern void appWifiDisconnected();
extern void appMqttConnected() ;
extern void appMqttDisconnected();
extern bool appHandleMsg(const JsonDocument& doc);
extern void onDs18b20SetupComplete();
extern void appTempChg();



