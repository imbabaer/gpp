#pragma once
#include "gep/interfaces/subsystem.h"
#include "gep/interfaces/resourceManager.h"

namespace gep
{
    // Forward declarations (declared somewhere else)
    //////////////////////////////////////////////////////////////////////////

    // Forward declarations (declared later in this file)
    //////////////////////////////////////////////////////////////////////////
    class IAnimationFactory;
    class IAnimationResource;

    // Class definitions
    //////////////////////////////////////////////////////////////////////////

    class IAnimationSystem : public ISubsystem
    {
    public:
        virtual ~IAnimationSystem() = 0 {}

        virtual IAnimationFactory* getAnimationFactory() const = 0;
    };

    class IAnimationFactory
    {
    public:
        virtual ~IAnimationFactory() = 0 {}

        virtual ResourcePtr<IAnimationResource> loadAnimation(const char* path) = 0;
    };

    class IAnimationResource : public IResource
    {
    public:
        virtual ~IAnimationResource() = 0 {}
    };
}
