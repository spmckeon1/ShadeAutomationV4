#pragma once

#include <Arduino.h>
#include <stdint.h>

#if CONFIG_IDF_TARGET_ESP32
    constexpr uint8_t DEVICE_IS_RUNNING = 2;
#elif CONFIG_IDF_TARGET_ESP32C3
    constexpr uint8_t DEVICE_IS_RUNNING = 8;
#else
    #error "Unsupported ESP32 target"
#endif

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

//#define SHADE_CONTROLLER_DRIVER
#define SHADE_CONTROLLER_WINDSHIELD
//#define SHADE_CONTROLLER_PASSENGER


#if (defined(SHADE_CONTROLLER_DRIVER) + defined(SHADE_CONTROLLER_WINDSHIELD) + defined(SHADE_CONTROLLER_PASSENGER)) != 1
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

#if defined(SHADE_CONTROLLER_DRIVER)
  #define CONTROLLER_NAME "DRIVER SHADE CONTROLLER"
#elif defined(SHADE_CONTROLLER_WINDSHIELD)
  #define CONTROLLER_NAME "WINDSHIELD SHADE CONTROLLER"
#elif defined(SHADE_CONTROLLER_PASSENGER)
  #define CONTROLLER_NAME "PASSENGER SHADE CONTROLLER"
#endif


#if defined(BUILD_PRODUCTION)
  #define BUILD_TARGET_NAME "PRODUCTION"
#elif defined(BUILD_TEST)
  #define BUILD_TARGET_NAME "TEST"
#endif

constexpr uint8_t EVENT_MAX_SUBSCRIBERS = 2;

// ============================================================================
// BUILD WARNING
// ============================================================================


// ============================================================================
// WINDSHIELD CONTROLLER
//
// Complete hardware profile for this physical controller.
//
// Hardware values are derived from the current production controller.
// Each controller will have its own complete profile, even when hardware
// values happen to be identical.
// ============================================================================

#if defined(SHADE_CONTROLLER_WINDSHIELD)
// Choose EXACTLY ONE active storage medium for your target hardware partition
#define SYSTEM_USES_LITTLEFS
// #define SYSTEM_USES_SD_CARD

// ============================================================================
// SYSTEM LOW-LEVEL CONSTANTS
// ============================================================================
  #define FORMAT_LITTLEFS_IF_FAILED   true
  #define SD_EVT_TYPE "WS_SD"
  #define SHADE_USES_DS18B20                            // Temperature sensor

/*
  constexpr uint8_t PK_BK_PIN             = 35;   // Parking brake
  constexpr uint8_t PK_BK_ON                    =  0;
  constexpr uint8_t PK_BK_OFF                   =  1;


  constexpr uint8_t TB6612FNG_STBY_PIN          =  4;   // TB6612FNG
  constexpr uint8_t NIGHT_SHADE_PWM_PIN         = 16;
  constexpr uint8_t DAY_SHADE_PWM_PIN           = 17;

  constexpr uint8_t NIGHT_SHADE_UP_INPUT_PIN    = 25;   // Night shade switches
  constexpr uint8_t NIGHT_SHADE_DOWN_INPUT_PIN  = 26;

  constexpr uint8_t DAY_SHADE_UP_INPUT_PIN      = 32;   // Day shade switches
  constexpr uint8_t DAY_SHADE_DOWN_INPUT_PIN    = 34;


  constexpr uint8_t NIGHT_SHADE_UP_OUTPUT_PIN   = 33;   // Night shade motor outputs
  constexpr uint8_t NIGHT_SHADE_DOWN_OUTPUT_PIN = 27;

  constexpr uint8_t DAY_SHADE_UP_OUTPUT_PIN     = 22;   // Day shade motor outputs
  constexpr uint8_t DAY_SHADE_DOWN_OUTPUT_PIN   = 21;
*/
#endif  // SHADE_CONTROLLER_WINDSHIELD

