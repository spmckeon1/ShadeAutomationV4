
#include "ctrlOps.h"
#include "shadeAutomationV4.h"
#include <ei_appPolicy.h>
#include <ei_mqtt.h>

/*---------------  ON MQTT CONNECT TAKE CARE OF ALL NEEDED MQTT UBSCRIPTIONS---------------*/

void addAppMQTTSubscriptions()
{
    mqtt.addSubscription("Server",      TO_SERVER_SUB,          2);
    mqtt.addSubscription("App Data",    MQTT_SUB_APP_DATA,      0);
}


/*-----  CONFIGURE MqttLwtPolicy  -----*/

void cfgMqttLwtPolicy() {
	enabled             = true;
	topic               = appIDs.sourceId + "/mqtt/LWT/status";    
	String onlineMsg    = "Online";
	String offlineMsg   = "Offline";
	uint8_t qos         = 1;
	bool retain         = true;
}


/*-----  PCONFIG THE MQTT DATAS -----*/

void configureMqtt() {
	const MqttConfig cfgMqtt {"192.168.1.9", 1883, "curly", "redrover"};
	mqtt.configure(cfgMqtt);
}

/*-----  POPULATE THE appIds STRUCT -----*/

void fillAppIDs() {
  appIDs.appName = "Shade Automation V4";
  appIDs.sourceId =  "SD_AUTO";
  appIDs.accessPointName = "ShadeAutomationV4";
  appIDs.pageTitle = "Shades";
  appIDs.pageHeader = "Shades";
  appIDs.uploadPage = "UPLOAD";
}

ShadeAutomationV4 shadeAuto;


/*---- PERFORM ALL NEEDED STTARTUP ACTIVITIES ----*/

bool ShadeAutomationV4::startup() {
  configureMqtt();
  fillAppIDs();
  eiSystem.enableHeapMonitor(true);
  eiSystem.setHeapMonitorInterval(5);
  return true;
}


/*---- THE ShadeAutomationV4 CLASS EVENT LOOP  ----*/

void ShadeAutomationV4::evtLoop() {
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