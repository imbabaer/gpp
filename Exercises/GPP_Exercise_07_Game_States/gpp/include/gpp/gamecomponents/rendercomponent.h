#pragma once

#include "gpp/gameObjectSystem.h"
#include "gep/interfaces/updateFramework.h"


namespace gep
{
    class IModel;
    class IRendererExtractor;
}


namespace gpp
{
    class RenderComponent : public Component
    {
    public:
        RenderComponent();
        virtual ~RenderComponent();

        //make sure the path is set before initializing this component!
        virtual void initalize();

        virtual void update(float elapsedMS);

        virtual void destroy();

        void extract(gep::IRendererExtractor& extractor);

        inline void setPath(const std::string& path){ m_path = path; }
        inline const std::string& getPath() const { return m_path; }
        inline std::string getPathCopy() const { return m_path; }

        virtual void setState(State::Enum state) override;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setPath)
            LUA_BIND_FUNCTION_NAMED(getPathCopy, "getPath")
            LUA_BIND_FUNCTION(setState)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        gep::IModel* m_pModel;

        std::string m_path;
        gep::CallbackId m_extractionCallbackId;


    };

    template<>
    struct ComponentMetaInfo<RenderComponent>
    {
        static const char* name(){ return "RenderComponent"; }
        static const int priority(){ return 1; }
        static RenderComponent* create(){return new RenderComponent(); }
    };
}
