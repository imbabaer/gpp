#include "stdafx.h"
#include "gep/unittest/UnittestManager.h"
#include "gepimpl/subsystems/renderer/ddsLoader.h"
#include "gep/memory/allocators.h"

using namespace gep;

GEP_UNITTEST_GROUP(Renderer)
GEP_UNITTEST_TEST(Renderer, DDSLoader)
{
  {
    // DXT1 compressed file without mipmaps
    try
    {
        DDSLoader loader(&g_simpleLeakCheckingAllocator);

        loader.loadFile("testdata\\dxt1.dds");
        GEP_ASSERT(loader.getImages().length() == 1, "number of textures incorrect");
        GEP_ASSERT(loader.getImages()[0].length() == 1, "number of mipmaps incorrect");
        GEP_ASSERT(loader.getImages()[0][0].length() > 0, "no image data loaded");
    }
    catch(DDSLoadingException& ex)
    {
        GEP_ASSERT(0, ex.what());
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks

    // DXT1 compressed file with mipmaps
    try
    {
        DDSLoader loader(&g_simpleLeakCheckingAllocator);

        loader.loadFile("testdata\\dxt1_with_mipmaps.dds");
        GEP_ASSERT(loader.getImages().length() == 1, "number of textures incorrect");
        GEP_ASSERT(loader.getImages()[0].length() == 7, "number of mipmaps incorrect");
        for(int i=0; i<7; i++)
        {
            GEP_ASSERT(loader.getImages()[0][i].length() > 0, "no image data loaded");
        }
    }
    catch(DDSLoadingException& ex)
    {
        GEP_ASSERT(0, ex.what());
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks

    //DXT3 compressed file
    try
    {
        DDSLoader loader(&g_simpleLeakCheckingAllocator);

        loader.loadFile("testdata\\dxt3.dds");
        GEP_ASSERT(loader.getImages().length() == 1, "number of textures incorrect");
        GEP_ASSERT(loader.getImages()[0].length() == 1, "number of mipmaps incorrect");
        GEP_ASSERT(loader.getImages()[0][0].length() > 0, "no image data loaded");
    }
    catch(DDSLoadingException& ex)
    {
        GEP_ASSERT(0, ex.what());
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks

    //DXT3 compressed file with mipmaps
    try
    {
        DDSLoader loader(&g_simpleLeakCheckingAllocator);

        loader.loadFile("testdata\\dxt3_with_mipmaps.dds");
        GEP_ASSERT(loader.getImages().length() == 1, "number of textures incorrect");
        GEP_ASSERT(loader.getImages()[0].length() == 7, "number of mipmaps incorrect");
        for(int i=0; i<7; i++)
        {
            GEP_ASSERT(loader.getImages()[0][i].length() > 0, "no image data loaded");
        }
    }
    catch(DDSLoadingException& ex)
    {
        GEP_ASSERT(0, ex.what());
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks

    // DXT5 copressed image
    try
    {
        DDSLoader loader(&g_simpleLeakCheckingAllocator);

        loader.loadFile("testdata\\dxt5.dds");
        GEP_ASSERT(loader.getImages().length() == 1, "number of textures incorrect");
        GEP_ASSERT(loader.getImages()[0].length() == 1, "number of mipmaps incorrect");
        GEP_ASSERT(loader.getImages()[0][0].length() > 0, "no image data loaded");
    }
    catch(DDSLoadingException& ex)
    {
      GEP_ASSERT(0, ex.what());
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks

    // DXT5 compressed image with mipmaps
    try
    {
        DDSLoader loader(&g_simpleLeakCheckingAllocator);

        loader.loadFile("testdata\\dxt5_with_mipmaps.dds");
        GEP_ASSERT(loader.getImages().length() == 1, "number of textures incorrect");
        GEP_ASSERT(loader.getImages()[0].length() == 7, "number of mipmaps incorrect");
        for(int i=0; i<7; i++)
        {
            GEP_ASSERT(loader.getImages()[0][i].length() > 0, "no image data loaded");
        }
    }
    catch(DDSLoadingException& ex)
    {
        GEP_ASSERT(0, ex.what());
    }
    SimpleLeakCheckingAllocator::destroyInstance(); //checks for memory leaks
  }
}
