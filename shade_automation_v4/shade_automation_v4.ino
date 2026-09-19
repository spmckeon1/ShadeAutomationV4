#include <Arduino.h>
#include <ArduinoTrace.h>

#include "shadeAutomationV4.h"
#include "ctrlOps.h"
#include <ei_appFramework.h>

#ifdef SENSOR_USES_DS18B20
  #include <ei_ds18b20.h>
#elif defined(SENSOR_USES_DHT)
  #include <ei_dht.h>
#endif

#include <ei_logging.h>
#include <ei_mqtt.h>
//#include <ei_sensors.h>
#include <ei_system.h>

#include "shadeOps.h"

ShadeAutomationV4 shadeAuto;

uint8_t sdEvtType;

void ShadeAutomationV4::setupTempSensors() {
#ifdef SENSOR_USES_DS18B20
	EiDs18b20Sensor _pcbT = { SENSOR_NAME, SENSOR_APP_ID, SENSOR_TEMP_UNIT, SENSOR_ADDRESS, SENSOR_HYSTERESIS, SENSOR_RESOLUTION};
  if(!ds18b20.addSensor(_pcbT))
    logError(LS, ET::SENSOR, "failed to add the temperature sensor.");
#elif defined(SENSOR_USES_DHT)
	// DHT ELEMENTS - THIS IS CURRENTLY NOT SED BY DHT SENSORS
#endif 
}

/*-----  SHANDLE THE onDs18b20SetupComplete EVENT  -----*/

void onDs18b20SetupComplete() {
  shadeAuto.setupTempSensors();
}

/*-----  HANDLE A TEMPERATURE SENSOR READING CHANGE  -----*/

void appTempChg() {
  shadeAuto.sendPcbTemp();
}

/*-----  SEND THE CURRENT PCB TEMPERATURE  -----*/

void ShadeAutomationV4::sendPcbTemp() {
    TRACE();
#ifdef SENSOR_USES_DS18B20
    JsonDocument data;
    data["schema"] = "pcbTemperature.v1";
    data["name"] = SENSOR_NAME;
    data["temperature"] = ds18b20.getHysteresisTempF(ds18b20.getSensorId(SENSOR_APP_ID));
    mqtt.mqttPubMsg(appIDs.mqttTopic, QOS0, FORGET, appFramework.buildJsonAppMqttMsg(String(SD_CMD_ROUTE), "TEMPERATURE", data.as<JsonObjectConst>()), LN);

#elif defined(SENSOR_USES_DHT)
    JsonDocument data;
    data["schema"] = "pcbTemperature.v1";
    data["name"] = SENSOR_NAME;
    data["temperature"] = dht.getTempF();
    mqtt.mqttPubMsg(appIDs.mqttTopic, QOS0, FORGET, appFramework.buildJsonAppMqttMsg(String(SD_CMD_ROUTE), "TEMPERATURE", data.as<JsonObjectConst>()), LN);

#endif
}
/*-----    LOG THE TEMOERATURE SEMSOR AW TEMPERATURE IN ºF   -----*/

void ShadeAutomationV4::logRawPcbTemp() {
  TRACE();
#ifdef SENSOR_USES_DS18B20
  logInfo(LS, SD_EVT_TYPE, String(ds18b20.getTempF(ds18b20.getSensorId(SENSOR_APP_ID))));

#elif defined(SENSOR_USES_DHT)
  logInfo(LS, SD_EVT_TYPE, String(dht.getTempF()));

#endif
}
/*-----    EI LIBRARY REQUIRED MSG RECEIVER   -----*/

bool appHandleMsg(const JsonDocument& doc) {
  const char* route = doc["route"] | "";
  DUMP(route);
  const String shadeOpsRoute = "nr/to/" + String(appIDs.sourceId) + "/shadeOps";
  const String appInfoRoute = "nr/to/" + String(appIDs.sourceId) + "/appInfo";
  const String pageRefreshRoute = "nr/to/" + String(appIDs.sourceId) + "/pg/refresh";
  DUMP(shadeOpsRoute);
  DUMP(appInfoRoute);
  DUMP(pageRefreshRoute);
  
  if (String(route).startsWith(shadeOpsRoute)) {
    shadeOps.processMsg(doc);
    return true;
  }
  if (strcmp(route, appInfoRoute.c_str()) == 0) {
    return shadeAuto.hdlAppInfoRequest();
  }
  if (strcmp(route, pageRefreshRoute.c_str()) == 0) {
    shadeOps.processMsg(doc);
    ctrlOps.processMsg(doc);
    shadeAuto.sendPcbTemp();
    return true;
  }
  logError(LS, SD_EVT_TYPE, "Failed to route msg: " + String(route));
  return false;
}

/*-----  HANDLE THE APPLICATION INFORMATION REQUEST -----*/

