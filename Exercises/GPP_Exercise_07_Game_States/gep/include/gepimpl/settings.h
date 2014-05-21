#pragma once
#include "gep/settings.h"

#include "gep/math3d/vec2.h"

namespace gep
{
    class Settings : public ISettings
    {
        settings::Video m_video;
        ScriptTableWrapper m_scriptTable;
    public:
        Settings();
        virtual ~Settings();

        virtual void loadFromScriptTable(ScriptTableWrapper table) override;

        virtual void setVideoSettings(const settings::Video& settings) override { m_video = settings; }
        virtual       settings::Video& getVideoSettings()       override { return m_video; }
        virtual const settings::Video& getVideoSettings() const override { return m_video; }

    };
}
