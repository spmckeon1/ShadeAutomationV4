#pragma once

// ============================================================================
// Shade Automation V4
//
// Top-level application definition.
//
// The controller selection below is the single place where the target
// controller is selected for a build.
// ============================================================================

// ============================================================================
// CONTROLLER SELECTION
// ============================================================================

//#define SHADE_CONTROLLER_DRIVER
#define SHADE_CONTROLLER_WINDSHIELD
//#define SHADE_CONTROLLER_PASSENGER

#if (defined(SHADE_CONTROLLER_DRIVER) + \
     defined(SHADE_CONTROLLER_WINDSHIELD) + \
     defined(SHADE_CONTROLLER_PASSENGER)) != 1
  #error "Exactly one Shade Automation V4 controller must be selected."
#endif

// ============================================================================
// APPLICATION CONFIGURATION
// ============================================================================

#include "myConfig.h"

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