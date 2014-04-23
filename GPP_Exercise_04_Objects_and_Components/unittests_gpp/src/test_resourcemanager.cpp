#include "stdafx.h"
#include "gep/ArrayPtr.h"
#include "gep/globalManager.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/exception.h"
#include "gep/file.h"

enum class LoaderState
{
    FailNextLoad,
    FailAllLoads,
    FailSecondLoad,
    LoadSuccessfully
};

class TestResourceLoader;

class TestResource : public gep::IResource
{
private:
    const char* m_resourceType;

public:
    TestResourceLoader* m_pLoader;
    gep::uint32 m_finalizeOptions;
    static int s_finalizeCount;

    TestResource(const char* resourceType, gep::uint32 finalizeOptions) :
        m_resourceType(resourceType),
        m_pLoader(nullptr),
        m_finalizeOptions(finalizeOptions)
    {
    }

    virtual gep::IResourceLoader* getLoader() override;
    virtual void setLoader(gep::IResourceLoader* pLoader) override;
    virtual gep::IResource* getSuperResource() override { return nullptr; }
    virtual const char* getResourceType() override { return m_resourceType; }
    virtual void unload() override { }
    virtual void finalize() override { s_finalizeCount++; }
    virtual gep::uint32 getFinalizeOptions() override { return m_finalizeOptions; }
    virtual bool isLoaded() override { return true; }
};

int TestResource::s_finalizeCount = 0;

class TestResourceLoader : public gep::IResourceLoader
{
private:
    const char* m_resourceId;
    const char* m_resourceType;
    const char* m_registerForFile;
    LoaderState m_state;
    bool m_isRegistered;
    gep::uint32 m_finalizeOptions;
    bool m_isCopy;

public:
    static int s_copyCount;
    static int s_deleteCount;
    static int s_postLoadCount;

    TestResourceLoader(const char* resourceId, const char* resourceType, LoaderState state, const char* registerForFile, gep::uint32 finalizeOptions) :
        m_resourceId(resourceId),
        m_resourceType(resourceType),
        m_state(state),
        m_registerForFile(registerForFile),
        m_isRegistered(false),
        m_finalizeOptions(finalizeOptions),
        m_isCopy(false)
    {

    }

    ~TestResourceLoader()
    {
        if(m_isRegistered)
        {
            g_globalManager.getResourceManager()->deregisterLoaderForReload(m_registerForFile, this);
        }
    }

    virtual gep::IResource* loadResource(gep::IResource* pInPlace) override
    {
        switch(m_state)
        {
        case LoaderState::FailNextLoad:
            m_state = LoaderState::LoadSuccessfully;
            throw gep::LoadingError("loading test resource failed");
            break;
        case LoaderState::FailAllLoads:
            throw gep::LoadingError("loading test resource failed");
        case LoaderState::FailSecondLoad:
            m_state = LoaderState::FailNextLoad;
            //fall through
        case LoaderState::LoadSuccessfully:
            return new TestResource(m_resourceType, m_finalizeOptions);
            break;
        }
        GEP_ASSERT(0, "should not be reached");
        return nullptr;
    }

    virtual void deleteResource(gep::IResource* pResource) override
    {
        TestResource* pTestResource = dynamic_cast<TestResource*>(pResource);
        GEP_ASSERT(pTestResource != nullptr, "pResource is null or wrong type");
        delete pTestResource;
    }

    virtual const char* getResourceType() override
    {
        return m_resourceType;
    }

    virtual const char* getResourceId() override
    {
        return m_resourceId;
    }

    virtual gep::IResourceLoader* moveToHeap() override
    {
        s_copyCount++;
        GEP_ASSERT(m_isCopy == false, "this is already a copy");
        auto result = new TestResourceLoader(*this);
        result->m_isCopy = true;
        return result;
    }

    virtual void release() override
    {
        s_deleteCount++;
        delete this;
    }

