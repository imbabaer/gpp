#pragma once
#include "gep/interfaces/physics/world.h"
#include "gep/interfaces/events.h"
#include "gep/container/DynamicArray.h"
#include "gep/interfaces/physics/contact.h"
#include "gepimpl/subsystems/physics/havok/contact.h"

namespace gep
{
    class IPhysicsEntity;

    class HavokBaseAction;

    class HavokWorld : public IWorld, public IContactListener
    {
        hkRefPtr<hkpWorld> m_pWorld;

        DynamicArray< SmartPtr<IPhysicsEntity> > m_entities;
        DynamicArray< SmartPtr<ICharacterRigidBody> > m_characters;
        DynamicArray<HavokContactListener*> m_contactListeners;
        HavokContactListener m_actualContactListener;
        gep::Event<gep::ContactPointArgs*> m_event_contactPoint;
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

        virtual void castRay(const RayCastInput& input, RayCastOutput& output) const;

        hkpWorld* getHkpWorld() const { return m_pWorld; }

        virtual gep::Event<gep::ContactPointArgs*>* getContactPointEvent() override;

        virtual void contactPointCallback(const ContactPointArgs& evt) override;
        virtual void collisionAddedCallback(const CollisionArgs& evt) override;
        virtual void collisionRemovedCallback(const CollisionArgs& evt) override;
    };
}
