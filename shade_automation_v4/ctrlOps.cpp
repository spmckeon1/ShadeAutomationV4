
#include "ctrlOps.h"
#include "config.h"

CtrlOps ctrlOps;

bool CtrlOps::startup() {
    pinMode(PK_BK_INPUT_PIN, INPUT);

    _pkBkState = (digitalRead(PK_BK_INPUT_PIN) == PK_BK_ON);
    _pkBkChanged = false;

    return true;
}

void CtrlOps::evtLoop() {
  bool currentState = isPkBkOn();
  if (currentState != _pkBkState) {
    _pkBkState = currentState;
    _pkBkChanged = true;
  }
}
bool CtrlOps::isPkBkOn() {
    return digitalRead(PK_BK_INPUT_PIN) == PK_BK_ON;
}