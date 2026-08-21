
#include <ArduinoTrace.h>

#include <ei_appPolicy.h>
#include <ei_logging.h>
#include <ei_mqtt.h>
#include <ei_storage.h>

#include "shadeOps.h"

#include <ei_utilities.h>
#include "ctrlOps.h"

ShadeOps shadeOps;

inline constexpr const char SHADE_OPS[] = "SHADE_OPS";

/*---- PERFORM ALL REQUIRED SETUP ACTIONS  ----*/

bool ShadeOps::setup() {
	shadeOps.initPins();
  _sdDataTop = String(appIDs.sourceId) + "/to/nr/sd/data";
  _dySd.fname = appDirs.appData + "/dySd_sdRunT.json";
  _ntSd.fname = appDirs.appData + "/ntSd_sdRunT.json";
  loadSdRunT(_dySd);
  loadSdRunT(_ntSd);
	logInfo(LS, SHADE_OPS, "SHADE_OPS setup() has completed");
	return true;
}

/*---- EXECUTE THE SHADEOPS EVENT LOOP  ----*/

bool ShadeOps::startup() {
	logInfo(LS, SHADE_OPS, "SHADE_OPS startup() has completed");
	return true;
}

bool ShadeOps::evtLoop() {
	ckPhySdSwStates();						  // check the sates ofthe physical switches
	processSwitchActions();				  // process and physical switch changes
  checkShades();                  // manage shades currently in motion
  checkExtPtnrState();
	return true;
}

/*---- CHECK THE SHADES FOR NEEDED ACTIONS  ----*/

void ShadeOps::checkShades() {
  updateSdRunT(_dySd);

  if (isTimeToStop(_dySd))
    turnOff(_dySd, _dySd.cmdSource);

  updateSdRunT(_ntSd);

  if (isTimeToStop(_ntSd))
    turnOff(_ntSd, _ntSd.cmdSource);
}

/*---- PINITIALIZE ALL THE SHADE PINS  ----*/

void ShadeOps::initPins() {
  // Physical shade switches
  pinMode(NT_SW_UP_PIN, INPUT);
  pinMode(NT_SW_DN_PIN, INPUT);
  pinMode(DY_SW_UP_PIN, INPUT);
  pinMode(DY_SW_DN_PIN, INPUT);

  // Shade motor direction outputs
  pinMode(NT_UP_MTR_PIN, OUTPUT);
  pinMode(NT_DN_MTR_PIN, OUTPUT);
  pinMode(DY_UP_MTR_PIN, OUTPUT);
  pinMode(DY_DN_MTR_PIN, OUTPUT);

  // Ensure all motor direction outputs are OFF
  digitalWrite(NT_UP_MTR_PIN, LOW);
  digitalWrite(NT_DN_MTR_PIN, LOW);
  digitalWrite(DY_UP_MTR_PIN, LOW);
  digitalWrite(DY_DN_MTR_PIN, LOW);

  // TB6612FNG standby
  pinMode(TB6612FNG_STBY_PIN, OUTPUT);
  digitalWrite(TB6612FNG_STBY_PIN, LOW);

  // PWM outputs
  ledcAttachChannel(NT_PWMA_PIN, _PWM_FREQ, _PWM_RESOLUTION, PWM_A_CH);
  ledcAttachChannel(DY_PWMB_PIN, _PWM_FREQ, _PWM_RESOLUTION, PWM_B_CH);

  // Ensure both motor channels have no drive
  ledcWrite(NT_PWMA_PIN, 0);
  ledcWrite(DY_PWMB_PIN, 0);
}

/*---- CHECK THE PHYSCAL SHADE SWITCH STATES  ----*/

void ShadeOps::ckPhySdSwStates() {
	ckPhySwState(_dySdSwUp);
	ckPhySwState(_dySdSwDn);
	ckPhySwState(_ntSdSwUp);
	ckPhySwState(_ntSdSwDn);
}

/*---- CHECK A PHYSICAL SWITCH STATE  ----*/

void ShadeOps::ckPhySwState(ShadeSwitch& sw) {
	sw.previous = sw.current;
  sw.current = GPIO::readPin(sw.pin, sw.previous, _debounceT);

  if (sw.current && !sw.previous)	{																// if the switch has just been closed
    sw.closedAt = millis();
		DUMP(sw.pin);
	}
}

/*---- CHECK SHADE S FOR ISAUTOON TRANSITION  ----*/