bool ShadeAutomationV4::hdlAppInfoRequest() {

    JsonDocument data;

    AppInfo::getAppInfo(data, FI, COMPILE_DATE);

    String message =
        appFramework.buildJsonAppMqttMsg(
            String(appIDs.sourceId) + "/to/nr/appInfo",
            "RESPONSE",
            data.as<JsonObjectConst>()
        );

    return mqtt.mqttPubMsg(
        appIDs.mqttTopic,
        QOS0,
        FORGET,
        message,
        LN
    );
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

/*-----  CLASS FUNCTION TO HANDLE MQTT CONNECTED ACTIONS -----*/

void ShadeAutomationV4::mqttConnected() {
  JsonDocument doc;
  doc["sourceId"] = appIDs.sourceId;
  String payload;
  serializeJson(doc, payload);
//PUT STUFF HERE THAT NEED STO GO TO NODE RED ON CONNECT
}

/*-----  ACTIONS TO TAKE ON MQTT CONNECT -----*/

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
  #ifdef SENSOR_USES_DS18B20
    eiEvents.on(EiEvent::Ds18b20SetupComplete, onDs18b20SetupComplete);
    eiEvents.on(EiEvent::Ds18b20TempChg, appTempChg);
  #elif defined(SENSOR_USES_DHT)
    // PUT DHT eiEvents.on CALLS HERE - CURRENTLY NT IN USE FOR DHT SENSORS
  #endif
}

/*---------------    SET HEARTBEAT   ---------------*/

void ShadeAutomationV4::setupHeartBeat() {
  mqttHbPolicy.enabled = true;
  mqttHbPolicy.interval = 60000;
  mqttHbPolicy.timeout = 180000;
}

/*---------------  ON MQTT CONNECT TAKE CARE OF ALL NEEDED MQTT UBSCRIPTIONS---------------*/

void ShadeAutomationV4::addAppMQTTSubscriptions() {
  const String TO_SERVER_SUB = String("nr/to/") + appIDs.sourceId + "/#";       // nr/to/<sourceId/#

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
  appIDs.pageId = PG_ID;
  appIDs.accessPointName = ACCESS_PT_NAME;
  appIDs.pageTitle = PG_TITLE;
  appIDs.pageHeader = PAGE_HEADER;
  appIDs.uploadPage = UPLOAD_PG;
  appIDs.appVersion = APP_VERSION;
  appIDs.mqttTopic = APP_SOURCE_ID "/to/nr";
}

/*-----  SETUP MQTT TOPICS -----*/

void ShadeAutomationV4::setupMqttTopics() {
  // CREATE ANY NEEDED MQTT TICS HERE
}

/*-----  DO THE INITIAL SENSOR SETUP -----*/

void ShadeAutomationV4::sensorSetup() {
  #ifdef SENSOR_USES_DS18B20
    ds18b20.sendStartupData(DS18B20_DATA_PIN, countOfTempSensors);
  #elif defined(SENSOR_USES_DHT)
    dht.setConfig(DHTPIN, DHTTYPE, DHT22_READ_INTERVAL);
  #endif
}

/*---- PERFORM ALL NEEDED STTARTUP ACTIVITIES ----*/

bool ShadeAutomationV4::setup() {
  setupMqttTopics();
  mqtt.addToSubCount(1);
  fillAppIDs();
  setupHeartBeat();
  eiSystem.enableHeapMonitor(true);
  eiSystem.setHeapMonitorInterval(5);
  sdEvtType = logging.registerEventType(SD_EVT_TYPE); 

  if(!eiSystem.bootStrap()) DUMP("eiSystem.bootStrap() FAILURE");

  sensorSetup();

  cfgMqttLwtPolicy();
  registerEiEvtHandelers();
  if(!eiSystem.setup()) 
    DUMP("eiSystem.setup() FAILURE");

  configureMqtt();
	addAppMQTTSubscriptions();
  if(!eiSystem.startup()) DUMP("eiSystem.startup() FAILURE");
//  sensors.startup();
  ctrlOps.setup();
  shadeOps.setup();
  shadeOps.startup();

  logging.dividerStr(FN, LN);                                                // end of function, log a seperator
  return true;
}


/*---- THE ShadeAutomationV4 CLASS EVENT LOOP  ----*/

void ShadeAutomationV4::evtLoop() {
  eiSystem.evtLoop();
  ctrlOps.evtLoop();
  shadeOps.evtLoop();
//  DUMP(_pcbT.rptTempUpdated);
//  sensors.evtLoop();

}


/*---- CALL ALL SETUP ITEMS HERE ----*/

void setup() {
  shadeAuto.setup();
}


/*---- RUN THE MAIN LOOP ----*/

void loop() {
  shadeAuto.evtLoop();
}

