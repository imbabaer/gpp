#pragma once

#include "gep/interfaces/inputHandler.h"
#include "gep/container/hashmap.h"

namespace gep
{
    class InputHandler : public IInputHandler
    {
    private:
        vec2 m_mouseDelta;

        vec2 m_mouseSensitivity;
        uint32 m_currentFrame;

        struct KeyInfo
        {
            bool isPressed;
            uint32 keyDownFrame;

            inline KeyInfo() : isPressed(false), keyDownFrame(0) {}
        };
        enum { MAX_NUM_KEYS = VK_OEM_CLEAR };
        KeyInfo m_keyMap[MAX_NUM_KEYS];

    public:
        InputHandler();

        virtual void initialize() override;
        virtual void destroy() override;
        virtual void update(float elapsedTime) override;

        /// \brief returns if the given virtual key (VK_) is still pressed
        virtual bool isPressed(uint8 keyCode) override;
        /// \brief returns if the given virtual key (VK_) was pressed this frame
        virtual bool wasTriggered(uint8 keyCode) override;
        /// \brief gets the mouse delta for this frame
        /// \return true if there was mouse movement, false otherwise
        virtual bool getMouseDelta(vec2& mouseDelta) override;
    };
}
