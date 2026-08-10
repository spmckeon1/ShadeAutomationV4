#pragma once

#include <Arduino.h>

class CtrlOps
{
public:

    bool startup();
    void evtLoop();

    bool isPkBkOn();
    float temperature();

private:
  bool _pkBkState = false;
  bool _pkBkChanged = false;
};

extern CtrlOps ctrlOps;
