
#include <ArduinoTrace.h>

#include <ei_appFramework.h>
#include <ei_logging.h>

#include "ctrlOps.h"
#include "shadeDefs.h"
#include "appEvents.h"
#include "config.h"

inline constexpr const char CTRL_OPS[] = "CTRL_OPS";

CtrlOps ctrlOps;

bool CtrlOps::setup() {
  pinMode(PK_BK_PIN, INPUT);
  _pkBkState = (digitalRead(PK_BK_PIN) == PK_BK_ON);
  logInfo(LS, CTRL_OPS, "The parking brake is: " + String((_pkBkState ? "ON":"OFF")));
  _pkBkChanged = false;
  appEvents.on(AppEvent::ParkingBrakeChanged, parkingBrakeChanged);
  eiEvents.on(EiEvent::MqttConnected, ctrlOpsMqttConnected);
  eiEvents.on(EiEvent::MqttDisconnected, ctrlOpsMqttDisconnected);
	logInfo(LS, CTRL_OPS, "CTRL_OPS setup() has completed");

    return true;
}

bool CtrlOps::startup() {
  sendParkingBrakeState();
	logInfo(LS, CTRL_OPS, "CTRL_OPS startup() has completed");
  return true;
}

void CtrlOps::evtLoop() {
  bool currentState = isPkBkOn();
  if (currentState != _pkBkState) {
    _pkBkState = currentState;
    _pkBkChanged = true;
    appEvents.emit(AppEvent::ParkingBrakeChanged);
  }
}
bool CtrlOps::isPkBkOn() {
    return digitalRead(PK_BK_PIN) == PK_BK_ON;
}

void parkingBrakeChanged() {
  logInfo(LS, CTRL_OPS, "The parking brake has transistioned to: " + String(ctrlOps.isParkingBrakeOn()?"ON":"OFF"));
  ctrlOps.sendParkingBrakeState();
}

bool CtrlOps::isParkingBrakeOn() const {
    return _pkBkState;
}

void CtrlOps::sendParkingBrakeState() {
  JsonDocument data;
  data["schema"] = "ctrlOps";
  data["ParkBrake"] = ctrlOps.isParkingBrakeOn() ? "ON" : "OFF";
  String json = appFramework.buildJsonAppMqttMsg(PK_BK_ROUTE, "PARK_BRAKE", data.as<JsonObjectConst>());
  DUMP(appIDs.mqttTopic);
  DUMP(json);
  mqtt.mqttPubMsg(appIDs.mqttTopic, QOS0, FORGET, json, LN);
}

void CtrlOps::updateNodeRed() {
  sendParkingBrakeState();
}

void ctrlOpsMqttConnected() {
  ctrlOps.mqttConnected();
}

void CtrlOps::mqttConnected() {
  updateNodeRed();
}

void ctrlOpsMqttDisconnected() {
  ctrlOps.mqttDisconnected();
}

void CtrlOps::mqttDisconnected() {
  // PLACE ANY CODE NEEDED WHEN MQTT DISCONNECTS HERE
}

void CtrlOps::processMsg(const JsonDocument& doc) {
  const char* command = doc["command"] | "";
  if (strcmp(command, "REFRESH") == 0) {
    updateNodeRed();
    return;
  }
}