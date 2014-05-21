// GameApp.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "gep/globalManager.h"
#include "gep/exit.h"
#include "gepimpl/subsystems/logging.h"
#include "gepimpl/subsystems/updateFramework.h"

// Implement new/delete so they match the engine dll
#include "gep/memory/newdelete.inl"
#include "gep/interfaces/renderer.h"
#include "gep/cameras.h"
#include "gep/interfaces/inputHandler.h"
#include "gep/threading/taskQueue.h"
#include "gep/interfaces/sound.h"
#include "gep/utils.h"

using namespace gep;

class Player
{
private:
    vec3 m_position;
    float m_rotation;
    IModel* m_pModel;
    
public:
    Player() : 
        m_position(0,0,20),
        m_rotation(0.0f),
        m_pModel(nullptr)
    {
    }

    void initialize()
    {
        m_pModel = g_globalManager.getRenderer()->loadModel("data/models/ball.thModel");
    }

    const mat4 getTransformation() const
    {
        return mat4::translationMatrix(m_position) * Quaternion(vec3(0,0,1), m_rotation).toMat4();
    }

    void extract(IRendererExtractor& extractor)
    {
        m_pModel->extract(extractor, getTransformation());
    }

    void move(vec3 delta)
    {
        std::swap(delta.y, delta.z);
        delta.y = -delta.y;
        m_position += Quaternion(vec3(0,0,1), m_rotation).toMat3() * delta;
    }

    void look(vec2 delta)
    {
        m_rotation += delta.x;
    }
};

class MandelbrotTask : public ITask
{
private:
    uint32 m_y, m_size;
    ArrayPtr<uint8> m_data;
    float m_zoom;

public:
    void setRegion(float zoom, uint32 y, uint32 size, ArrayPtr<uint8> data)
    {
        m_y = y;
        m_size = size;
        m_data = data;
        m_zoom = zoom;
    }

    int mandelbrot(vec2 coord)
    {
      double max_quadrat = 4.0;
      int iter = 0;
      double betrag_quadrat = 0.0;
      vec2 pos = vec2(0.0, 0.0);

      const int maxIterations = 1000;
      while(betrag_quadrat <= max_quadrat && iter < maxIterations)
      {
        pos = vec2(pos.x * pos.x - pos.y * pos.y + coord.x, 2.0f * pos.x * pos.y + coord.y);
        iter++;
        betrag_quadrat = pos.squaredLength();
      }

      return iter;
    }

    void execute() override
    {
        float size = (float)m_size;
        float y = ((float)m_y / size - 0.5f) * m_zoom + 0.27f;
        
        for(uint32 x=0; x < m_size; x++)
        {
            int result = mandelbrot(vec2(y, ((float)x / size - 0.5f) * m_zoom + 0.005f));
            m_data[x*4] = (result % 32) * 8;      // R 
            m_data[x*4+1] = (result % 128) * 2;   // G
            m_data[x*4+2] = result % 256;         // B
            m_data[x*4+3] = 255;                  // A
        }
    }
};


class GameApp
{
private:
    IModel* m_pBall;
    IModel* m_pSponza;
    mat4 m_ballTransform;
    enum class CameraMode
    {
        Free,
        Horizon,
        ThirdPerson
    };

    CameraMode m_cameraMode;
    ICamera* m_pCamera;
    FreeCamera* m_pFreeCamera;
    Player m_player;

    TaskGroup* m_pTaskGroup;
#ifdef _DEBUG
    static const uint32 MANDELBROT_SIZE = 64;
#else
    static const uint32 MANDELBROT_SIZE = 128;
#endif
    DynamicArray<MandelbrotTask> m_tasks;
    float m_mandelbrotZoom;
    float m_zoomDir;
    Semaphore m_taskDone;
    ResourcePtr<IResource> m_mandelbrotTexture;

    ResourcePtr<ISoundLibrary> m_masterSoundLibrary;
    ResourcePtr<ISoundLibrary> m_stringSoundLibrary;
    ResourcePtr<ISoundLibrary> m_ambienceSoundLibrary;
    ResourcePtr<ISoundLibrary> m_vehicleSoundLibrary;

    ResourcePtr<ISound> m_ambienceSound;
    ResourcePtr<ISoundInstance> m_ambienceSoundInstance;
    ResourcePtr<ISound> m_engineSound;
    ResourcePtr<ISoundInstance> m_engineSoundInstance;
    SoundParameter m_engineRPM;

public:
    GameApp() : 
        m_pFreeCamera(nullptr)
      , m_pCamera(nullptr)
      , m_pTaskGroup(nullptr)
      , m_mandelbrotZoom(1.0f)
      , m_zoomDir(-1.0f)
      , m_taskDone(0)
    {
    }

    ~GameApp()
    {
        DELETE_AND_NULL(m_pCamera)
    }

    void deinit()
    {
        g_globalManager.getTaskQueue()->deleteGroup(m_pTaskGroup);
    }

