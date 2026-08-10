#pragma once

// ============================================================================
// SHADE OPERATIONS
//
// Represents one physical shade and owns its movement behavior.
// ============================================================================

#include <Arduino.h>

class ShadeOps {
public:
    bool startup();
    void evtLoop();

private:
    static void parkingBrakeChanged();

    
};

extern ShadeOps shadeOps;