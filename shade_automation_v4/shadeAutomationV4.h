#pragma once
// ============================================================================
// Shade Automation
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

#if (defined(SHADE_CONTROLLER_DRIVER) + defined(SHADE_CONTROLLER_WINDSHIELD) + defined(SHADE_CONTROLLER_PASSENGER)) != 1
  #error "Exactly one Shade Automation controller must be selected."
#endif

// ============================================================================
// APPLICATION CONFIGURATION
// ============================================================================
#include "myConfig.h"

