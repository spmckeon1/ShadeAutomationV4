
#include <ArduinoTrace.h>

#include <ei_appFramework.h>
#include <ei_logging.h>
#include <ei_mqtt.h>
#include <ei_storage.h>

#include "shadeOps.h"

#include <ei_utilities.h>

#include "appEvents.h"
#include "ctrlOps.h"
#include "shadeDefs.h"

ShadeOps shadeOps;

inline constexpr const char SHADE_OPS[] = "SHADE_OPS"; 

/*---- PERFORM ALL REQUIRED SETUP ACTIONS  ----*/

bool ShadeOps::setup() {
	shadeOps.initPins();
  _config.cfgFname = appDirs.appData + "/shadeCfg.json";
  _sdDataTop = appIDs.mqttTopic;
  _dySd.fname = appDirs.appData + "/dySd_sdRunT.json";
  _ntSd.fname = appDirs.appData + "/ntSd_sdRunT.json";
  if (!loadSdVarFromDisk()) {
    return false;
  }
  loadSdRunT(_dySd);
  loadSdRunT(_ntSd);
  eiEvents.on(EiEvent::MqttConnected, shadeOpsMqttConnected);
  eiEvents.on(EiEvent::MqttDisconnected, shadeOpsMqttDisconnected);
	logInfo(LS, SHADE_OPS, "SHADE_OPS setup() has completed");
  appEvents.on(AppEvent::ParkingBrakeChanged, shadeOpsParkingBrakeChanged);
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
  checkExtPtnrState(false);
	return true;
}

/*-----  DETECTION OF THE PARKING BRAKE CHANDING STATE  -----*/

void shadeOpsParkingBrakeChanged() {
  shadeOps.parkingBrakeChanged();
}

/*-----  CALL THE NEEDED ACTIONS ON PARKING BRAKE CHANGED  -----*/

/*-----  CALL THE NEEDED ACTIONS ON PARKING BRAKE CHANGED  -----*/

void ShadeOps::parkingBrakeChanged() {
  if (!ctrlOps.isParkingBrakeOn()) {    // Corrective action only when the brake is released.
    handleBrakeRelease(_dySd);          // Check each shade this controller manages.
    handleBrakeRelease(_ntSd);
  }
  checkExtPtnrState(true);              // Publish current state for external partners.
}
/*-----  ACTIONS TO TAKE ON MQTT CONNECT -----*/

void shadeOpsMqttConnected() {
  shadeOps.mqttConnected();
  logging.msg(__FILE__, FN, LN, T::EVENT, L::INFO, ET::USER, "ShadeOps received MqttConnected");
}

/*-----  HANDLE ALL ACTIONS WHEN CONNECTED TO MQTT -----*/

void ShadeOps::mqttConnected() {
  ExtPtnrState state;
  getExtPtnrState(state);
  DUMP(_sdDataTop);
  String json = buildJsonShadeState(state);
  if (mqtt.mqttPubMsg(_sdDataTop, QOS0, FORGET, json, LN)) {
    _lastExtPtnrState = state;
    _lastExtPtnrState.valid = true;
  } else {
    _lastExtPtnrState.valid = false;
    logError(
      LS,
      SHADE_OPS,
      "Unable to publish shade state after MQTT connection."
    );
  }
}

/*-----  ACTIONS TO TAKE ON MQTT DISCONNECT -----*/

