#include "stdafx.h"
#include "gepimpl/subsystems/physics/havok/world.h"
#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/conversion/vector.h"
#include "gepimpl/subsystems/physics/havok/action.h"

#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/renderer.h"
#include "gep/interfaces/physics/characterController.h"

gep::HavokWorld::HavokWorld(const WorldCInfo& cinfo) :
    m_pWorld(nullptr)
{
    m_entities.reserve(64);

    //TODO: Check if this has to be freed somehow.
    hkpWorldCinfo worldInfo;
    worldInfo.setupSolverInfo(hkpWorldCinfo::SOLVER_TYPE_4ITERS_MEDIUM);
    conversion::hk::toHkVector4(cinfo.gravity, worldInfo.m_gravity);
    worldInfo.m_broadPhaseBorderBehaviour = hkpWorldCinfo::BROADPHASE_BORDER_FIX_ENTITY;
    worldInfo.setBroadPhaseWorldSize(cinfo.worldSize);
//	worldInfo.m_simulationType = hkpWorldCinfo::SIMULATION_TYPE_DISCRETE;
    m_pWorld = new hkpWorld(worldInfo);

    // Register all collision agents
    // It's important to register collision agents before adding any entities to the world.
    hkpAgentRegisterUtil::registerAllAgents(m_pWorld->getCollisionDispatcher());
}

gep::HavokWorld::~HavokWorld()
{
    for (auto& characterWrapper : m_characters)
    {
        if (characterWrapper.iOwnIt)
        {
            characterWrapper.character->destroy();
            DELETE_AND_NULL(characterWrapper.character);
        }
    }

    for (auto& entityWrapper : m_entities)
    {
        if (entityWrapper.iOwnIt)
        {
            entityWrapper.entity->destroy();
            DELETE_AND_NULL(entityWrapper.entity);
        }
    }

    GEP_HK_REMOVE_REF_AND_NULL(m_pWorld);
}

void gep::HavokWorld::addEntity(IPhysicsEntity* entity, TakeOwnership own)
{
    //TODO: can only add rigid bodies at the moment.
    auto* actualEntity = dynamic_cast<HavokRigidBody*>(entity);
    GEP_ASSERT(actualEntity != nullptr, "Attempted to add wrong kind of entity. (only rigid bodies are supported at the moment)");
    addEntity(actualEntity->getHkpRigidBody());
    PhysicsEntityWrapper wrapper = { entity, own == TakeOwnership::yes };
    m_entities.append(wrapper);
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
    // TODO: Can only remove rigid bodies at the moment.
    auto* actualEntity = dynamic_cast<HavokRigidBody*>(entity);
    GEP_ASSERT(actualEntity != nullptr, "Attempted to remove wrong kind of entity. (only rigid bodies are supported at the moment)");

    // remove the internal wrapper
    for (auto& wrapper : m_entities)
    {
        if (wrapper.entity == entity)
        {
            wrapper.entity = nullptr;
            wrapper.iOwnIt = false;
        }
    }

    // Remove the actual havok entity
    removeEntity(actualEntity->getHkpRigidBody());
}

void gep::HavokWorld::removeEntity(hkpEntity* entity)
{
    m_pWorld->removeEntity(entity);
}


void gep::HavokWorld::addCharacter(ICharacterRigidBody* character, TakeOwnership own /*= TakeOwnership::yes*/)
{
    addEntity(character->getRigidBody(), TakeOwnership::no);
    CharacterWrapper wrapper = { character, own == TakeOwnership::yes };
    m_characters.append(wrapper);
}

void gep::HavokWorld::removeCharacter(ICharacterRigidBody* character)
{
    // remove the internal wrapper
    for (auto& wrapper : m_characters)
    {
        if (wrapper.character == character)
        {
            wrapper.character = nullptr;
            wrapper.iOwnIt = false;
        }
    }

    removeEntity(character->getRigidBody());
}

void gep::HavokWorld::update(float elapsedTime)
{
    GEP_UNUSED(elapsedTime);
    //TODO: tweak this value if havok is complaining too hard about the simulation becoming unstable.
    m_pWorld->stepDeltaTime(g_globalManager.getUpdateFramework()->calcElapsedTimeAverage(60) / 1000.0f);
}

void gep::HavokWorld::castRay(const RayCastInput& input, RayCastOutput& output) const
{
    hkpWorldRayCastInput actualInput;
    hkpWorldRayCastOutput actualOutput;

    // Process input
    conversion::hk::toHkVector4(input.from, actualInput.m_from);
    conversion::hk::toHkVector4(input.to, actualInput.m_to);

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
