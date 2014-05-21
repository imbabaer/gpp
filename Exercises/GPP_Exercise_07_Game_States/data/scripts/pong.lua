print("Executing pong.lua")

--World
local cinfo = WorldCInfo()
cinfo.gravity = Vec3(0.0, 0.0, -9.81)
cinfo.worldSize = 2000.0
local world = PhysicsFactory:createWorld(cinfo)
PhysicsSystem:setWorld(world)


--PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

--// strip-start "PongImpl"

local playerSpeed = 200.0
local playerRotSpeed = 2.5
local maxLinearVelocity = 350

p1 = GameObjectManager:createGameObject("player1")
p2 = GameObjectManager:createGameObject("player2")
ball = GameObjectManager:createGameObject("ball")
cam = GameObjectManager:createGameObject("camera")

function initializePlayer(go)
--	local renderComponent = go:createRenderComponent()
--	renderComponent:setPath("data/models/ball.thModel")
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(7.5, 7.5, 25.0)

	cinfo.motionType = MotionType.Keyframed
	cinfo.restitution = 1.0
	physicsComponent:createRigidBody(cinfo)
	local scriptComponent = go:createScriptComponent()
	return go
end

function initializeBall(go)
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createSphere(7,5)
	cinfo.motionType = MotionType.Dynamic
	cinfo.mass = 1.0
	cinfo.friction = 0.5
	cinfo.restitution = 1.0
	cinfo.linearVelocity = Vec3(0, -90, 25)
	cinfo.angularVelocity = Vec3(0, 15,0)
	cinfo.maxLinearVelocity = maxLinearVelocity
	physicsComponent:createRigidBody(cinfo)
	local scriptComponent = go:createScriptComponent()
	return go
end

function createBorder()
	--TOP
	local go = GameObjectManager:createGameObject("borderTop")
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(1.0,500.0,1.0)
	cinfo.motionType = MotionType.Fixed
	cinfo.friction = 0.0
	cinfo.angularDamping = 0.0
	cinfo.restitution = 1.0
	physicsComponent:createRigidBody(cinfo)
	go:setPosition(Vec3(0.0,0.0,125.0))
	--BOTTOM
	local go = GameObjectManager:createGameObject("borderBottom")
	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(1.0,500.0,1.0)
	cinfo.motionType = MotionType.Fixed
	cinfo.friction = 0.0
	cinfo.angularDamping = 0.0
	cinfo.restitution = 1.0
	physicsComponent:createRigidBody(cinfo)
	go:setPosition(Vec3(0.0,0.0,-125.0))
end


function p1.update(gameObjectName, elapsedMilliseconds)
	local pos = p1:getPosition()
	local vel = Vec3(0.0, 0.0, 0.0)
	local angVel = Vec3(0.0, 0.0, 0.0)

	--Move Up
	if (InputHandler:isPressed(Key.W) and pos.z < 105) then vel.z = playerSpeed end
	--Move Down
	if (InputHandler:isPressed(Key.S) and pos.z > -105) then vel.z = -playerSpeed end
	--Move Left
	if (InputHandler:isPressed(Key.A) and pos.y < 295) then vel.y = playerSpeed end
	--Move Right
	if (InputHandler:isPressed(Key.D) and pos.y > 100) then vel.y = -playerSpeed end
	--Rotate Left
	if InputHandler:isPressed(Key.E) then angVel.x = playerRotSpeed end
	--Rotate Right
	if InputHandler:isPressed(Key.Q) then angVel.x =  -playerRotSpeed end

	p1:getPhysicsComponent():getRigidBody():setLinearVelocity(vel)
	p1:getPhysicsComponent():getRigidBody():setAngularVelocity(angVel)
end

function p2.update(gameObjectName, elapsedMilliseconds)
	local pos = p2:getPosition()
	local vel = Vec3(0.0, 0.0, 0.0)
	local angVel = Vec3(0.0, 0.0, 0.0)

	--Move Up
	if (InputHandler:isPressed(Key.Up) and pos.z < 105) then vel.z = playerSpeed end
	--Move Down
	if (InputHandler:isPressed(Key.Down) and pos.z > -105) then vel.z = -playerSpeed end
	--Move Left
	if (InputHandler:isPressed(Key.Left) and pos.y < -100) then vel.y = playerSpeed end
	--Move Right
	if (InputHandler:isPressed(Key.Right) and pos.y > -295) then vel.y = -playerSpeed end
	--Rotate Left
	if InputHandler:isPressed(Key.O) then angVel.x = playerRotSpeed end
	--Rotate Right
	if InputHandler:isPressed(Key.P) then angVel.x = -playerRotSpeed end

	p2:getPhysicsComponent():getRigidBody():setLinearVelocity(vel)
	p2:getPhysicsComponent():getRigidBody():setAngularVelocity(angVel)
end

function ball.update(gameObjectName, elapsedMilliseconds)
	--prevent the ball from leaving the 2 dimensional world
	local vel = ball:getPhysicsComponent():getRigidBody():getLinearVelocity()
	ball:getPhysicsComponent():getRigidBody():setLinearVelocity(Vec3(0, vel.y, vel.z))
	local pos = ball:getPosition()
	ball:setPosition(Vec3(0.0, pos.y, pos.z))

	--player two misses
	if pos.y < -300 then
		p1.score = p1.score + 1
		ball:setPosition(Vec3(0, 0, 0))
		ball:getPhysicsComponent():getRigidBody():setLinearVelocity(Vec3(0, 120, math.random(0,50)))
	end

	--player one misses
	if pos.y > 300 then
		p2.score = p2.score + 1
		ball:setPosition(Vec3(0, 0, 0))
		ball:getPhysicsComponent():getRigidBody():setLinearVelocity(Vec3(0, -120, math.random(0,50)))
	end

	DebugRenderer:printText(Vec2(-0.9, 0.85), "Player 1 Score: " .. p1.score)
	DebugRenderer:printText(Vec2(0.7, 0.85), "Player 2 Score: " .. p2.score)
end


--Player 1
initializePlayer(p1)
p1:setPosition(Vec3(0,250,0))
p1:getScriptComponent():setUpdateFunction(p1.update)
p1.score = 0

--player 2
initializePlayer(p2)
p2:setPosition(Vec3(0,-250,0))
p2:getScriptComponent():setUpdateFunction(p2.update)
p2.score = 0

--ball
initializeBall(ball)
ball:getScriptComponent():setUpdateFunction(ball.update)

--borders
createBorder()

cameraComponent = cam:createCameraComponent()
cameraComponent:setPosition(Vec3(-300,0,0))
cameraComponent:lookAt(Vec3(0,0,0))
cameraComponent:setState(ComponentState.Active)


--// strip-end
