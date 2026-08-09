
#include "shadeAutomation.h"
#include "shadeAutomationV4.h"


/*---- CALL ALL SETUP ITEMS HERE  ----*/

void setup() {
  shadeAuto.startup();
}

/*---- RUN THE MAIN LOOP  ----*/

void loop() {
  shadeAuto.evtLoop();
}

