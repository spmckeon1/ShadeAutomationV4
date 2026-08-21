#pragma once

// ============================================================================
// SHADE OPERATIONS
//
// Represents one physical shade and owns its movement behavior.
// ============================================================================

#include <Arduino.h>

#include "shadeDefs.h"

#ifdef WINDSHIELD_SHADES
  constexpr uint8_t NT_SW_UP_PIN = 								25;		// night shade up switch pin
  constexpr uint8_t NT_SW_DN_PIN = 								26;		// night shade down switch pin
  constexpr uint8_t DY_SW_UP_PIN = 								32;		// day shade up switch pin
  constexpr uint8_t DY_SW_DN_PIN = 								34;		// day shade down switch pin
  constexpr uint8_t NT_UP_MTR_PIN = 							33;		// night shade up output pin
  constexpr uint8_t NT_DN_MTR_PIN = 							27;		// night shade down output pin
  constexpr uint8_t DY_UP_MTR_PIN = 							22;		// day shade up output pin
  constexpr uint8_t DY_DN_MTR_PIN = 							21;		// day shade down output pin
  constexpr uint8_t NT_PWMA_PIN = 								16;		// TB6612FNG channel A output pin
  constexpr uint8_t DY_PWMB_PIN = 								17;		// TB6612FNG channel B output pin
  constexpr uint8_t TB6612FNG_STBY_PIN = 					 4;		// ESP32 pin connected to the TB6612FNG Stby pin
  constexpr uint8_t PWM_A_CH = 										 0;		// channel number for the TB6612FNG A motor speed
  constexpr uint8_t PWM_B_CH =										 1;		// channel number for the TB6612FNG B motor speed
	constexpr uint8_t SHADE_PWM_MAX = 						 255;		// max speed of the 

	constexpr uint8_t DEBOUNCE_TIME = 							20;		// swoych debounce milliseconds
	constexpr uint8_t AUTO_MAX_TIME = 						 250;		// max tie before a swich down transitions to not remaining in auto mode

#elif defined DRIVER_SHADES

#elif defined PASSENGER_SHADES

#else 
	#error "A shade controller must be defined.  Please do this in the 'shadeDefs.h' file befor before contnuing."
#endif

enum class SdDir {
	NONE,
	UP,
	DOWN
};

enum class SwId {
  DAY_UP,
  DAY_DOWN,
  NIGHT_UP,
  NIGHT_DOWN
};

enum class CmdSrc {
  NONE,
  PHY_SW,
  WEB_SW,
  NODE_RED_SW,
  TIMER,
  COUNT
};

constexpr uint8_t CLOSED 	= 0;
constexpr uint8_t OPEN 		= 1;

struct ShadeSwitch {
  SwId id;												// identifies the physical switch.
	uint8_t pin;										// is its GPIO.
  bool current = false;						// is the most recently accepted physical state.
  bool previous = false;					// is the accepted state before current
  time_t closedAt = 0;						// is the millis() timestamp when the current closure began.
};
using ShadeSwPtr = ShadeSwitch*;

struct SdRunTime {
  time_t dnRunT;
  time_t upRunT;
};
using RunTimePtr = SdRunTime*;

struct Shade {
	String name;
	uint8_t upMotorPin;										// Physical up motor GPIO
	uint8_t downMotorPin;									// Physical down motor GPIO
	RunTimePtr pbOnRunTimePtr;						// parking brake on MAX run time 
	RunTimePtr pbOffRunTimePtr;						// parking brake off MAX run time 
  ShadeSwPtr upSwitchPtr;               // Physical up switch
  ShadeSwPtr downSwitchPtr;             // Physical down switch
	uint8_t pwmPin;                       // TB6612FNG PWM output pin
	uint8_t pwmChannel;                   // TB6612FNG PWM channel
	String fname;													// file name for saving needed Shade data
	CmdSrc cmdSource = CmdSrc::NONE;			// what device originated the last shade command
  time_t runStartT = 0;									// when the current movement segment began
  time_t sdRunT = 0;										// accumulated/current position of the shade, measured from fully UP
	time_t runStartSdRunT = 0;						// sdRunT when current movement segment began
	bool isAutoOn = false;								// true means the shade will go all the way up or down before stopping, false means it will stop when the command is removed
	bool moving = false;									// Current shade in motion state
	SdDir direction = SdDir::NONE;				// if in motion is it going up or down. If not in motion then NONE
};