void ShadeOps::checkForAutoTransition(Shade& shade) {
  if (!shade.isAutoOn)
    return;

  ShadeSwitch& sw =
      (shade.direction == SdDir::DOWN)
          ? *shade.downSwitchPtr
          : *shade.upSwitchPtr;

  time_t elapsedT = MATH::suli(millis(), sw.closedAt);

  if (elapsedT >= _sdSwAutoTransT) {
    shade.isAutoOn = false;
//    DUMP("Auto transition: " + shade.name);
  }
}

/*
void ShadeOps::checkForAutoTransition(Shade& shade, ShadeSwitch& sw) {
  if (sw.current && sw.current == sw.previous) {
    if (MATH::suli(millis(), sw.closedAt) > _sdSwAutoTransT)
      shade.isAutoOn = true;
  }
}
*/
/*---- PRROCESS ANY AND ALL CPHYSICAL SWITCH CHANGES  ----*/

void ShadeOps::processSwitchActions() {
  if (_dySdSwUp.current != _dySdSwUp.previous) {
    DUMP(_dySdSwUp.pin);
    doShadeSwStateChg(_dySd);
  }
  if (_dySdSwDn.current != _dySdSwDn.previous) {
    DUMP(_dySdSwDn.pin);
    doShadeSwStateChg(_dySd);
  }
  checkForAutoTransition(_dySd);

  if (_ntSdSwUp.current != _ntSdSwUp.previous) {
    DUMP(_ntSdSwUp.pin);
    doShadeSwStateChg(_ntSd);
  }
  if (_ntSdSwDn.current != _ntSdSwDn.previous) {
    DUMP(_ntSdSwDn.pin);
    doShadeSwStateChg(_ntSd);
  }
  checkForAutoTransition(_ntSd);
}

/*---- PRROCESS ANY NEEDED SHADE ACTIONS  ----*/

void ShadeOps::doShadeSwStateChg(Shade& shade) {

  // UP switch closed
  if (shade.upSwitchPtr->current && !shade.upSwitchPtr->previous) {

    if (shade.moving) {
      if (shade.direction == SdDir::UP)
        turnOff(shade, CmdSrc::PHY_SW);
      else
        turnOn(shade, SdDir::UP, CmdSrc::PHY_SW);

      return;
    }

    turnOn(shade, SdDir::UP, CmdSrc::PHY_SW);
    return;
  }

  // DOWN switch closed
  if (shade.downSwitchPtr->current && !shade.downSwitchPtr->previous) {

    if (shade.moving) {
      if (shade.direction == SdDir::DOWN)
        turnOff(shade, CmdSrc::PHY_SW);
      else
        turnOn(shade, SdDir::DOWN, CmdSrc::PHY_SW);

      return;
    }

    turnOn(shade, SdDir::DOWN, CmdSrc::PHY_SW);
    return;
  }

  // A switch was released.
  if (!shade.upSwitchPtr->current || !shade.downSwitchPtr->current) {
    if (!shade.isAutoOn) {
      turnOff(shade, CmdSrc::PHY_SW);
      DUMP("Switch released: " + shade.name +
           "; isAutoOn = " + String(shade.isAutoOn));
    }
  }
}

/*---- KEEPS THE SHADE RUN TIMES CURRET AND UP TO DATE  ----*/

void ShadeOps::updateSdRunT(Shade& shade) {
  if (!shade.moving)
    return;

  time_t nowT = millis();
  time_t elapsedT = MATH::suli(nowT, shade.runStartT);

  if (shade.direction == SdDir::DOWN) {
    shade.sdRunT = shade.runStartSdRunT + elapsedT;

    // sdRunT represents physical shade position.
    // Never allow it to exceed the true full-DOWN position.
    if (shade.sdRunT > shade.pbOnRunTimePtr->dnRunT)
      shade.sdRunT = shade.pbOnRunTimePtr->dnRunT;


//      DUMP("Going DOWN - " + shade.name + "; sdRunT = " + String(static_cast<int>(shade.sdRunT)) + "; %down = " + String(static_cast<int>(getSdPctDown(shade))) + "%");

//    DUMP("Going DOWN - " + shade.name + "; " + String(static_cast<int>(shade.sdRunT)));
  }
  else if (shade.direction == SdDir::UP) {
    shade.sdRunT = MATH::suli(shade.runStartSdRunT, elapsedT);

//    DUMP("Going UP - " + shade.name + "; " + String(static_cast<int>(shade.sdRunT)));
  }
}

/*---- IS IT TIME TO STOP THE SHADE  ----*/

