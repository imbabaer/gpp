#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/world.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/action.h"
#include "gepimpl/subsystems/physics/havok/contact.h"

#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/physics/characterController.h"

gep::HavokWorld::HavokWorld(const WorldCInfo& cinfo) :
    m_pWorld(nullptr),
    m_entities(),
    m_characters(),
    m_actualContactListener(this),
    m_event_contactPoint()
{
    m_entities.reserve(64);

    hkpWorldCinfo worldInfo;
    worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
    conversion::hk::to(cinfo.gravity, worldInfo.m_gravity);
    worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY;
    worldInfo.setBroadPhaseWorldSize(cinfo.worldSize);
//	worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_DISCRETE;
    m_pWorld = new hkpWorld(worldInfo);

    // Register all collision agents
    // It's important to register collision agents before adding any entities to the world.
    hkpAgentRegisterUtil::registerAllAgents(m_pWorld->getCollisionDispatcher());

    m_pWorld->addContactListener(&m_actualContactListener);
}

gep::HavokWorld::~HavokWorld()
{
    m_pWorld->removeContactListener(&m_actualContactListener);
}

void gep::HavokWorld::addEntity(IPhysicsEntity* entity)
{
    //TODO: can only add rigid bodies at the moment.
    auto* actualEntity = dynamic_cast<HavokRigidBody*>(entity);
    GEP_ASSERT(actualEntity != nullptr, "Attempted to add wrong kind of entity. (only rigid bodies are supported at the moment)");
    addEntity(actualEntity->getHkpRigidBody());
    m_entities.append(entity);
}

void gep::HavokWorld::addEntity(hkpEntity* entity)
{
    m_pWorld->addEntity(entity);
    auto* action = new HavokRigidBodySyncAction();
    action->setEntity(entity);
    m_pWorld->addAction(action);
    action->removeReference();
}

void gep::HavokWorld::removeEntity(IPhysicsEntity* entity)
{
    GEP_ASSERT(entity != nullptr, "Attempt to remove nullptr.");

    // TODO Can only remove rigid bodies at the moment.
    auto* actualEntity = dynamic_cast<HavokRigidBody*>(entity);
    GEP_ASSERT(actualEntity != nullptr, "Attempt to remove wrong kind of entity. (only rigid bodies are supported at the moment)");

    size_t index;
    for (index = 0; index < m_entities.length(); ++index)
    {
        auto& entityPtr = m_entities[index];
        if (entityPtr.get() == actualEntity)
        {
            break;
        }
    }
    GEP_ASSERT(index < m_entities.length(), "Attempt to remove character from world that does not exist there", actualEntity, index, m_entities.length());
    m_entities.removeAtIndex(index);

    // Remove the actual havok entity
    removeEntity(actualEntity->getHkpRigidBody());
}

void gep::HavokWorld::removeEntity(hkpEntity* entity)
{
    m_pWorld->removeEntity(entity);
}


void gep::HavokWorld::addCharacter(ICharacterRigidBody* character)
{
    addEntity(character->getRigidBody());
    m_characters.append(character);
}

void gep::HavokWorld::removeCharacter(ICharacterRigidBody* character)
{
    GEP_ASSERT(character != nullptr);

    size_t index;
    for (index = 0; index < m_characters.length(); ++index)
    {
        auto& characterPtr = m_characters[index];
        if (characterPtr.get() == character)
        {
            break;
        }
    }
    GEP_ASSERT(index < m_characters.length(), "Attempt to remove character from world that does not exist there", character, index, m_characters.length());
    m_characters.removeAtIndex(index);

    removeEntity(character->getRigidBody());
}

void gep::HavokWorld::update(float elapsedTime)
{
    GEP_UNUSED(elapsedTime);
    //TODO tweak this value if havok is complaining too hard about the simulation becoming unstable.
    m_pWorld->stepDeltaTime(g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60) / 1000.0f);
}

void gep::HavokWorld::castRay(const RayCastInput& input, RayCastOutput& output) const
{
    hkpWorldRayCastInput actualInput;
    hkpWorldRayCastOutput actualOutput;

    // Process input
    conversion::hk::to(input.from, actualInput.m_from);
    conversion::hk::to(input.to, actualInput.m_to);

    // Cast the ray
    m_pWorld->castRay(actualInput, actualOutput);

    // Process output
    output.hitFraction = actualOutput.m_hitFraction;
    // TODO: havok uses "collidables", which form a hierarchy. We have to wrap this as well if we want to have something like hit zones.
    // TODO: Check if it is really ok to const_cast here.
    if (actualOutput.m_rootCollidable)
    {
        output.hitEntity = new HavokCollidable(const_cast<hkpCollidable*>(actualOutput.m_rootCollidable));
    }
}

void gep::HavokWorld::contactPointCallback(const ContactPointArgs& evt)
{
    m_event_contactPoint.trigger(&const_cast<gep::ContactPointArgs&>(evt));
}

void gep::HavokWorld::collisionAddedCallback(const CollisionArgs& evt)
{
    GEP_ASSERT(false, "Not implemented.");
}

void gep::HavokWorld::collisionRemovedCallback(const CollisionArgs& evt)
{
    GEP_ASSERT(false, "Not implemented.");
}

gep::Event<gep::ContactPointArgs*>* gep::HavokWorld::getContactPointEvent()
{
    return &m_event_contactPoint;
}
