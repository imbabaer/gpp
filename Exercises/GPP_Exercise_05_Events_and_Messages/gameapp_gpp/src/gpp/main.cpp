#include "stdafx.h"

#include "gpp/application.h"

#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

#include "gpp/gameObjectSystem.h"

int main(int argc, const char* argv[])
{
    gpp::Experiments e;
    try {
        g_globalManager.initialize();
        g_globalManager.getUpdateFramework()->registerUpdateCallback(std::bind(&gpp::Experiments::update, &e, std::placeholders::_1));
        g_globalManager.getUpdateFramework()->registerInitializeCallback(std::bind(&gpp::Experiments::initialize, &e));
        g_globalManager.getUpdateFramework()->registerDestroyCallback(std::bind(&gpp::Experiments::destroy, &e));

        // initialize and destroy the game object manager in the game thread
        auto& gameObjectManager = g_gameObjectManager;
        g_globalManager.getUpdateFramework()->registerInitializeCallback(std::bind(&gpp::GameObjectManager::initialize, &gameObjectManager));
        g_globalManager.getUpdateFramework()->registerDestroyCallback(std::bind(&gpp::GameObjectManager::destroy, &gameObjectManager));

    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("Fatal error initializing: %s", ex.what());
        g_globalManager.destroy();
        #ifdef _DEBUG
        std::cout << std::endl << "press ENTER to quit...";
        std::cin.get();
        #endif
        return -1;
    }

    try
    {
        g_globalManager.getUpdateFramework()->run();
    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("Fatal error during execution: %s", ex.what());
    }
    g_globalManager.destroy();
    gep::destroy(); // Shut down the engine

    // Uncomment to enable a prompt before exiting the game to read the console or something
    //#ifdef _DEBUG
    //std::cout << std::endl << "press ENTER to quit...";
    //std::cin.get();
    //#endif

    return 0;
}
