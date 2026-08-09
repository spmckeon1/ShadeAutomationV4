#include "shadeAutomationV4.h"


ShadeAutomationV4 shadeAuto;


/*---- PERFORM ALL NEEDED STTARTUP ACTIVITIES ----*/

bool ShadeAutomationV4::startup() {
    return true;
}


/*---- THE ShadeAutomationV4 CLASS EVENT LOOP  ----*/

void ShadeAutomationV4::evtLoop() {
}


/*---- CALL ALL SETUP ITEMS HERE ----*/

void setup() {
    shadeAuto.startup();
}


/*---- RUN THE MAIN LOOP ----*/

void loop() {
    shadeAuto.evtLoop();
}