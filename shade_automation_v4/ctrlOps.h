#pragma once

#include <Arduino.h>

 #include "shadeDefs.h"

constexpr uint8_t PK_BK_ON                    =  0;
constexpr uint8_t PK_BK_OFF                   =  1;

#ifdef WINDSHIELD_SHADES
  constexpr uint8_t PK_BK_PIN                 = 35;   // Parking brake

#elif defined DRIVER_SHADES

#elif defined PASSENGER_SHADES

#else 
	#error "A shade controller must be defined.  Please do this in the 'shadeDefs.h' file befor before contnuing."
#endif


class CtrlOps
{
public:
  bool setup();
  bool startup();
  void evtLoop();

  bool isParkingBrakeOn() const;
  float temperature();

private:
  bool _pkBkState = false;
  bool _pkBkChanged = false;

  bool isPkBkOn();

};

extern CtrlOps ctrlOps;
extern void parkingBrakeChanged();