bool ShadeOps::isTimeToStop(Shade& shade) {

  if (!shade.moving)
    return false;

  if (isTimeLeft(shade, shade.direction))
    return false;

  // Run time has expired.
  // A physical switch still being held overrides the timer.
  ShadeSwitch& sw = (shade.direction == SdDir::DOWN) ? *shade.downSwitchPtr : *shade.upSwitchPtr;
  if (sw.current)
    return false;
  shade.cmdSource = CmdSrc::TIMER;
  return true;
}
/*---- IS THE REQUESTED DIRECTION AVAILABLE OR IS THE SHADE FULLY MOVED IN THIS DIRECTION ALREADY  ----*/

bool ShadeOps::isTimeLeft(Shade& shade, SdDir dir) {
  RunTimePtr runTimePtr = ctrlOps.isParkingBrakeOn() ? shade.pbOnRunTimePtr : shade.pbOffRunTimePtr;

  if (dir == SdDir::UP)
    return shade.sdRunT > 0;

  if (dir == SdDir::DOWN)
    return shade.sdRunT < runTimePtr->dnRunT;

  return false;
}

/*----  SHOULD THE SHADE BE ALLOWED TO TURN ON AT THIS TIME  ----*/

bool ShadeOps::shouldSdBeTurnedOn(Shade& shade, SdDir dir, CmdSrc cmdSource) {
  reverseShade(shade, dir, cmdSource);

	if (shade.upSwitchPtr->current || shade.downSwitchPtr->current)	// Physical switch is being held — user gets what they requested.
  	return true;
  
  if (!isTimeLeft(shade, dir)) {															 // There is no time left in the requested direction.
   	logWarn(LS, SHADE_OPS, "Shade cannot be turned on; no time left in requested direction.");
	  return false;
  }

  return true;
}

/*----  TURN THE SHADE MOTOR OFF  ----*/

void ShadeOps::turnOff(Shade& shade, CmdSrc cmdSource) {
	shade.cmdSource = cmdSource;
  if (shade.direction == SdDir::UP)  														// Stop this shade's motor immediately.
    digitalWrite(shade.upMotorPin, LOW);
  else if (shade.direction == SdDir::DOWN)
    digitalWrite(shade.downMotorPin, LOW);
  ledcWrite(shade.pwmChannel, 0);        												// remove PWM drive
	updateSdRunT(shade);																					// Capture the final position while it was still marked moving.
  shade.moving = false;
  shade.direction = SdDir::NONE;
  if (!anyShadeMoving())																				// If no shade is moving, put the TB6612FNG into standby.
    digitalWrite(_TB6612StdbyPin, LOW);
  saveSdRunT(shade);
	logInfo(LS, SHADE_OPS, "The " + shade.name + " was turned off by " + cmdSrcToText(cmdSource));
}

/*----  TURN THE SHADE MOTOR ON  ----*/

void ShadeOps::turnOn(Shade& shade, SdDir dir, CmdSrc cmdSource) {
  if (!shouldSdBeTurnedOn(shade, dir, cmdSource))
    return;

  shade.runStartT = millis();
	shade.runStartSdRunT = shade.sdRunT;
	shade.direction = dir;
	shade.cmdSource = cmdSource;
  shade.isAutoOn = true;
	shade.moving = true;
	digitalWrite(TB6612FNG_STBY_PIN, HIGH);
	ledcWrite(shade.pwmChannel, SHADE_PWM_MAX);
	digitalWrite(shade.pwmPin, HIGH);
	logInfo(LS, SHADE_OPS, "The " + shade.name + " was turned on by " + cmdSrcToText(cmdSource));

}

/*----  CHECK TO SE IF THE MOTOR NEEDS TO BE REVERSED  ----*/

bool ShadeOps::reverseShade(Shade& shade, SdDir newDir, CmdSrc cmdSource) {
  if (!shade.moving || shade.direction == newDir)
    return false;

  turnOff(shade, cmdSource);
  delay(100);

  return true;
}

/*----  IS ANY SHADE IN MOTION  ----*/

bool ShadeOps::anyShadeMoving() {
  return _ntSd.moving || _dySd.moving;
}

/*----  CONVERT cmdRecource TO A STRING  ----*/

String ShadeOps::cmdSrcToText(CmdSrc src) {
  switch (src) {
    case CmdSrc::NONE:        return "NONE";
    case CmdSrc::PHY_SW:      return "PHY_SW";
    case CmdSrc::WEB_SW:      return "WEB_SW";
    case CmdSrc::NODE_RED_SW: return "NODE_RED_SW";
    case CmdSrc::TIMER:       return "TIMER";
    default: return "UNKNOWN CmdSrc: " + String(static_cast<int>(src));
  }
}

