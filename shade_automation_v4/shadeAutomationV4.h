#pragma once

// The application uses the hardware profile selected in config.h.

#include <Arduino.h>

#include "config.h"

// ============================================================================
// APPLICATION
// ============================================================================

class ShadeAutomationV4
{
public:

  bool startup();
  void evtLoop();
};

extern ShadeAutomationV4 shadeAuto;