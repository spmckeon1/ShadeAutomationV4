#define DO_NOT_COMPILE
#ifndef DO_NOT_COMPILE



#include <ArduinoTrace.h>
#include <OneWire.h>

#include <ei_appFramework.h>

#include "oneWireOps.h"

OneWireOps oneWireOps;
inline constexpr const char ONE_WIRE_OPS[] = "ONE_WIRE_OPS";

bool OneWireOps::setup() {
  ds18b20.sendStartupData(DS18B20_DATA_PIN, countOfTempSensors);
  setupEvents();
  return true;
}

bool OneWireOps::startup() {
  ds18b20.setHysteresis(oneWireOps._pcbT);
  _readSensor = {IntervalType::IT_MINUTE, _gettempInterval, -1};           // init the eventloop read sensore timer
  eiEvents.on(EiEvent::MqttConnected, oneWireOpsMqttConnected);
  eiEvents.on(EiEvent::MqttDisconnected, oneWireOpsMqttDisconnected);
  logInfo(LS, ONE_WIRE_OPS, "ONE_WIRE_OPS setup() has completed");
  return true;
}

void OneWireOps::evtLoop() {
//  if(_pcbT.rptTempUpdated) {                                // if a temp sensor update has been posted
//    sendPcbTemp();                                          // send the update
//    _pcbT.rptTempUpdated = false;
//  }
}

/*-----  SETUP ONE WIRE OPS EVENTS  -----*/

bool OneWireOps::setupEvents() {
  if (!eiEvents.on(EiEvent::Ds18b20SetupComplete, onDs18b20SetupComplete))
    return false;
  return true;
}

/*-----  SHANDLE THE onDs18b20SetupComplete EVENT  -----*/

void onDs18b20SetupComplete() {
  TRACE();
  oneWireOps.setupTempSensors();
}

/*-----  SEND THE CURRENT PCB TEMPERATURE  -----*/

void OneWireOps::sendPcbTemp() {
  int curTemp = ds18b20.getHysteresisTempF(_pcbT.ds18b20Index);
  JsonDocument data;
  data["schema"] = "pcbTemperature.v1";
  data["temperature"] = curTemp;
  String json = appFramework.buildJsonAppMqttMsg(String(SD_CMD_ROUTE), "TEMPERATURE", data.as<JsonObjectConst>());
  mqtt.mqttPubMsg(appIDs.mqttTopic, QOS0, FORGET, json, LN);
}

/*-----    LOG THE TEMOERATURE SEMSOR AW TEMPERATURE IN ºF   -----*/

void OneWireOps::logRawPcbTemp() {
  logInfo(LS, SD_EVT_TYPE, String(ds18b20.getRawTempF(oneWireOps._pcbT.ds18b20Index)));
}
 
/*-----    SET THE PCB TEMPERATURE SENSOR UP   -----*/

void OneWireOps::setupTempSensors() {
    if(!ds18b20.addSensor(oneWireOps._pcbT))
      logError(LS, ET::SENSOR, "failed to add the temperature sensor '" + 
              oneWireOps._pcbT.name + "'. Received error: " + 
              String(static_cast<int>(oneWireOps._pcbT.result)));
}

void OneWireOps::updateNodeRed() {
  sendPcbTemp();
}

void oneWireOpsMqttConnected() {
  oneWireOps.mqttConnected();
}

void OneWireOps::mqttConnected() {

}

void oneWireOpsMqttDisconnected() {
  oneWireOps.mqttDisconnected();
}

void OneWireOps::mqttDisconnected() {
  // PLACE ANY CODE NEEDED WHEN MQTT DISCONNECTS HERE
}

void OneWireOps::processMsg(const JsonDocument& doc) {
  const char* command = doc["command"] | "";
  if (strcmp(command, "REFRESH") == 0) {
    updateNodeRed();
    return;
  }
}

#endif