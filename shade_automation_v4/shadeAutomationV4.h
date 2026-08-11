#pragma once

// The application uses the hardware profile selected in config.h.

#include <Arduino.h>
#include <ArduinoJson.h>


#include "config.h"
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

// ============================================================================
// APPLICATION
// ============================================================================

class ShadeAutomationV4 {
public:

  bool startup();
  void evtLoop();
 
private:

  void registerEiEvtHandelers();
  void setupHeartBeat();
  void addAppMQTTSubscriptions();
  void cfgMqttLwtPolicy();
  void configureMqtt();
  void fillAppIDs();

};

extern ShadeAutomationV4 shadeAuto;

extern void writeBootBanner();
extern void appWifiConnected();
extern void appWifiDisconnected();
extern void appMqttConnected() ;
extern void appMqttDisconnected();
extern  void appHandleMsg(const JsonDocument& doc, Source source);



