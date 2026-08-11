
#include <Arduino.h>
#include <ArduinoTrace.h>

#include "ctrlOps.h"
#include "shadeAutomationV4.h"
#include <ei_appPolicy.h>
#include <ei_logging.h>
#include <ei_mqtt.h>
#include <ei_system.h>

ShadeAutomationV4 shadeAuto;

uint8_t sdEvtType;

/*-----    EI LIBRARY REQUIRED MSG RECEIVER   -----*/

void appHandleMsg(const JsonDocument& doc, Source source) {
}

/*-----  WRITE THE BOOT BANNER -----*/

void writeBootBanner() {
    JsonDocument doc;
    AppInfo::getAppInfo(doc, FI, COMPILE_DATE);
    logInfo(LS, SD_EVT_TYPE, "\n\n" + AppInfo::addRuntimeInfo(AppInfo::formatAppInfo(doc)) + "\n\n");
      digitalWrite(DEVICE_IS_RUNNING, HIGH);                                          // turm on the blue light on the ESP32
}

/*-----  ACTIONS TO TAKE ON WIFI CONNECT -----*/

void appWifiConnected() {
    logging.msg(__FILE__, FN, LN,
                T::EVENT,
                L::INFO,
                ET::USER,
                "Application received WifiConnected");
}

/*-----  ACTIOS TO TAKE O WIFI DISCONNECT -----*/

void appWifiDisconnected() {
    logging.msg(__FILE__, FN, LN,
                T::EVENT,
                L::INFO,
                ET::USER,
                "Application received WifiDisconnected");
}

/*-----  ACTIONS TO TAKE ON MQTT CONNECT -----*/

void appMqttConnected() {
    logging.msg(__FILE__, FN, LN,
                T::EVENT,
                L::INFO,
                ET::USER,
                "Application received MqttConnected");
}

/*-----  ACTIONS TO TAKE ON MQTT DISCONNECT -----*/

void appMqttDisconnected() {
    logging.msg(__FILE__, FN, LN,
                T::EVENT,
                L::INFO,
                ET::USER,
                "Application received MqttDisconnected");
}

/*-----  REGISTER ALL NEEDED EVENT HANDLERS  -----*/

void ShadeAutomationV4::registerEiEvtHandelers() {
    eiEvents.on(EiEvent::SystemReady, writeBootBanner);
    eiEvents.on(EiEvent::WifiConnected, appWifiConnected);
    eiEvents.on(EiEvent::WifiDisconnected, appWifiDisconnected);
    eiEvents.on(EiEvent::MqttConnected, appMqttConnected);
    eiEvents.on(EiEvent::MqttDisconnected, appMqttDisconnected);
}

/*---------------    SET HEARTBEAT   ---------------*/

void ShadeAutomationV4::setupHeartBeat() {
  mqttHbPolicy.enabled = true;
  mqttHbPolicy.interval = 60000;
  mqttHbPolicy.timeout = 180000;
}

/*---------------  ON MQTT CONNECT TAKE CARE OF ALL NEEDED MQTT UBSCRIPTIONS---------------*/

void ShadeAutomationV4::addAppMQTTSubscriptions() {
  const String TO_SERVER_SUB = String("to/server/") + appIDs.sourceId + "/#";       // to/server/SD_AUTO/#

  mqtt.setMaxSubCnt(1);
  mqtt.addSubscription("Server",      TO_SERVER_SUB,          2);
}


/*-----  CONFIGURE MqttLwtPolicy  -----*/

void ShadeAutomationV4::cfgMqttLwtPolicy() {
	appMqttLwtPolicy.enabled    = true;
	appMqttLwtPolicy.topic      =  String("/mqtt/LWT/") + appIDs.sourceId;    
	appMqttLwtPolicy.onlineMsg  = "Online";
	appMqttLwtPolicy.offlineMsg = "Offline";
	appMqttLwtPolicy.qos        = 1;
	appMqttLwtPolicy.retain     = true;
}


/*-----  PCONFIG THE MQTT DATAS -----*/

void ShadeAutomationV4::configureMqtt() {
	const MqttConfig cfgMqtt {"192.168.1.9", 1883, "curly", "redrover"};
	mqtt.configure(cfgMqtt);
}

/*-----  POPULATE THE appIds STRUCT -----*/

void ShadeAutomationV4::fillAppIDs() {
  appIDs.appName = "Shade Automation V4";
  appIDs.sourceId =  "SD_AUTO";
  appIDs.accessPointName = "ShadeAutomationV4";
  appIDs.pageTitle = "Shades";
  appIDs.pageHeader = "Shades";
  appIDs.uploadPage = "UPLOAD";
}

/*---- PERFORM ALL NEEDED STTARTUP ACTIVITIES ----*/

bool ShadeAutomationV4::startup() {
  fillAppIDs();
  setupHeartBeat();
  eiSystem.enableHeapMonitor(true);
  eiSystem.setHeapMonitorInterval(5);

  sdEvtType = logging.registerEventType(SD_EVT_TYPE); 

  if(!eiSystem.bootStrap()) DUMP("eiSystem.bootStrap() FAILURE");
  cfgMqttLwtPolicy();
  if(!eiSystem.setup()) DUMP("eiSystem.setup() FAILURE");
  configureMqtt();
  
  registerEiEvtHandelers();
  if(!eiSystem.startup()) DUMP("eiSystem.startup() FAILURE");
	addAppMQTTSubscriptions();

  return true;
}


/*---- THE ShadeAutomationV4 CLASS EVENT LOOP  ----*/

void ShadeAutomationV4::evtLoop() {
  eiSystem.evtLoop();
  ctrlOps.evtLoop();
}


/*---- CALL ALL SETUP ITEMS HERE ----*/

void setup() {
  shadeAuto.startup();
}


/*---- RUN THE MAIN LOOP ----*/

void loop() {
  shadeAuto.evtLoop();
}