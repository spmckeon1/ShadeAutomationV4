
#include <ArduinoTrace.h>

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
	logInfo(LS, CTRL_OPS, "CTRL_OPS setup() has completed");

    return true;
}

bool CtrlOps::startup() {

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
}

bool CtrlOps::isParkingBrakeOn() const {
    return _pkBkState;
}

