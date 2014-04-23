#include "stdafx.h"
#include "gpp/gameComponents/renderComponent.h"
#include "gep/globalManager.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/logging.h"

#include "gep/interfaces/scripting.h"


gpp::RenderComponent::RenderComponent():
    Component(),
    m_path(),
    m_pModel(nullptr),
    m_extractionCallbackId(0)
{

}

gpp::RenderComponent::~RenderComponent()
{
    m_pModel = nullptr;
}

void gpp::RenderComponent::initalize()
{
    if(m_path.empty())
    {
        g_globalManager.getLogging()->logError("Render Component's path on GameObject %s hasn't been set!", m_pParentGameObject->getName().c_str());
        GEP_ASSERT(false);
    }
    else
    {
        m_pModel = g_globalManager.getRenderer()->loadModel(m_path.c_str());
        m_extractionCallbackId = g_globalManager.getRendererExtractor()->registerExtractionCallback(std::bind(&RenderComponent::extract,this,std::placeholders::_1));
    }
}

void gpp::RenderComponent::update(float elapsedMS)
{
}

void gpp::RenderComponent::destroy()
{
    g_globalManager.getRendererExtractor()->deregisterExtractionCallback(m_extractionCallbackId);
}

void gpp::RenderComponent::extract(gep::IRendererExtractor& extractor)
{
    m_pModel->extract(extractor, m_pParentGameObject->getTransformationMatrix());
}
