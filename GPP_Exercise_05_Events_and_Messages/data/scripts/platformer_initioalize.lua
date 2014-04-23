print("executing platformer_init.lua")


-- Physics World
local cinfo = WorldCInfo()
cinfo.gravity = Vec3(0, 0, -9.81)
cinfo.worldSize = 2000.0
local world = PhysicsFactory:createWorld(cinfo)
PhysicsSystem:setWorld(world)

-- Camera
Cam:setPosition(Vec3(-300.0, 0.0, 0.0))

-- PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

guid = 1
function nextGUID()
	local guid_string = tostring(guid)
	guid=guid+1
	return guid_string
end

ground = {}
ground.go = GameObjectManager:createGameObject("ground")
ground.pc = ground.go:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(1000, 1000, 10))
cinfo.motionType = MotionType.Fixed
ground.rb = ground.pc:createRigidBody(cinfo)

camera = {}
camera.go = GameObjectManager:createGameObject(nextGUID())
camera.cc = camera.go:createCameraComponent()
camera.cc:setPosition(Vec3(0,-300,200))
camera.cc:lookAt(Vec3(0,0,0))
camera.cc:setActive()

character = {}
character.go = GameObjectManager:createGameObject("character")
character.pc = character.go:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createSphere(25)
cinfo.motionType = MotionType.Dynamic
cinfo.restitution = 0
cinfo.position = Vec3(0,0,100)
cinfo.friction = 0
cinfo.gravityFactor = 10
cinfo.mass = 90
cinfo.maxLinearVelocity = 50
cinfo.linearDamping = 1

character.rb = character.pc:createRigidBody(cinfo)

character.pc:getContactPointEvent():registerListener(collisionCharacter)

character.sc = character.go:createScriptComponent()
character.sc:setUpdateFunction(updateCharacter)





Events.Update:registerListener(update)



