#pragma once

#include <Arduino.h>

#include <ei_ds18b20.h>

#include "shadeDefs.h"
#include "config.h"


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

#else 
	#error "A shade controller must be defined.  Please do this in the 'shadeDefs.h' file befor before contnuing."
#endif




class OneWireOps {
public:
  bool setup();
  bool startup();
  void evtLoop();
  void setupTempSensors();
  void mqttConnected();
  void mqttDisconnected();
  void processMsg(const JsonDocument& doc);

private:
  RunTime _readSensor;
  uint8_t _gettempInterval = 1;
  EiDs18b20Sensor _pcbT = { SENSOR_NAME, SENSOR_APP_ID, SENSOR_TEMP_UNIT, SENSOR_ADDRESS, SENSOR_HYSTERESIS, SENSOR_RESOLUTION};

  bool setupEvents();
  void sendPcbTemp();
  void logRawPcbTemp();
  void updateNodeRed();

};

extern OneWireOps oneWireOps;

extern void onDs18b20SetupComplete();
extern void oneWireOpsMqttConnected();
void oneWireOpsMqttDisconnected();