struct ExtPtnrState {
    bool valid = false;

    bool parkingBrake;

    int dayPercentDown;
    SdDir dayDirection;
    bool dayUpEnabled;
    bool dayDownEnabled;

    int nightPercentDown;
    SdDir nightDirection;
    bool nightUpEnabled;
    bool nightDownEnabled;

    bool operator!=(const ExtPtnrState& other) const {
        return parkingBrake     != other.parkingBrake
            || dayPercentDown   != other.dayPercentDown
            || dayDirection     != other.dayDirection
            || dayUpEnabled     != other.dayUpEnabled
            || dayDownEnabled   != other.dayDownEnabled
            || nightPercentDown != other.nightPercentDown
            || nightDirection   != other.nightDirection
            || nightUpEnabled   != other.nightUpEnabled
            || nightDownEnabled != other.nightDownEnabled;
    }
};

class ShadeOps {
public:
	bool setup() ;
	bool startup();
	bool evtLoop();

private:
	String _sdDataTop;
	ShadeSwitch _dySdSwUp = {SwId::DAY_UP, DY_SW_UP_PIN};
	ShadeSwitch _dySdSwDn = {SwId::DAY_DOWN, DY_SW_DN_PIN};
	ShadeSwitch _ntSdSwUp = {SwId::NIGHT_UP, NT_SW_UP_PIN};
	ShadeSwitch _ntSdSwDn = {SwId::NIGHT_DOWN, NT_SW_DN_PIN};

	SdRunTime _dyPkBkOn = {19000, 18000};
	SdRunTime _dyPkBkOff = {4600, 4500};
	SdRunTime _ntPkBkOn = {19000, 18000};
	SdRunTime _ntPkBkOff = {4600, 4500};

	Shade _dySd = {"Day", DY_UP_MTR_PIN, DY_DN_MTR_PIN, &_dyPkBkOn, &_dyPkBkOff, &_dySdSwUp, &_dySdSwDn, DY_PWMB_PIN, PWM_B_CH};
	Shade _ntSd = {"Night", NT_UP_MTR_PIN, NT_DN_MTR_PIN, &_ntPkBkOn, &_ntPkBkOff, &_ntSdSwUp, &_ntSdSwDn, NT_PWMA_PIN, PWM_A_CH};

	time_t _sdSwAutoTransT = AUTO_MAX_TIME;
	uint8_t _debounceT = DEBOUNCE_TIME;
	uint8_t _TB6612StdbyPin = TB6612FNG_STBY_PIN;
	const int _PWM_FREQ = 5000;                                          // what is the PMW frequency set to on the TB6612FNG
	const int _PWM_RESOLUTION = 10;                                      // what PMW resolution is the TB6612FNG set 
	ExtPtnrState _lastExtPtnrState;																				// date=a last sent to our partners (Node-Red/Web)

	void checkShades();
	void initPins();
	void ckPhySdSwStates();
	void ckPhySwState(ShadeSwitch& sw);
	void checkForAutoTransition(Shade& shade);
//	void checkForAutoTransition(Shade& shade, ShadeSwitch& sw);
	void processSwitchActions();
	void doShadeSwStateChg(Shade& shade);
	void updateSdRunT(Shade& shade);
	bool isTimeToStop(Shade& shade);
	bool isTimeLeft(Shade& shade, SdDir dir);
	void turnOff(Shade& shade, CmdSrc cmdSource);
	void turnOn(Shade& shade, SdDir dir, CmdSrc cmdSource);
	bool shouldSdBeTurnedOn(Shade& shade, SdDir dir, CmdSrc cmdSource);
	bool reverseShade(Shade& shade, SdDir newDir, CmdSrc cmdSource);
	bool anyShadeMoving();
	String cmdSrcToText(CmdSrc src);
	String sdDirToText(SdDir dir);
	time_t getSdPctDown(Shade& shade);
	String buildExtPtnrStateJson(const ExtPtnrState& state);
	void getExtPtnrState(ExtPtnrState& state);
	void checkExtPtnrState();
	void saveSdRunT(Shade& shade);
	void loadSdRunT(Shade& shade);
    
};

extern ShadeOps shadeOps;