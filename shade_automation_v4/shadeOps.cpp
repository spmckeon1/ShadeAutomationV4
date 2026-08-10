
#include "shadeOps.h"
#include "appEvents.h"
#include <ei_logging.h>

ShadeOps shadeOps;

inline constexpr const char SHADE_OPS[] = "SHADE_OPS";

bool ShadeOps::startup()
{
    appEvents.on(AppEvent::ParkingBrakeChanged, parkingBrakeChanged);
}

void ShadeOps::parkingBrakeChanged() {
    logInfo(LS, SHADE_OPS, "Parking brake change event received.");
}