void shadeOpsMqttDisconnected() {
    logging.msg(__FILE__, FN, LN,
                T::EVENT,
                L::INFO,
                ET::USER,
                "ShadeOps received MqttDisconnected");
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

/*---- ITIALIZE ALL THE SHADE PINS  ----*/

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

bool ShadeOps::loadSdVarFromDisk() {
  JsonDocument doc;

  // Build the current defaults.
  JsonObject day = doc["day"].to<JsonObject>();

  JsonObject dayPbOn = day["parkingBrakeOn"].to<JsonObject>();
  dayPbOn["upRunTime"] = _dyPkBkOn.upRunT;
  dayPbOn["downRunTime"] = _dyPkBkOn.dnRunT;

  JsonObject dayPbOff = day["parkingBrakeOff"].to<JsonObject>();
  dayPbOff["upRunTime"] = _dyPkBkOff.upRunT;
  dayPbOff["downRunTime"] = _dyPkBkOff.dnRunT;

  JsonObject night = doc["night"].to<JsonObject>();

  JsonObject nightPbOn = night["parkingBrakeOn"].to<JsonObject>();
  nightPbOn["upRunTime"] = _ntPkBkOn.upRunT;
  nightPbOn["downRunTime"] = _ntPkBkOn.dnRunT;

  JsonObject nightPbOff = night["parkingBrakeOff"].to<JsonObject>();
  nightPbOff["upRunTime"] = _ntPkBkOff.upRunT;
  nightPbOff["downRunTime"] = _ntPkBkOff.dnRunT;

  doc["debounceTime"] = _debounceT;
  doc["autoTransitionTime"] = _sdSwAutoTransT;

  // Create the file if it does not already exist.
  if (!storage.ensureFileExistsBool(_config.cfgFname, doc, LN)) {
    return false;
  }

  // Read the actual configuration from disk.
  doc.clear();

  if (!storage.readJsonFile(_config.cfgFname.c_str(), doc, LN)) {
    logError(LS, SHADE_OPS,
             "Unable to read ShadeOps configuration from disk.");
    return false;
  }

  // Apply the configuration.
  _dyPkBkOn.upRunT  = doc["day"]["parkingBrakeOn"]["upRunTime"];
  _dyPkBkOn.dnRunT  = doc["day"]["parkingBrakeOn"]["downRunTime"];

  _dyPkBkOff.upRunT = doc["day"]["parkingBrakeOff"]["upRunTime"];
  _dyPkBkOff.dnRunT = doc["day"]["parkingBrakeOff"]["downRunTime"];

  _ntPkBkOn.upRunT  = doc["night"]["parkingBrakeOn"]["upRunTime"];
  _ntPkBkOn.dnRunT  = doc["night"]["parkingBrakeOn"]["downRunTime"];

  _ntPkBkOff.upRunT = doc["night"]["parkingBrakeOff"]["upRunTime"];
  _ntPkBkOff.dnRunT = doc["night"]["parkingBrakeOff"]["downRunTime"];

  _debounceT        = doc["debounceTime"];
  _sdSwAutoTransT   = doc["autoTransitionTime"];

  return true;
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
	}
}

/*---- CHECK SHADES FOR AUTO TRANSITION  ----*/

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
/*---- PRROCESS ANY AND ALL PHYSICAL SWITCH CHANGES  ----*/

void ShadeOps::processSwitchActions() {
  if (_dySdSwUp.current != _dySdSwUp.previous) {
    doShadeSwStateChg(_dySd);
  }
  if (_dySdSwDn.current != _dySdSwDn.previous) {
    doShadeSwStateChg(_dySd);
  }
  checkForAutoTransition(_dySd);

  if (_ntSdSwUp.current != _ntSdSwUp.previous) {
    doShadeSwStateChg(_ntSd);
  }
  if (_ntSdSwDn.current != _ntSdSwDn.previous) {
    doShadeSwStateChg(_ntSd);
  }
  checkForAutoTransition(_ntSd);
}

/*---- PRROCESS ANY NEEDED SHADE ACTIONS  ----*/

void ShadeOps::doShadeSwStateChg(Shade& shade) {

  // UP switch closed
  if (shade.upSwitchPtr->current && !shade.upSwitchPtr->previous) {
    executeShadeCommand(shade, SdDir::UP, CmdSrc::PHY_SW);
    return;
  }

  // DOWN switch closed
  if (shade.downSwitchPtr->current && !shade.downSwitchPtr->previous) {
    executeShadeCommand(shade, SdDir::DOWN, CmdSrc::PHY_SW);
    return;  
  }

  // A switch was released.
  if (!shade.upSwitchPtr->current || !shade.downSwitchPtr->current) {
    if (!shade.isAutoOn) {
      turnOff(shade, CmdSrc::PHY_SW);
    }
  }
}

/*----  EXECUTE A SHADE COMMAND  ----*/

