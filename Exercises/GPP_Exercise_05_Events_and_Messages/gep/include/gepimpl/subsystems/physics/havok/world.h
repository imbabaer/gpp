#pragma once
#include "gep/interfaces/physics/world.h"
#include "gep/container/DynamicArray.h"

namespace gep
{
    class IPhysicsEntity;

    class HavokBaseAction;

    class HavokWorld : public IWorld
    {
        hkpWorld* m_pWorld;

        DynamicArray< SmartPtr<IPhysicsEntity> > m_entities;
        DynamicArray< SmartPtr<ICharacterRigidBody> > m_characters;
    public:
        HavokWorld(const WorldCInfo& cinfo);

        virtual ~HavokWorld();

        virtual void addEntity(IPhysicsEntity* entity) override;
        void addEntity(hkpEntity* entity);
        virtual void removeEntity(IPhysicsEntity* entity);
        void removeEntity(hkpEntity* entity);

        virtual void addCharacter(ICharacterRigidBody* character) override;
        virtual void removeCharacter(ICharacterRigidBody* character) override;

        void update(float elapsedTime);

        hkpWorld* getHkpWorld() const { return m_pWorld; }

        virtual void castRay(const RayCastInput& input, RayCastOutput& output) const;

    };
}
