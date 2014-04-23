#include "stdafx.h"
#include "gepimpl/subsystems/inputHandler.h"
#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"

gep::InputHandler::InputHandler()
    : m_mouseSensitivity(0.5f, 0.5f),
    m_currentFrame(1)
{
}

void gep::InputHandler::initialize()
{
    m_currentFrame = 1;

    // adds HID mouse and also ignores legacy mouse messages
    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01;
    Rid.usUsage = 0x02;
    Rid.dwFlags = 0;
    Rid.hwndTarget = 0;
    BOOL r = RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
    GEP_ASSERT(r, "getting raw mouse input failed");
}

void gep::InputHandler::destroy()
{
}

void gep::InputHandler::update(float elapsedTime)
{
    m_mouseDelta = vec2(0.0f, 0.0f);
    m_currentFrame++;
    MSG msg = {0};
    while( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
    {
        switch( msg.message )
        {
        case WM_QUIT:
            g_globalManager.getUpdateFramework()->stop();
            break;
        case WM_KEYDOWN: //Handle keyboard key down messages
            {
                //uint8 keyCode = (msg.lParam >> 16) & 0xFF;
                uint8 keyCode = uint8(msg.wParam);
                auto& info = m_keyMap[keyCode];

                if(!info.isPressed)
                {
                    info.isPressed = true;
                    info.keyDownFrame = m_currentFrame;
                }
            }
            break;
        case WM_KEYUP: //Handle keyboard key up messages
            {
                //uint8 keyCode = (msg.lParam >> 16) & 0xFF;
                uint8 keyCode = uint8(msg.wParam);
                m_keyMap[keyCode].isPressed = false;
            }
            break;
        case WM_INPUT: //Handle remaining raw input
            {
                UINT dwSize = 0;

                GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                if(dwSize > 0)
                {
                    uint8* buffer = (uint8*)alloca(dwSize);
                    UINT bytesWritten = GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER));
                    GEP_ASSERT(dwSize == bytesWritten);
                    RAWINPUT* raw = (RAWINPUT*)buffer;
                    if(raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        m_mouseDelta.x = (float)raw->data.mouse.lLastX * m_mouseSensitivity.x;
                        m_mouseDelta.y = (float)raw->data.mouse.lLastY * m_mouseSensitivity.y;
                    }
                }
            }
            break;
        default:
            TranslateMessage( &msg );
            DispatchMessage( &msg );
            break;
        }
    }
}

bool gep::InputHandler::isPressed(uint8 keyCode)
{
    return m_keyMap[keyCode].isPressed;
}

bool gep::InputHandler::wasTriggered(uint8 keyCode)
{
    return m_keyMap[keyCode].keyDownFrame == m_currentFrame;
}

bool gep::InputHandler::getMouseDelta(vec2& mouseDelta)
{
    mouseDelta = m_mouseDelta;
    return !m_mouseDelta.epsilonCompare(vec2(0.0f, 0.0f));
}
