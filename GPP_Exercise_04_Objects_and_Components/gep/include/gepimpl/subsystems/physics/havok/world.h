#pragma once
#include "gep/interfaces/physics/world.h"
#include "gep/container/DynamicArray.h"

namespace gep
{
    class IPhysicsEntity;

    class HavokBaseAction;

    class HavokWorld : public IWorld
    {
        struct PhysicsEntityWrapper
        {
            IPhysicsEntity* entity;
            bool iOwnIt;
        };

        struct CharacterWrapper
        {
            ICharacterRigidBody* character;
            bool iOwnIt;
        };

        hkpWorld* m_pWorld;

        DynamicArray<PhysicsEntityWrapper> m_entities;
        DynamicArray<CharacterWrapper> m_characters;
    public:
        HavokWorld(const WorldCInfo& cinfo);

        virtual ~HavokWorld();

        virtual void addEntity(IPhysicsEntity* entity, TakeOwnership own = TakeOwnership::yes) override;
        void addEntity(hkpEntity* entity);
        virtual void removeEntity(IPhysicsEntity* entity);
        void removeEntity(hkpEntity* entity);

        virtual void addCharacter(ICharacterRigidBody* character, TakeOwnership own = TakeOwnership::yes) override;
        virtual void removeCharacter(ICharacterRigidBody* character) override;

        void update(float elapsedTime);

        hkpWorld* getHkpWorld() const { return m_pWorld; }

        virtual void castRay(const RayCastInput& input, RayCastOutput& output) const;

    };
}