    virtual void postLoad(gep::ResourcePtr<gep::IResource> pResource) override
    {
        GEP_ASSERT(m_isCopy, "postLoad should only be called on a copy");
        s_postLoadCount++;
        if(m_registerForFile != nullptr && !m_isRegistered)
        {
            m_isRegistered = true;
            g_globalManager.getResourceManager()->registerLoaderForReload(m_registerForFile, this, pResource);
        }
    }
};

int TestResourceLoader::s_copyCount = 0;
int TestResourceLoader::s_deleteCount = 0;
int TestResourceLoader::s_postLoadCount = 0;

gep::IResourceLoader* TestResource::getLoader()
{
    return m_pLoader;
}

void TestResource::setLoader(gep::IResourceLoader* pLoader)
{
    auto pTemp = dynamic_cast<TestResourceLoader*>(pLoader);
    GEP_ASSERT(pTemp != nullptr, "pLoader is null or of wrong type");
    m_pLoader = pTemp;
}

GEP_UNITTEST_GROUP(ResourceManager)
GEP_UNITTEST_TEST(ResourceManager, ResourceManager)
{
    try {
        // create the resource files
        {
            gep::RawFile file("data/testResource1.txt", "wb");
            file.writeArray("test", 4);

            gep::RawFile file2("data/testResource2.txt", "wb");
            file.writeArray("test", 4);
        }

        g_globalManager.initialize();
        SCOPE_EXIT {
            g_globalManager.destroy();
            //+1 because of the manual allocated resource loader for the dummy resource
            GEP_ASSERT(TestResourceLoader::s_deleteCount == TestResourceLoader::s_copyCount+1, "resource loaders have not been released with the 'release' method");
        });

        // test if the dummy resource is returned on loading fail
        TestResource* pDummy1 = new TestResource("TestResource1", 0);
        pDummy1->setLoader(new TestResourceLoader("TestResourceDummy", "TestResource1", LoaderState::LoadSuccessfully, nullptr, 0));
        g_globalManager.getResourceManager()->registerResourceType("TestResource1", pDummy1);

        TestResourceLoader loader1("testResourceId1", "TestResource1", LoaderState::FailNextLoad, "data/testResource1.txt", gep::ResourceFinalize::NotYet || gep::ResourceFinalize::FromTest);
        auto ptr1 = g_globalManager.getResourceManager()->loadResource<TestResource>(loader1, gep::LoadAsync::No);
        GEP_ASSERT(TestResourceLoader::s_copyCount == 1, "resource loader has not been copied to the stack");
        GEP_ASSERT(ptr1 != nullptr, "no resource has been loaded! (Dummy resource should be returned)");
        GEP_ASSERT(ptr1 == pDummy1, "the dummy resource should have been returned on a failed load");
        GEP_ASSERT(TestResourceLoader::s_postLoadCount == 1, "postLoad should have been called");

        g_globalManager.getResourceManager()->finalizeResourcesWithFlags(gep::ResourceFinalize::FromTest);
        GEP_ASSERT( TestResource::s_finalizeCount == 0, "nothing should have been finalized");

        auto ptr4 = g_globalManager.getResourceManager()->loadResource<TestResource>(loader1, gep::LoadAsync::No);
        GEP_ASSERT(ptr1 == ptr4, "ptr1 and ptr4 should be the same (because of the same resource id beeing used)");
        GEP_ASSERT(TestResourceLoader::s_copyCount == 1, "the resource loader should not have been copied");
        GEP_ASSERT(TestResourceLoader::s_postLoadCount == 1, "post load should not have been called");

        TestResourceLoader loader2("testResourceId2", "TestResource1", LoaderState::LoadSuccessfully, "data/testResource2.txt", gep::ResourceFinalize::FromTest);
        auto ptr2 = g_globalManager.getResourceManager()->loadResource<TestResource>(loader2, gep::LoadAsync::No);
        GEP_ASSERT(TestResourceLoader::s_copyCount == 2, "resource loader has not been copied onto the stack");
        GEP_ASSERT(TestResourceLoader::s_postLoadCount == 2, "postLoad should have been called");
        GEP_ASSERT(ptr2 != nullptr, "no resource has been loaded!");
        GEP_ASSERT(ptr2 != pDummy1, "the loaded resource should not be the dummy!");

        TestResourceLoader loader3("testResourceId2", "TestResource1", LoaderState::LoadSuccessfully, "data/testResource2.txt", gep::ResourceFinalize::FromTest);
        auto ptr3 = g_globalManager.getResourceManager()->loadResource<TestResource>(loader3, gep::LoadAsync::No);
        GEP_ASSERT(TestResourceLoader::s_copyCount == 2, "when the same resource is loaded again the loader should not be copied to the heap");
        GEP_ASSERT(TestResourceLoader::s_postLoadCount == 2, "postLoad should not have been called");
        GEP_ASSERT(ptr3 == ptr2, "ptr2 and ptr3 should point to the same resource");

        TestResourceLoader loader4("testResourceId3", "TestResource1", LoaderState::LoadSuccessfully, nullptr, gep::ResourceFinalize::NotYet);
        auto ptr5 = g_globalManager.getResourceManager()->loadResource<TestResource>(loader4, gep::LoadAsync::No);
        GEP_ASSERT(TestResourceLoader::s_copyCount == 3, "loader has not been copied to the heap");
        GEP_ASSERT(TestResourceLoader::s_postLoadCount == 3, "postLoad should have been called");
        GEP_ASSERT(ptr5 != pDummy1, "ptr4 should not be the dummy");
        auto ptr6 = ptr5;
        g_globalManager.getResourceManager()->deleteResource(ptr5);
        GEP_ASSERT(ptr5.get() == nullptr, "ptr5 should be null after its resource got deleted");
        GEP_ASSERT(ptr6.get() == nullptr, "ptr6 should be null after its resource got deleted");


        GEP_ASSERT( TestResource::s_finalizeCount == 0, "nothing should have been finalized");
        g_globalManager.getResourceManager()->finalizeResourcesWithFlags(gep::ResourceFinalize::FromTest);
        GEP_ASSERT( TestResource::s_finalizeCount == 1, "one resource should have been finalized");

        //TODO fix
        // modify the resource file
        /*{
            gep::RawFile file("data/testResource1.txt", "wb");
            file.writeArray("test2", 5);
        }

        // give the resource manager a chance to reload
        g_globalManager.getResourceManager()->update(1000.0f);

        GEP_ASSERT(ptr1 != pDummy1, "the resource dummy should have been replaced by a real resource");
        GEP_ASSERT(ptr1->getLoader() != nullptr, "the loader was not set on the resource");

        g_globalManager.getResourceManager()->finalizeResourcesWithFlags(gep::ResourceFinalize::FromTest);
        GEP_ASSERT( TestResource::s_finalizeCount == 1, "nothing should have been finalized");

        ptr1->m_finalizeOptions = gep::ResourceFinalize::FromTest;
        g_globalManager.getResourceManager()->finalizeResourcesWithFlags(gep::ResourceFinalize::FromTest);
        GEP_ASSERT( TestResource::s_finalizeCount == 2, "one more resource should have been finalized");

        // modify the second resource file
        {
            gep::RawFile file("data/testResource2.txt", "wb");
            file.writeArray("test2", 5);
        }

        gep::IResource* pBefore = ptr3.operator->();

        // give the resource manager a chance to reload
        g_globalManager.getResourceManager()->update(1000.0f);

        GEP_ASSERT(ptr3 != pBefore, "the resource should have changed upon reload");
        GEP_ASSERT(ptr3 == ptr2, "ptr2 and ptr3 should still be the same");*/

    }
    catch(std::exception& ex)
    {
        GEP_ASSERT(false, "exception during unittest", ex.what());
    }
}
