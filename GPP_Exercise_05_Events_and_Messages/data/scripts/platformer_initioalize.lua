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
cinfo.shape = PhysicsFactory:createBox(15,15,15)
cinfo.motionType = MotionType.Dynamic
cinfo.restitution = 0
cinfo.position = Vec3(0,0,10)
cinfo.friction = 0
cinfo.gravityFactor = 10
cinfo.mass = 250
cinfo.maxLinearVelocity = 50000
cinfo.linearDamping = 1

character.rb = character.pc:createRigidBody(cinfo)

character.pc:getContactPointEvent():registerListener(collisionCharacter)

character.sc = character.go:createScriptComponent()
character.sc:setUpdateFunction(updateCharacter)

for variable = 0, 4000 do
	
	box = {}
	box.go = GameObjectManager:createGameObject(nextGUID())
	box.pc = box.go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	--cinfo.shape = PhysicsFactory:createSphere(math.random(1,7))
	cinfo.shape = PhysicsFactory:createBox(Vec3(math.random(1,6), math.random(1,6), math.random(1,6)))
	cinfo.motionType = MotionType.Dynamic
	cinfo.restitution = 1
	cinfo.position = Vec3(math.random(-500,500),math.random(-500,500),math.random(10,20))
	quat= Quaternion()
	quat.x=math.random(0.1,1.0)
	quat.y=math.random(0.1,1.0)
	quat.z=math.random(0.1,1.0)
	quat.angle=math.sin(math.random(0,3.1412))
	quat = quat:normalized()
	cinfo.rotation = quat
	cinfo.friction = 1
	cinfo.gravityFactor = 0.1
	cinfo.mass = math.random(1,10)
	cinfo.maxLinearVelocity = 50
	cinfo.linearDamping = 1
	box.rb = box.pc:createRigidBody(cinfo)
	
end



Events.Update:registerListener(update)