/*----  CONVERT cmdRecource TO A STRING  ----*/

String ShadeOps::sdDirToText(SdDir dir) {
    switch (dir) {
        case SdDir::NONE: return "NONE";
        case SdDir::UP:   return "UP";
        case SdDir::DOWN: return "DOWN";
        default:          return "UNKNOWN";
    }
}

/*----  SET THE SHADE PERCENT DOWN  ----*/

time_t ShadeOps::getSdPctDown(Shade& shade) {
  if (shade.pbOnRunTimePtr->dnRunT == 0)
    return 0;

  return (shade.sdRunT * 100) / shade.pbOnRunTimePtr->dnRunT;
}

/*----  BUILD THE MSG CONTETS GOING TO EXTERNAL CONTROL DEVICES ----*/

String ShadeOps::buildExtPtnrStateJson(const ExtPtnrState& state) {
  JsonDocument doc;

  doc["schema"] = "shadeState.v1";
  doc["parkingBrake"] = state.parkingBrake;

  JsonObject day = doc["day"].to<JsonObject>();
  day["percentDown"] = state.dayPercentDown;
  day["direction"] = sdDirToText(state.dayDirection);
  day["upEnabled"] = state.dayUpEnabled;
  day["downEnabled"] = state.dayDownEnabled;

  JsonObject night = doc["night"].to<JsonObject>();
  night["percentDown"] = state.nightPercentDown;
  night["direction"] = sdDirToText(state.nightDirection);
  night["upEnabled"] = state.nightUpEnabled;
  night["downEnabled"] = state.nightDownEnabled;

  String json;
  serializeJson(doc, json);

  return json;
}
/*----  BUILD THE EXTERNAL PARTNERS STATE STRUCT ----*/

void ShadeOps::getExtPtnrState(ExtPtnrState& state) {
  state.parkingBrake = ctrlOps.isParkingBrakeOn();

  RunTimePtr dayRunTimePtr =
    state.parkingBrake ? _dySd.pbOnRunTimePtr : _dySd.pbOffRunTimePtr;

  state.dayPercentDown = getSdPctDown(_dySd);
  state.dayDirection = _dySd.direction;
  state.dayUpEnabled = _dySd.sdRunT > 0;
  state.dayDownEnabled = _dySd.sdRunT < dayRunTimePtr->dnRunT;

  RunTimePtr nightRunTimePtr =
    state.parkingBrake ? _ntSd.pbOnRunTimePtr : _ntSd.pbOffRunTimePtr;

  state.nightPercentDown = getSdPctDown(_ntSd);
  state.nightDirection = _ntSd.direction;
  state.nightUpEnabled = _ntSd.sdRunT > 0;
  state.nightDownEnabled = _ntSd.sdRunT < nightRunTimePtr->dnRunT;
}

/*----  CHECK AND IS NEEDED SEND THE XETERNAL PARTNER STATE ----*/

void ShadeOps::checkExtPtnrState() {
  ExtPtnrState currentState;

  getExtPtnrState(currentState);

  if (!_lastExtPtnrState.valid || currentState != _lastExtPtnrState) {

    String json = buildExtPtnrStateJson(currentState);
    DUMP(_sdDataTop);
    mqtt.mqttPubMsg(_sdDataTop, QOS0, FORGET, json, LN);
    _lastExtPtnrState = currentState;
    _lastExtPtnrState.valid = true;
  }
}

/*----  SAVE THE A SHADES sdRunT TO DISK  ----*/

void ShadeOps::saveSdRunT(Shade& shade) {
  JsonDocument doc;
  doc["sdRunT"] = shade.sdRunT;

  if(storage.writeJsonFile(shade.fname.c_str(), doc, LN)  != Storage::WriteResult::Success) {
    logError(LS, ET::MQTT, "Unable to write " + shade.name + "'s sdRunT (" + String(shade.sdRunT) + ") data to disk.");
  }
}

/*----  READ A SHADES sdRunT FROM DISK  ----*/

void ShadeOps::loadSdRunT(Shade& shade) {
  JsonDocument doc;
  if(!storage.readJsonFile(shade.fname.c_str(), doc, LN)) {
    logError(LS, ET::MQTT, "Failed to read " + shade.name + "'s sdRunT (" + String(shade.sdRunT) + ") data to disk.");
  }
  else shade.sdRunT = doc["sdRunT"];
}



