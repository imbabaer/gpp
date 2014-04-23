#include "stdafx.h"
#include "gpp/application.h"

#include "gep/globalManager.h"
#include "gep/exception.h"
#include "gep/cameras.h"
#include "gep/utils.h"

#include "gep/interfaces/logging.h"
#include "gep/interfaces/physics.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/inputHandler.h"

#include "gep/math3d/vec3.h"
#include "gep/math3d/color.h"

#include "gpp/gameObjectSystem.h"

#include "gep/interfaces/cameraManager.h"

using namespace gep;

gpp::Experiments::Experiments() :
    m_pDummyCam(nullptr)
{
}

gpp::Experiments::~Experiments()
{
}

void gpp::Experiments::initialize()
{
    g_globalManager.getRendererExtractor()->registerExtractionCallback(std::bind(&Experiments::render, this, std::placeholders::_1));
    m_pDummyCam = new FreeCameraHorizon();
    m_pDummyCam->setViewAngle(60.0f);
    g_globalManager.getCameraManager()->setActiveCamera(m_pDummyCam);

    // Scripting related initialization
    //////////////////////////////////////////////////////////////////////////

    auto scripting = g_globalManager.getScriptingManager();

    makeScriptBindings();

    try
    {
        // setup.lua
        scripting->loadScript("setup.lua", IScriptingManager::LoadOptions::IsImportantScript);

        // initialize.lua
        scripting->setState(IScriptingManager::State::AcceptingScriptRegistration);
        SCOPE_EXIT { scripting->setState(IScriptingManager::State::NotAcceptingScriptRegistration); });

        scripting->loadScript("initialize.lua", IScriptingManager::LoadOptions::IsImportantScript);
    }
    catch (ScriptLoadException& e)
    {
        auto message = format("Error loading script %s", e.what());
        GEP_ASSERT(false, message.c_str());
        g_globalManager.getLogging()->logError(message.c_str());
        throw e;
    }
    catch (ScriptExecutionException& e)
    {
        auto message = format("Error executing script %s", e.what());
        GEP_ASSERT(false, message.c_str());
        g_globalManager.getLogging()->logError(message.c_str());
        throw e;
    }

    try
    {
        g_globalManager.getScriptingManager()->loadAllRegisteredScripts();
    }
    catch (ScriptLoadException& e)
    {
        auto message = format("Error loading script %s", e.what());
        GEP_ASSERT(false, message.c_str());
        g_globalManager.getLogging()->logError(message.c_str());
    }
    catch (ScriptExecutionException& e)
    {
        auto message = format("Error executing script %s", e.what());
        GEP_ASSERT(false, message.c_str());
        g_globalManager.getLogging()->logError(message.c_str());
    }

    g_gameObjectManager.initialize();
}

void gpp::Experiments::destroy()
{
    DELETE_AND_NULL(m_pDummyCam);
}

void gpp::Experiments::update(float elapsedTime)
{
    auto pInputHandler = g_globalManager.getInputHandler();
    auto pPhysicsSystem = g_globalManager.getPhysicsSystem();
    auto pRenderer = g_globalManager.getRenderer();

    if (pInputHandler->wasTriggered(gep::Key::Escape)) // Escape
    {
        g_globalManager.getUpdateFramework()->stop();
        return;
    }

    /*  
    vec2 mouseDelta;
    if(pInputHandler->getMouseDelta(mouseDelta))
    {
    m_pFreeCamera->look(mouseDelta);
    }
    vec3 moveDelta;
    if(pInputHandler->isPressed(gep::Key::W))
    moveDelta.z -= 1.0f;
    if(pInputHandler->isPressed(gep::Key::S))
    moveDelta.z += 1.0f;
    if(pInputHandler->isPressed(gep::Key::D))
    moveDelta.x += 1.0f;
    if(pInputHandler->isPressed(gep::Key::A))
    moveDelta.x -= 1.0f;

    moveDelta *= elapsedTime; 

    m_pFreeCamera->move(moveDelta);

    float tiltDelta = 0.0f;
    if(pInputHandler->isPressed(gep::Key::Q))
    tiltDelta -= 0.1f;
    if(pInputHandler->isPressed(gep::Key::E))
    tiltDelta += 0.1f;
    m_pFreeCamera->tilt(tiltDelta * elapsedTime);
    */
    // Modify FOV
    /*
    if (pInputHandler->isPressed(gep::Key::X))
        m_pDummyCam->setViewAngle(m_pDummyCam->getViewAngle() + 0.5f);
    if (pInputHandler->isPressed(gep::Key::Z))
        m_pDummyCam->setViewAngle(m_pDummyCam->getViewAngle() - 0.5f);
    */

    if (pInputHandler->wasTriggered(gep::Key::F9))
        pPhysicsSystem->setDebugDrawingEnabled(!pPhysicsSystem->getDebugDrawingEnabled());
    if (pInputHandler->wasTriggered(gep::Key::F8)) // Toggle VSync
        pRenderer->setVSyncEnabled(!pRenderer->getVSyncEnabled());

    auto& debugRenderer = pRenderer->getDebugRenderer();
    debugRenderer.printText(vec3(0.0f), "Origin");

    // Draw world axes
    debugRenderer.drawLocalAxes(vec3(0.0f), 30.0f);
    debugRenderer.printText(vec3(30.0f, 0.0f,  0.0f ), "X", Color::red());
    debugRenderer.printText(vec3(0.0f,  30.0f, 0.0f ), "Y", Color::green());
    debugRenderer.printText(vec3(0.0f,  0.0f,  30.0f), "Z", Color::blue());
    g_gameObjectManager.update(elapsedTime);
}

void gpp::Experiments::render(gep::IRendererExtractor& extractor)
{
    auto activeCam = g_globalManager.getCameraManager()->getActiveCamera();
    DebugMarkerSection marker(extractor, "Main");
    extractor.setCamera(activeCam);

    auto& context2D = extractor.getContext2D();
    float avg = g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60);
    float fps = 1000.0f / avg;
   

    context2D.printText(g_globalManager.getRenderer()->toNormalizedScreenPosition(ivec2(10, 5)), gep::format("FPS: %f", fps).c_str());
    context2D.printText(g_globalManager.getRenderer()->toNormalizedScreenPosition(ivec2(10, 20)), gep::format("Memory used by lua: %d KB", g_globalManager.getScriptingManager()->memoryUsed()).c_str());
    //context2D.printText(g_globalManager.getRenderer()->toNormalizedScreenPosition(ivec2(30, 20)), gep::format("Camera Position: [%f, %f, %f]", camPos.x, camPos.y, camPos.z).c_str());
    //context2D.printText(g_globalManager.getRenderer()->toNormalizedScreenPosition(ivec2(30, 35)), gep::format("Camera View Angle: %f", m_pFreeCamera->getViewAngle()).c_str());
}