    void initialize()
    {
        m_pTaskGroup = g_globalManager.getTaskQueue()->createGroup();
        m_tasks.resize(MANDELBROT_SIZE);
        for(auto& task : m_tasks)
        {
            m_pTaskGroup->addTask(&task);
        }
        m_pTaskGroup->setOnFinished(std::bind(&GameApp::mandelbrotDone, this));
        m_mandelbrotTexture = g_globalManager.getRenderer()->createGeneratedTexture(MANDELBROT_SIZE, MANDELBROT_SIZE, ".\\data\\sponza\\\\sponza_curtain_diff.dds", std::bind(&GameApp::generateMandelbrot, this, std::placeholders::_1));
        m_masterSoundLibrary = g_globalManager.getSoundSystem()->loadLibrary(".\\data\\sound\\Master Bank.bank");
        m_stringSoundLibrary = g_globalManager.getSoundSystem()->loadLibrary(".\\data\\sound\\Master Bank.bank.strings");
        m_ambienceSoundLibrary = g_globalManager.getSoundSystem()->loadLibrary(".\\data\\sound\\Surround_Ambience.bank");
        m_vehicleSoundLibrary = g_globalManager.getSoundSystem()->loadLibrary(".\\data\\sound\\Vehicles.bank");

        m_ambienceSound = g_globalManager.getSoundSystem()->getSound("/Ambience/Country");
        m_ambienceSoundInstance = m_ambienceSound->createInstance();
        m_ambienceSoundInstance->play();

        m_engineSound = g_globalManager.getSoundSystem()->getSound("/Vehicles/Basic Engine");
        m_engineSoundInstance = m_engineSound->createInstance();
        m_engineRPM = m_engineSoundInstance->getParameter("RPM");
        m_engineRPM.setValue(650.0f);
        m_engineSoundInstance->play();
        m_pBall = g_globalManager.getRenderer()->loadModel("data/models/ball.thModel");
        m_pSponza = g_globalManager.getRenderer()->loadModel("data/sponza/sponza.thModel");
        g_globalManager.getRendererExtractor()->registerExtractionCallback(std::bind(&GameApp::render, this, std::placeholders::_1));
        m_pFreeCamera = new FreeCamera();
        m_pCamera = m_pFreeCamera;
        m_cameraMode = CameraMode::Free;
        m_player.initialize();
    }

