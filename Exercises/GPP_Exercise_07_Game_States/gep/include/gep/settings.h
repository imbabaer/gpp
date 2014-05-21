#pragma once

#include "gep/math3d/vec2.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    namespace settings
    {
        struct Video
        {
            ivec2 screenResolution;
            bool vsyncEnabled;

            Video() :
                screenResolution(1280, 720),
                vsyncEnabled(true)
            {
            }
        };
    }

    // Can be set in scripts
    class ISettings
    {
    public:
        ISettings() {}
        virtual ~ISettings() {}

        virtual void setVideoSettings(const settings::Video& settings) = 0;
        virtual       settings::Video& getVideoSettings()       = 0;
        virtual const settings::Video& getVideoSettings() const = 0;

        virtual void loadFromScriptTable(ScriptTableWrapper table) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(loadFromScriptTable, "load")
        LUA_BIND_REFERENCE_TYPE_END
    };
}
