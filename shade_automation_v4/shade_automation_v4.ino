
#include <Arduino.h>
#include <ArduinoTrace.h>

#include "shadeAutomationV4.h"
#include "ctrlOps.h"
#include <ei_appPolicy.h>
#include <ei_logging.h>
#include <ei_mqtt.h>
#include <ei_system.h>

#include "shadeOps.h"

ShadeAutomationV4 shadeAuto;

uint8_t sdEvtType;

void ShadeAutomationV4::sendPcbTemp() {
  int curTemp = ds18b20.getHysteresisTempF(_pcbT.ds18b20Index);
  DUMP(curTemp);
  mqtt.mqttPubMsg(PCB_TEMP_TOPIC, QOS0, FORGET, String(curTemp), LN);
}

/*-----    LOG THE TEMOERATURE SEMSOR AW TEMPERATURE IN ºF   -----*/

void ShadeAutomationV4::logRawPcbTemp() {
  logInfo(LS, SD_EVT_TYPE, String(ds18b20.getRawTempF(shadeAuto._pcbT.ds18b20Index)));
}
 
/*-----    SET THE PCB TEMPERATURE SENSOR UP   -----*/

void ShadeAutomationV4::setupTempSensors() {
    if(!ds18b20.addSensor(shadeAuto._pcbT))
      logError(LS, ET::SENSOR, "failed to add the temperature sensor '" + 
              shadeAuto._pcbT.name + "'. Received error: " + 
              String(static_cast<int>(shadeAuto._pcbT.result)));
}

/*-----    EI LIBRARY REQUIRED MSG RECEIVER   -----*/

bool appHandleMsg(const JsonDocument& doc, Source source) {

  return false;
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

/*-----  CLASS FUNCTIOM TO HANDLE MQTT CONNECTED ACTIONS -----*/

void ShadeAutomationV4::mqttConnected() {
  JsonDocument doc;
  doc["sourceId"] = appIDs.sourceId;
  String payload;
  serializeJson(doc, payload);
//PUT STUFF HERE THAT NEED STO GO TO NODE RED ON CONNECT
}/*-----  ACTIONS TO TAKE ON MQTT CONNECT -----*/

void appMqttConnected() {
  shadeAuto.mqttConnected();
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
  appIDs.appName = APPNAME;
  appIDs.sourceId =  APP_SOURCE_ID;
  appIDs.accessPointName = ACCESS_PT_NAME;
  appIDs.pageTitle = PG_TITLE;
  appIDs.pageHeader = PAGE_HEADER;
  appIDs.uploadPage = UPLOAD_PG;
}

/*-----  SETUP MQTT TOPICS -----*/

void ShadeAutomationV4::setupMqttTopics() {
  // CREATE ANY NEEDED MQTT TICS HERE
}

/*---- PERFORM ALL NEEDED STTARTUP ACTIVITIES ----*/

bool ShadeAutomationV4::startup() {
  setupMqttTopics();
  fillAppIDs();
  setupHeartBeat();
  ds18b20.sendStartupData(DS18B20_DATA_PIN, countOfTempSensors);
  eiSystem.enableHeapMonitor(true);
  eiSystem.setHeapMonitorInterval(5);
  sdEvtType = logging.registerEventType(SD_EVT_TYPE); 

  if(!eiSystem.bootStrap()) DUMP("eiSystem.bootStrap() FAILURE");
  cfgMqttLwtPolicy();
  if(!eiSystem.setup()) DUMP("eiSystem.setup() FAILURE");

  shadeAuto.setupTempSensors();
  configureMqtt();
  if(!eiSystem.startup()) DUMP("eiSystem.startup() FAILURE");
	addAppMQTTSubscriptions();
  registerEiEvtHandelers();
  ds18b20.setHysteresis(shadeAuto._pcbT);
//  ds18b20.setReadInterval(5000);
   _readSensor = {IntervalType::IT_MINUTE, _gettempInterval, -1};           // init the eventloop read sensore timer
  ctrlOps.setup();
  shadeOps.setup();
  ctrlOps.startup();
  shadeOps.startup();

  logging.dividerStr(FN, LN);                                               // end of function, log a seperator

  return true;
}


/*---- THE ShadeAutomationV4 CLASS EVENT LOOP  ----*/

void ShadeAutomationV4::evtLoop() {
  eiSystem.evtLoop();
  ctrlOps.evtLoop();
  shadeOps.evtLoop();
  if(_pcbT.rptTempUpdated) {                                // if a tem sensor update has been posted
    sendPcbTemp();                                          // send the update
    DUMP(ds18b20.getHysteresisTempF(_pcbT.ds18b20Index));
    _pcbT.rptTempUpdated = false;
  }


}


/*---- CALL ALL SETUP ITEMS HERE ----*/

void setup() {
  shadeAuto.startup();
}


/*---- RUN THE MAIN LOOP ----*/

void loop() {
  shadeAuto.evtLoop();
}