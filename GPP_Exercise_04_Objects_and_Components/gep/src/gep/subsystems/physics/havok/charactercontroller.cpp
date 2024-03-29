#include "stdafx.h"

#include "gepimpl/subsystems/physics/havok/entity.h"
#include "gepimpl/subsystems/physics/havok/characterController.h"

#include "gepimpl/subsystems/physics/havok/conversion/shape.h"
#include "gepimpl/subsystems/physics/havok/conversion/characterInputAndOutput.h"
#include "gepimpl/subsystems/physics/havok/conversion/surfaceInfo.h"

#include "gepimpl/havok/util.h"


gep::HavokCharacterRigidBody::HavokCharacterRigidBody(const CharacterRigidBodyCInfo& cinfo) :
    m_pRigidBody(nullptr),
    m_pHkCharacterRigidBody(nullptr)
{
    {
        hkpCharacterRigidBodyCinfo hkcinfo;

        hkcinfo.m_mass = cinfo.mass;
        hkcinfo.m_maxForce = cinfo.maxForce;
        hkcinfo.m_friction = cinfo.friction;
        hkcinfo.m_maxSlope = cinfo.maxSlope;
        hkcinfo.m_unweldingHeightOffsetFactor = cinfo.unweldingHeightOffsetFactor;
        conversion::hk::toHkVector4(cinfo.up, hkcinfo.m_up);
        hkcinfo.m_maxLinearVelocity = cinfo.maxLinearVelocity;
        hkcinfo.m_allowedPenetrationDepth = cinfo.allowedPenetrationDepth;
        hkcinfo.m_maxSpeedForSimplexSolver = cinfo.maxSpeedForSimplexSolver;
        hkcinfo.m_collisionFilterInfo = cinfo.collisionFilterInfo;
        conversion::hk::toHkVector4(cinfo.position, hkcinfo.m_position);
        conversion::hk::toHkQuaternion(cinfo.rotation, hkcinfo.m_rotation);
        hkcinfo.m_supportDistance = cinfo.supportDistance;
        hkcinfo.m_hardSupportDistance = cinfo.hardSupportDistance;
        hkcinfo.m_shape = conversion::hk::toHkShape(cinfo.shape);

        m_pHkCharacterRigidBody = new hkpCharacterRigidBody(hkcinfo);
        if(hkcinfo.m_shape) hkcinfo.m_shape->removeReference();
    }

    // Set default character listener
    {
        hkpCharacterRigidBodyListener* listener = new hkpCharacterRigidBodyListener();
        m_pHkCharacterRigidBody->setListener( listener );
        listener->removeReference();
    }

    // Create an internal havok rigid body from the existing rigid body so we can use this class's getRigidBody.
    m_pRigidBody = new HavokRigidBody(m_pHkCharacterRigidBody->getRigidBody());
    
    // Create the Character state machine and context
    {
        hkpCharacterState* state;
        hkpCharacterStateManager* manager = new hkpCharacterStateManager();

        state = new hkpCharacterStateOnGround();
        manager->registerState(state, HK_CHARACTER_ON_GROUND);
        state->removeReference();

        state = new hkpCharacterStateInAir();
        manager->registerState(state, HK_CHARACTER_IN_AIR);
        state->removeReference();

        state = new hkpCharacterStateJumping();
        manager->registerState(state, HK_CHARACTER_JUMPING);
        state->removeReference();

        state = new hkpCharacterStateClimbing();
        manager->registerState(state, HK_CHARACTER_CLIMBING);
        state->removeReference();

        m_pHkCharacterContext = new hkpCharacterContext(manager, HK_CHARACTER_ON_GROUND );
        manager->removeReference();

        // Set character type
        m_pHkCharacterContext->setCharacterType(hkpCharacterContext::HK_CHARACTER_RIGIDBODY);
    }
}

gep::HavokCharacterRigidBody::~HavokCharacterRigidBody()
{
    DELETE_AND_NULL(m_pRigidBody);
    GEP_HK_REMOVE_REF_AND_NULL(m_pHkCharacterRigidBody);
    GEP_HK_REMOVE_REF_AND_NULL(m_pHkCharacterContext);
}

void gep::HavokCharacterRigidBody::initialize()
{
    m_pRigidBody->initialize();
}

void gep::HavokCharacterRigidBody::destroy()
{
    m_pRigidBody->destroy();
}

void gep::HavokCharacterRigidBody::update(const CharacterInput& input, CharacterOutput& output)
{
    hkpCharacterInput hkinput;
    hkpCharacterOutput hkoutput;

    conversion::hk::toHkCharacterInput(input, hkinput);
    m_pHkCharacterContext->update(hkinput, hkoutput);
    conversion::hk::fromHkCharacterOutput(hkoutput, output);
}

void gep::HavokCharacterRigidBody::checkSupport(float deltaTime, SurfaceInfo& surfaceinfo)
{
    hkStepInfo step;
    step.m_deltaTime = deltaTime / 1000.0f;
    step.m_invDeltaTime = 1.0f/step.m_deltaTime;

    hkpSurfaceInfo surface;
    conversion::hk::toHkSurfaceInfo(surfaceinfo, surface);

    m_pHkCharacterRigidBody->checkSupport(step, surface);
    conversion::hk::fromHkSurfaceInfo(surface, surfaceinfo);
}

gep::IRigidBody* gep::HavokCharacterRigidBody::getRigidBody() const
{
    return m_pRigidBody;
}

void gep::HavokCharacterRigidBody::setLinearVelocity(const vec3& newVelocity, float deltaTime)
{
    hkVector4 linearVelocity;
    conversion::hk::toHkVector4(newVelocity, linearVelocity);
    m_pHkCharacterRigidBody->setLinearVelocity(linearVelocity, deltaTime);
}

gep::CharacterState::Enum gep::HavokCharacterRigidBody::getState() const
{
    return static_cast<CharacterState::Enum>(m_pHkCharacterContext->getState());
}