void ShadeOps::executeShadeCommand(Shade& shade, SdDir dir, CmdSrc cmdSource) {
  if (shade.moving) {
    if (shade.direction == dir)
      turnOff(shade, cmdSource);
    else
      turnOn(shade, dir, cmdSource);
    return;
  }

  turnOn(shade, dir, cmdSource);
}/*---- KEEPS THE SHADE RUN TIMES CURRET AND UP TO DATE  ----*/

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
  }
  else if (shade.direction == SdDir::UP) {
    shade.sdRunT = MATH::suli(shade.runStartSdRunT, elapsedT);
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

  if (dir == SdDir::UP) {
   if (shade.brkRelUp)                                     // Brake-release correction: 
      return shade.sdRunT > shade.pbOffRunTimePtr->dnRunT;  // raise only to the parking-brake-OFF max-down position.
    return shade.sdRunT > 0;                                // Normal UP operation: continue until fully UP.
  }
  if (dir == SdDir::DOWN) {
    return shade.sdRunT < runTimePtr->dnRunT;
  }

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
  shade.brkRelUp = false;
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
    case CmdSrc::PARK_BRAKE:  return "PARK_BRAKE";
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

String ShadeOps::buildJsonShadeState(const ExtPtnrState& state) {
  JsonDocument data;

  data["schema"] = "shadeState.v1";

  JsonObject day = data["day"].to<JsonObject>();
  day["percentDown"] = state.dayPercentDown;
  day["direction"] = sdDirToText(state.dayDirection);
  day["upEnabled"] = state.dayUpEnabled;
  day["downEnabled"] = state.dayDownEnabled;

  JsonObject night = data["night"].to<JsonObject>();
  night["percentDown"] = state.nightPercentDown;
  night["direction"] = sdDirToText(state.nightDirection);
  night["upEnabled"] = state.nightUpEnabled;
  night["downEnabled"] = state.nightDownEnabled;

  return appFramework.buildJsonAppMqttMsg(String(SD_CMD_ROUTE), "STATE", data.as<JsonObjectConst>());
}

/*----  BUILD THE EXTERNAL PARTNERS STATE STRUCT ----*/

void ShadeOps::getExtPtnrState(ExtPtnrState& state) {

  RunTimePtr dayRunTimePtr = ctrlOps.isParkingBrakeOn() ? _dySd.pbOnRunTimePtr : _dySd.pbOffRunTimePtr;

  RunTimePtr nightRunTimePtr = ctrlOps.isParkingBrakeOn()  ? _ntSd.pbOnRunTimePtr : _ntSd.pbOffRunTimePtr;

  state.dayPercentDown = getSdPctDown(_dySd);
  state.dayDirection = _dySd.direction;
  state.dayUpEnabled = _dySd.sdRunT > 0;
  state.dayDownEnabled = _dySd.sdRunT < dayRunTimePtr->dnRunT;

  state.nightPercentDown = getSdPctDown(_ntSd);
  state.nightDirection = _ntSd.direction;
  state.nightUpEnabled = _ntSd.sdRunT > 0;
  state.nightDownEnabled = _ntSd.sdRunT < nightRunTimePtr->dnRunT;
}
/*----  CHECK AND IF NEEDED SEND THE SHADE STATE ----*/

void ShadeOps::checkExtPtnrState(bool force) {
  ExtPtnrState currentState;
  getExtPtnrState(currentState);
  if (force || !_lastExtPtnrState.valid || currentState != _lastExtPtnrState) {\
    String json = buildJsonShadeState(currentState);\
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
    if (!storage.readJsonFile(shade.fname.c_str(), doc, LN)) {
      logError(LS, ET::MQTT, "Failed to read " + shade.name + "'s sdRunT (" + String(shade.sdRunT) + ") data to disk.");
    }
    else {
      shade.sdRunT = doc["sdRunT"];
    }
}

/*----  PROCESS AN INCOMING MESSAGE  ----*/

void ShadeOps::processMsg(const JsonDocument& doc) {
  const char* command = doc["command"] | "";
  DUMP(command);
  if (strcmp(command, "EXEC") == 0) {                 // EXEC — execute a requested shade operation.
    const char* shade = doc["data"]["shade"] | "";
    const char* action = doc["data"]["action"] | "";
    if (strcmp(shade, "day") == 0) {
      if (strcmp(action, "UP") == 0) {
        executeShadeCommand(_dySd, SdDir::UP, CmdSrc::NODE_RED_SW);
        return;
      }
      if (strcmp(action, "DOWN") == 0) {
        executeShadeCommand(_dySd, SdDir::DOWN, CmdSrc::NODE_RED_SW);
        return;
      }
      if (strcmp(action, "STOP") == 0) {
        turnOff(_dySd, CmdSrc::NODE_RED_SW);
        return;
      }
    }
    if (strcmp(shade, "night") == 0) {
      if (strcmp(action, "UP") == 0) {
        executeShadeCommand(_ntSd, SdDir::UP, CmdSrc::NODE_RED_SW);
        return;
      }
      if (strcmp(action, "DOWN") == 0) {
        executeShadeCommand(_ntSd, SdDir::DOWN, CmdSrc::NODE_RED_SW);
        return;
      }
      if (strcmp(action, "STOP") == 0) {
        turnOff(_ntSd, CmdSrc::NODE_RED_SW);
        return;
      }
    }
    logError(LS, SHADE_OPS, "Unknown shade/action command");
    return;
  }
  if (strcmp(command, "REFRESH") == 0) {              // REFRESH — return the current controller state.
    checkExtPtnrState(true);
    return;
  }
  if (strcmp(command, "REQUEST") == 0) {              // REQUEST — return the current configuration.
    sendConfig();
    return;
  }
  if (strcmp(command, "CONFIG") == 0) {               // CONFIG — apply validated configuration rows from Node-RED.
    processCfgUpdate(doc);
    return;
  }
  logError(LS, SHADE_OPS, "Unexpected command received: " + String(command));
}


/*----  EXECUTE THE REQUIRED ACTION WHEN TH EOARKING BRAKE IS RELEASED  ----*/

void ShadeOps::handleBrakeRelease(Shade& shade) {
  time_t maxDown = shade.pbOffRunTimePtr->dnRunT;
  if (shade.sdRunT <= maxDown)                                // Shade is already within the new allowed range.
    return;
                                                              // Shade is too far down.  Mark this movement as a brake-release correction.
  shade.brkRelUp = true;
  if (shade.moving && shade.direction == SdDir::UP)           // Already moving UP: let the new isTimeLeft() rule stop it at maxDown.
    return;  
  executeShadeCommand(shade, SdDir::UP, CmdSrc::PARK_BRAKE);  // Stopped or moving DOWN: command it UP.
}

/*----  ASSEMBLE AND SEND CINFIG DATA TO NODE RED  ----*/

void ShadeOps::sendConfig() {
  TRACE();
  JsonDocument data;

  JsonObject day = data["day"].to<JsonObject>();
  JsonObject dayPbOn = day["parkingBrakeOn"].to<JsonObject>();
  dayPbOn["upRunTime"] = _dyPkBkOn.upRunT;
  dayPbOn["downRunTime"] = _dyPkBkOn.dnRunT;

  JsonObject dayPbOff = day["parkingBrakeOff"].to<JsonObject>();
  dayPbOff["upRunTime"] = _dyPkBkOff.upRunT;
  dayPbOff["downRunTime"] = _dyPkBkOff.dnRunT;

  JsonObject night = data["night"].to<JsonObject>();
  JsonObject nightPbOn = night["parkingBrakeOn"].to<JsonObject>();
  nightPbOn["upRunTime"] = _ntPkBkOn.upRunT;
  nightPbOn["downRunTime"] = _ntPkBkOn.dnRunT;

  JsonObject nightPbOff = night["parkingBrakeOff"].to<JsonObject>();
  nightPbOff["upRunTime"] = _ntPkBkOff.upRunT;
  nightPbOff["downRunTime"] = _ntPkBkOff.dnRunT;

  data["debounceTime"] = _debounceT;
  data["autoTransitionTime"] = _sdSwAutoTransT;

  String json = appFramework.buildJsonAppMqttMsg(
    String(SD_CFG_ROUTE),
    "CONFIG",
    data.as<JsonObjectConst>()
  );
  DUMP(json);
  DUMP(_sdDataTop);
  mqtt.mqttPubMsg(String(_sdDataTop) + "/shadeOps/cfg", QOS0, FORGET, json, LN);
}

/*----  PROCESS A CONFIGURATION UPDATE MSG  ----*/

bool ShadeOps::processCfgUpdate(const JsonDocument& doc) {
  JsonArrayConst rows = doc["data"].as<JsonArrayConst>();

  if (rows.isNull()) {
    logError(LS, SHADE_OPS, "CONFIG command did not contain a configuration-row array.");
    return false;
  }

  for (JsonObjectConst row : rows) {
    const uint8_t id = row["id"] | 0;
    const time_t value = row["value"].as<time_t>();

    switch (id) {
      case 1:  _dyPkBkOn.upRunT  = value; break;
      case 2:  _dyPkBkOn.dnRunT  = value; break;
      case 3:  _dyPkBkOff.upRunT = value; break;
      case 4:  _dyPkBkOff.dnRunT = value; break;
      case 5:  _ntPkBkOn.upRunT  = value; break;
      case 6:  _ntPkBkOn.dnRunT  = value; break;
      case 7:  _ntPkBkOff.upRunT = value; break;
      case 8:  _ntPkBkOff.dnRunT = value; break;
      case 9:  _debounceT        = static_cast<uint8_t>(value); break;
      case 10: _sdSwAutoTransT   = value; break;
      default:
        logError(LS, SHADE_OPS, "CONFIG command contained an unknown row id: " + String(id));
        return false;
    }
  }
  logInfo(LS, SHADE_OPS, "Applied configuration: "
      "dyPkBkOn.upRunT=" + String(_dyPkBkOn.upRunT) +
      ", dyPkBkOn.dnRunT=" + String(_dyPkBkOn.dnRunT) +
      ", dyPkBkOff.upRunT=" + String(_dyPkBkOff.upRunT) +
      ", dyPkBkOff.dnRunT=" + String(_dyPkBkOff.dnRunT) +
      ", ntPkBkOn.upRunT=" + String(_ntPkBkOn.upRunT) +
      ", ntPkBkOn.dnRunT=" + String(_ntPkBkOn.dnRunT) +
      ", ntPkBkOff.upRunT=" + String(_ntPkBkOff.upRunT) +
      ", ntPkBkOff.dnRunT=" + String(_ntPkBkOff.dnRunT) +
      ", debounceT=" + String(_debounceT) +
      ", sdSwAutoTransT=" + String(_sdSwAutoTransT)
  );
  if(!saveConfig()) {
    sendCfgStateToNR(false);
    return false;
  }
  sendCfgStateToNR(true);
  return true;
}

/*----  SAVE THE SHADE CONFIG DATA TO DISK  ----*/

bool ShadeOps::sendCfgStateToNR(bool state) {
  JsonDocument data;
  data["success"] = state;
  String json = appFramework.buildJsonAppMqttMsg(
      String(SD_CFG_STATE_ROUTE),
      "CONFIG_RESULT",
      data.as<JsonObjectConst>()
  );
  mqtt.mqttPubMsg(
      String(_sdDataTop) + "/shadeOps/cfg",
      QOS0,
      FORGET,
      json,
      LN
  );
  return true;
}

/*----  SAVE THE SHADE CONFIG DATA TO DISK  ----*/

bool ShadeOps::saveConfig() {
  JsonDocument doc;

  JsonObject day = doc["day"].to<JsonObject>();

  JsonObject dayPbOn = day["parkingBrakeOn"].to<JsonObject>();
  dayPbOn["upRunTime"] = _dyPkBkOn.upRunT;
  dayPbOn["downRunTime"] = _dyPkBkOn.dnRunT;

  JsonObject dayPbOff = day["parkingBrakeOff"].to<JsonObject>();
  dayPbOff["upRunTime"] = _dyPkBkOff.upRunT;
  dayPbOff["downRunTime"] = _dyPkBkOff.dnRunT;

  JsonObject night = doc["night"].to<JsonObject>();

  JsonObject nightPbOn = night["parkingBrakeOn"].to<JsonObject>();
  nightPbOn["upRunTime"] = _ntPkBkOn.upRunT;
  nightPbOn["downRunTime"] = _ntPkBkOn.dnRunT;

  JsonObject nightPbOff = night["parkingBrakeOff"].to<JsonObject>();
  nightPbOff["upRunTime"] = _ntPkBkOff.upRunT;
  nightPbOff["downRunTime"] = _ntPkBkOff.dnRunT;

  doc["debounceTime"] = _debounceT;
  doc["autoTransitionTime"] = _sdSwAutoTransT;

  if (storage.writeJsonFile(_config.cfgFname.c_str(), doc, LN) != Storage::WriteResult::Success) {
    logError(LS, SHADE_OPS, "Unable to write ShadeOps configuration to disk.");
    return false;
  }

  return true;
}