    void gameloop(float elapsedTime)
    {
        float s = sin(g_globalManager.getTimer().getTimeAsFloat()/1000.0f);
        m_ballTransform = mat4::translationMatrix(vec3(0, 0, 100.0f + s * s * 50.0f));
        auto pInputHandler = g_globalManager.getInputHandler();
        vec2 mouseDelta;
        if(pInputHandler->getMouseDelta(mouseDelta))
        {
            if(m_cameraMode == CameraMode::ThirdPerson)
            {
                m_player.look(mouseDelta);
            }
            else
            {
                m_pFreeCamera->look(mouseDelta);
            }
        }
        vec3 moveDelta;
        if(pInputHandler->isPressed(17) || pInputHandler->isPressed(72))
            moveDelta.z -= 1.0f;
        if(pInputHandler->isPressed(31) || pInputHandler->isPressed(80))
            moveDelta.z += 1.0f;
        if(pInputHandler->isPressed(32) || pInputHandler->isPressed(77))
            moveDelta.x += 1.0f;
        if(pInputHandler->isPressed(30) || pInputHandler->isPressed(75))
            moveDelta.x -= 1.0f;
        moveDelta *= elapsedTime;
        if(m_cameraMode == CameraMode::ThirdPerson)
        {
            m_player.move(moveDelta * 0.2f);
            ThirdPersonCamera* pThirdPersonCamera = static_cast<ThirdPersonCamera*>(m_pCamera);
            pThirdPersonCamera->follow(m_player.getTransformation());
            if (pInputHandler->wasTriggered(33)) // f
            {
                switch (pThirdPersonCamera->getFollowMode())
                {
                    case ThirdPersonCamera::Direct: pThirdPersonCamera->setFollowMode(ThirdPersonCamera::Smooth); break;
                    case ThirdPersonCamera::Smooth: pThirdPersonCamera->setFollowMode(ThirdPersonCamera::Direct); break;
                }
            }
        }
        else
        {
            m_pFreeCamera->move(moveDelta);
        }
        float tiltDelta = 0.0f;
        if(pInputHandler->isPressed(16))
            tiltDelta -= 0.1f;
        if(pInputHandler->isPressed(18))
            tiltDelta += 0.1f;
        if(m_cameraMode != CameraMode::ThirdPerson)
            m_pFreeCamera->tilt(tiltDelta * elapsedTime);
        if(pInputHandler->wasTriggered(57)) //space
        {
            DELETE_AND_NULL(m_pCamera);
            m_pFreeCamera = nullptr;
            switch(m_cameraMode)
            {
            case CameraMode::Free:
                m_cameraMode = CameraMode::Horizon;
                m_pFreeCamera = new FreeCameraHorizon();
                g_globalManager.getLogging()->logMessage("now using horizon camera");
                m_pCamera = m_pFreeCamera;
                break;
            case CameraMode::Horizon:
                m_cameraMode = CameraMode::ThirdPerson;
                m_pCamera = new ThirdPersonCamera(vec3(0, -100, 20));
                g_globalManager.getLogging()->logMessage("now using third person camera");
                break;
            case CameraMode::ThirdPerson:
                m_cameraMode = CameraMode::Free;
                m_pFreeCamera = new FreeCamera();
                g_globalManager.getLogging()->logMessage("now using free camera");
                m_pCamera = m_pFreeCamera;
                break;
            }
        }

        m_mandelbrotZoom += m_zoomDir * elapsedTime * 0.01f;
        if(m_mandelbrotZoom < 0.005f)
        {
            m_mandelbrotZoom = 0.005f;
            m_zoomDir = 1.0f;
        }
        if(m_mandelbrotZoom > 3.0f)
        {
            m_mandelbrotZoom = 3.0f;
            m_zoomDir = -1.0f;
        }
        g_globalManager.getResourceManager()->reloadResource(m_mandelbrotTexture);

        // pause the engine sound
        if(pInputHandler->wasTriggered(25))	// p
        {
            m_engineSoundInstance->setPaused(!m_engineSoundInstance->getPaused());
        }
        // increase the engine's RPM
        if(pInputHandler->wasTriggered(78) || pInputHandler->wasTriggered(27))	// +
        {
            float value;
            m_engineRPM.getValue(&value);
            if (value<=1950.f)
                m_engineRPM.setValue(value + 50.0f);
        }
        // decrease the engine's RPM
        if(pInputHandler->wasTriggered(74) || pInputHandler->wasTriggered(53))	// -
        {
            float value;
            m_engineRPM.getValue(&value);
            if (value>=50.f)
                m_engineRPM.setValue(value - 50.0f);
        }
                
        // Update position for engine sound
        vec3 ballPos(m_ballTransform.data[12], m_ballTransform.data[13], m_ballTransform.data[14]);
        m_engineSoundInstance->setPosition(ballPos * 0.01f);

        // Update listener position
        mat4 cameraTransform = m_pCamera->getViewMatrix().inverse();
        g_globalManager.getSoundSystem()->setListenerOrientation(Quaternion(cameraTransform.rotationPart()));
        g_globalManager.getSoundSystem()->setListenerPosition(cameraTransform.translationPart() * 0.01f);

    }

    void render(IRendererExtractor& extractor)
    {
        extractor.setCamera(m_pCamera);
        m_pSponza->extract(extractor, mat4::identity());
        m_pBall->extract(extractor, m_ballTransform);
        m_player.extract(extractor);

        auto& context2D = extractor.getContext2D();
        float avg = g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60);
        float fps = 1000.0f / avg;
        context2D.printText(vec2(20, 20), gep::format("FPS: %f", fps).c_str());
        context2D.printText(vec2(20, 34), "Press [Space] to switch camera modes");
    }

    void generateMandelbrot(ArrayPtr<uint8> data)
    {
        uint32 y = 0;
        for(auto& task : m_tasks)
        {
            task.setRegion(m_mandelbrotZoom, y, MANDELBROT_SIZE, data(y * MANDELBROT_SIZE * 4, (y+1) * MANDELBROT_SIZE * 4));
            y++;
        }
        g_globalManager.getTaskQueue()->scheduleForExecution(m_pTaskGroup);
        g_globalManager.getTaskQueue()->runTasks();
        m_taskDone.waitAndDecrement();
    }

    void mandelbrotDone()
    {
        m_taskDone.increment();
    }
};

int main(int argc, const char* argv[])
{
    {
        GameApp app;
        try {
            g_globalManager.initialize();
            app.initialize();
            g_globalManager.getUpdateFramework()->registerUpdateCallback(std::bind(&GameApp::gameloop, &app, std::placeholders::_1));
        }
        catch(std::exception& ex)
        {
            g_globalManager.getLogging()->logError("Fatal Error initializing engine: %s", ex.what());
            g_globalManager.destroy();
            #ifdef _DEBUG
            std::cout << std::endl << "press any key to quit...";
            std::cin.get();
            #endif
            return -1;
        }

        try {
            g_globalManager.getUpdateFramework()->run();
        }
        catch(std::exception& ex)
        {
            g_globalManager.getLogging()->logError("Fatal Error during execution: %s", ex.what());
        }
        app.deinit();

        g_globalManager.destroy();
    }

    gep::destroy(); //Shutdown gep

    #ifdef _DEBUG
    std::cout << std::endl << "press any key to quit...";
    std::cin.get();
    #endif

    return 0;
}

