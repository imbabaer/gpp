#pragma once

#include "gep/interfaces/scripting.h"
#include "gep/container/DynamicArray.h"

namespace gep
{
    class IAllocatorStatistics;

    class ScriptingManager : public IScriptingManager
    {
    public:

        ScriptingManager(const std::string& scriptsRoot = "data/scripts/", const std::string& importantScriptsRoot = "data/base/");
        virtual ~ScriptingManager();
        
        /// \brief IScriptingManager interface
        virtual void initialize();
        virtual void update(float elapsedTime);
        virtual void destroy();

        virtual void loadScript(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default);

        virtual void registerScript(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default) override;
        virtual void loadAllRegisteredScripts() override;

        virtual       std::string& getScriptsRoot()       override { return m_scriptsRoot; }
        virtual const std::string& getScriptsRoot() const override { return m_scriptsRoot; }
        virtual void setScriptsRoot(const std::string& value) override { m_scriptsRoot = value; }

        virtual       std::string& getImportantScriptsRoot()       override { return m_importantScriptsRoot; }
        virtual const std::string& getImportantScriptsRoot() const override { return m_importantScriptsRoot; }
        virtual void setImportantScriptsRoot(const std::string& value) override { m_importantScriptsRoot = value; }

        virtual void bindEnum(const char* enumName, ...) override;

        virtual int32 memoryUsed() const override;
        virtual void collectGarbage() override;

        virtual void debugBreak(const char* message) const override;

        void makeBasicBindings();

    private:
        IAllocatorStatistics* m_pAllocator;

        lua_State* m_L;
        State m_state;

        std::string m_scriptsRoot;
        std::string m_importantScriptsRoot;

        gep::DynamicArray<std::string> m_scriptsToLoad;

        std::string constructFileName(const std::string& filename, LoadOptions::Enum loadOptions = LoadOptions::Default);

        virtual void setState(State state) override { m_state = state; }

        virtual State getState() const override { return m_state; }

        virtual lua_State* getState() override { return m_L; }
    };

}
