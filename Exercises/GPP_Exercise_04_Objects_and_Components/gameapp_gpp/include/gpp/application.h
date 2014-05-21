#pragma once
#include "gep\interfaces\updateFramework.h"

namespace gep
{
    class FreeCamera;
    class IRendererExtractor;
}

namespace gpp
{
    class GameObjectManager;

    class Experiments
    {
        gep::FreeCamera* m_pFreeCamera;

    public:
        Experiments();

        virtual ~Experiments();

        void initialize();
        void destroy();
        void update(float elapsedTime);
        void render(gep::IRendererExtractor& extractor);

        void makeScriptBindings();
    };
}
