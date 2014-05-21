#include "stdafx.h"
#include "gepimpl/settings.h"



gep::Settings::Settings() :
    m_video()
{
}

gep::Settings::~Settings()
{
}

void gep::Settings::loadFromScriptTable(ScriptTableWrapper table)
{
    {
        ScriptTableWrapper videoSettings;
        table.tryGet("video", videoSettings);
        videoSettings.tryGet("screenResolution", m_video.screenResolution);
        videoSettings.tryGet("vsyncEnabled", m_video.vsyncEnabled);
    }

    // more ...
}
