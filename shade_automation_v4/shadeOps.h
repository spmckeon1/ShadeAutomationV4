#pragma once

// ============================================================================
// SHADE OPERATIONS
//
// Represents one physical shade and owns its movement behavior.
// ============================================================================

class ShadeOps
{
public:

    bool startup();

    void evtLoop();

};

extern ShadeOps shadeOps;