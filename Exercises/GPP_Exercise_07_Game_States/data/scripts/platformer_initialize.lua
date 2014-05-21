
print("initializing platformer")

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
	guid = guid + 1
	return guid_string
end

ground = {}
ground.go = GameObjectManager:createGameObject(nextGUID())
ground.pc = ground.go:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(1000, 1000, 10))
cinfo.motionType = MotionType.Fixed
ground.rb = ground.pc:createRigidBody(cinfo)

camera = {}
camera.go = GameObjectManager:createGameObject(nextGUID())
camera.cc = camera.go:createCameraComponent()
local lookAt = Vec3(0, 0, 50)
camera.cc:lookAt(lookAt)
camera.posOffset = Vec3(0, -400, 400)
camera.cc:setPosition(lookAt + camera.posOffset)
camera.cc:setState(ComponentState.Active)

character = {}
character.go = GameObjectManager:createGameObject(nextGUID())
character.pc = character.go:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createSphere(25)
cinfo.motionType = MotionType.Dynamic
cinfo.restitution = 0
cinfo.friction = 0
cinfo.gravityFactor = 100
cinfo.mass = 90
cinfo.maxLinearVelocity = 500
cinfo.linearDamping = 1
character.rb = character.pc:createRigidBody(cinfo)
-- collision event
character.pc:getContactPointEvent():registerListener(collisionCharacter)
character.sc = character.go:createScriptComponent()
character.sc:setUpdateFunction(updateCharacter)
character.go:setComponentStates(ComponentState.Inactive)
character.grounded = false
character.alive = false

enemy = {}
enemy.go = GameObjectManager:createGameObject(nextGUID())
enemy.pc = enemy.go:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(40, 40, 40))
cinfo.motionType = MotionType.Keyframed
enemy.rb = enemy.pc:createRigidBody(cinfo)
enemy.sc = enemy.go:createScriptComponent()
enemy.sc:setUpdateFunction(updateEnemy)
enemy.go:setComponentStates(ComponentState.Inactive)
enemy.alive = false
enemy.direction = Vec3(0, 0, 0)
enemy.halfHeight = 35
-- register global update function
Events.Update:registerListener(update)

math.randomseed(os.clock())
