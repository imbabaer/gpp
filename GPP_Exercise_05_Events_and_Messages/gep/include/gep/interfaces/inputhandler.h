#pragma once

#include "gep/types.h"
#include "gep/interfaces/subsystem.h"
#include "gep/math3d/vec2.h"

// For convenience
#include "gep/input/windowsVirtualKeyCodes.h"

namespace gep
{
    class IInputHandler : public ISubsystem
    {
    public:
        /// \brief returns if the given key is still pressed
        virtual bool isPressed(uint8 keyCode) = 0;
        /// \brief returns if the given key was pressed this frame
        virtual bool wasTriggered(uint8 keyCode) = 0;
        /// \brief gets the mouse delta for this frame
        /// \return true if there was mouse movement, false otherwise
        virtual bool getMouseDelta(vec2& mouseDelta) = 0;


        vec2 getMouseDeltaCopy()
        {
            vec2 result;
            getMouseDelta(result);
            return result;
        }

        bool hasMouseDelta()
        {
            vec2 unused;
            return getMouseDelta(unused);
        }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(isPressed)
            LUA_BIND_FUNCTION(wasTriggered)
            LUA_BIND_FUNCTION_NAMED(getMouseDeltaCopy, "getMouseDelta")
            LUA_BIND_FUNCTION(hasMouseDelta)
        LUA_BIND_REFERENCE_TYPE_END
    };
}
