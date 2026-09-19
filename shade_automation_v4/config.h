//#ifndef SHADE_AUTOMATION_V4_CONFIG_H
//#define SHADE_AUTOMATION_V4_CONFIG_H

#pragma once

#include <Arduino.h>
#include <stdint.h>

// ============================================================================
// SHADE AUTOMATION V4
// BUILD CONFIGURATION
//
// This file describes the complete physical device being built.
//
// Select exactly one controller.
// Select exactly one build target.
//
// Check the compiler output before flashing!
// ============================================================================


// ============================================================================
// CONTROLLER SELECTION
// ============================================================================

//#define WINDSHIELD_SHADES
//#define DRIVER_SHADES
#define PASSENGER_SHADES

#if (defined(WINDSHIELD_SHADES) + defined(DRIVER_SHADES) + defined(PASSENGER_SHADES)) != 1
  #error "ERROR: Exactly ONE shade controller must be selected."
#endif

// ============================================================================
// BUILD TARGET
// ============================================================================

//#define BUILD_PRODUCTION
#define BUILD_TEST


#if (defined(BUILD_PRODUCTION) + defined(BUILD_TEST)) != 1
  #error "ERROR: Exactly ONE build target must be selected."
#endif


// ============================================================================
// BUILD IDENTIFICATION
// ============================================================================

#if defined(DRIVER_SHADES)
  #define CONTROLLER_NAME "DRIVER SHADE CONTROLLER"
#elif defined(WINDSHIELD_SHADES)
  #define CONTROLLER_NAME "WINDSHIELD SHADE CONTROLLER"
#elif defined(PASSENGER_SHADES)
  #define CONTROLLER_NAME "PASSENGER SHADE CONTROLLER"
#endif

#if defined(BUILD_PRODUCTION)
  #define BUILD_TARGET_NAME "PRODUCTION"
#elif defined(BUILD_TEST)
  #define BUILD_TARGET_NAME "TEST"
#endif

// ============================================================================
// CONTROLLER HARDWARE PROFILE
// ============================================================================

#if defined(WINDSHIELD_SHADES)

// ============================================================================
// WINDSHIELD CONTROLLER
// ============================================================================

// Choose EXACTLY ONE active storage medium for this controller.

#define SYSTEM_USES_LITTLEFS
// #define SYSTEM_USES_SD_CARD

#define FORMAT_LITTLEFS_IF_FAILED   true

#define SENSOR_USES_DS18B20

// ============================================================================

#elif defined(DRIVER_SHADES)

// ============================================================================
// DRIVER WINDOW CONTROLLER
// ============================================================================

// Choose EXACTLY ONE active storage medium for this controller.

#define SYSTEM_USES_LITTLEFS
// #define SYSTEM_USES_SD_CARD

#define FORMAT_LITTLEFS_IF_FAILED   true

#define SENSOR_USES_DS18B20

// ============================================================================

#elif defined(PASSENGER_SHADES)

// ============================================================================
// PASSENGER WINDOW CONTROLLER
// ============================================================================

// Choose EXACTLY ONE active storage medium for this controller.

#define SYSTEM_USES_LITTLEFS
// #define SYSTEM_USES_SD_CARD

#define FORMAT_LITTLEFS_IF_FAILED   true

#define SENSOR_USES_DHT

// ============================================================================

#else

  #error "ERROR: No valid shade controller profile selected."

#endif