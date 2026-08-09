#ifndef MY_CONFIG_H
#define MY_CONFIG_H

// ============================================================================
// ENVIRONMENTAL SENSOR
// Derived from controller selection.
// ============================================================================

#if defined(SHADE_CONTROLLER_PASSENGER)
  #define SHADE_USES_DHT22
#else
  #define SHADE_USES_DS18B20
#endif


// ============================================================================
// STORAGE
// ============================================================================

#define SYSTEM_USES_LITTLEFS
// #define SYSTEM_USES_SD_CARD

#define FORMAT_LITTLEFS_IF_FAILED true


// ============================================================================
// COMMON SHADE HARDWARE
// ============================================================================

// Physical shade switches
#define NIGHT_SHADE_UP_INPUT_PIN       25
#define NIGHT_SHADE_DOWN_INPUT_PIN     26
#define DAY_SHADE_UP_INPUT_PIN         32
#define DAY_SHADE_DOWN_INPUT_PIN       34

// Motor direction control
#define NIGHT_SHADE_UP_OUTPUT_PIN      33
#define NIGHT_SHADE_DOWN_OUTPUT_PIN    27
#define DAY_SHADE_UP_OUTPUT_PIN        22
#define DAY_SHADE_DOWN_OUTPUT_PIN      21

// TB6612FNG
#define TB6612FNG_STBY_PIN              4
#define NIGHT_SHADE_PWM_PIN            16
#define DAY_SHADE_PWM_PIN              17

// Parking brake
#define PARKING_BRAKE_INPUT_PIN        35
#define PARKING_BRAKE_ON_STATE          0
#define PARKING_BRAKE_OFF_STATE         1


// ============================================================================
// TEMPERATURE SENSOR HARDWARE
// ============================================================================

#if defined(SHADE_USES_DS18B20)

  #define DS18B20_DATA_PIN             13

#elif defined(SHADE_USES_DHT22)

  #define DHT22_DATA_PIN               33

#endif

#endif
