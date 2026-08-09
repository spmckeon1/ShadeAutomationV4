#ifndef MY_CONFIG_H
#define MY_CONFIG_H

#include <stdint.h>


// ============================================================================
// SHADE AUTOMATION V4
// BUILD CONFIGURATION
//
// THIS FILE DESCRIBES THE COMPLETE PHYSICAL DEVICE BEING BUILT.
//
// SELECT EXACTLY ONE CONTROLLER.
// SELECT EXACTLY ONE BUILD TARGET.
//
// CHECK THE COMPILER OUTPUT BEFORE FLASHING!
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


// ============================================================================
// BUILD WARNING
// ============================================================================

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


// ============================================================================
// WINDshield CONTROLLER
//
// Complete hardware profile for this physical controller.
// Values are derived from the current production controller.
//
// Do not assume another controller has the same hardware simply because
// a value happens to be identical.
// ============================================================================

#if defined(SHADE_CONTROLLER_WINDSHIELD)
  #define SYSTEM_USES_LITTLEFS                                                // Storage
  #define FORMAT_LITTLEFS_IF_FAILED true

  #define PK_BK_INPUT_PIN               35                                   // Parking brake
  #define PK_BK_ON                       0
  #define PK_BK_OFF                      1
  #define SHADE_USES_DS18B20                                                  // Temperature sensor
  #define DS18B20_DATA_PIN               13

  #define TB6612FNG_STBY_PIN              4                                   // TB6612FNG
  #define NIGHT_SHADE_PWM_PIN            16
  #define DAY_SHADE_PWM_PIN              17

  #define NIGHT_SHADE_UP_INPUT_PIN       25                                   // Night shade switches
  #define NIGHT_SHADE_DOWN_INPUT_PIN     26

  #define DAY_SHADE_UP_INPUT_PIN         32                                    // Day shade switches
  #define DAY_SHADE_DOWN_INPUT_PIN       34


  #define NIGHT_SHADE_UP_OUTPUT_PIN      33                                    // Night shade motor outputs
  #define NIGHT_SHADE_DOWN_OUTPUT_PIN    27

  #define DAY_SHADE_UP_OUTPUT_PIN        22                                    // Day shade motor outputs
  #define DAY_SHADE_DOWN_OUTPUT_PIN      21

#endif  // SHADE_CONTROLLER_WINDSHIELD


#endif  // MY_CONFIG_H