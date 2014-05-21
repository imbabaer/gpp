print("Executing pong.lua")

--Camera
Cam:setPosition(Vec3(-300,0,0))

--PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

--Create Borders
local goJules = GameObjectManager:createGameObject("floor")
local goJulesPhysics = goJules:createPhysicsComponent()
local cinfo = RigidBodyCInfo()
cinfo.shape = PhysicsFactory:createBox(Vec3(10,1000,1))
cinfo.motionType = MotionType.Fixed
cinfo.position = Vec3(0,0,-150)
cinfo.restitution = 1
goJulesPhysics:createRigidBody(cinfo)

local goSky = GameObjectManager:createGameObject("sky")
local goSkyPhysics = goSky:createPhysicsComponent()
local cinfos = RigidBodyCInfo()
cinfos.shape = PhysicsFactory:createBox(Vec3(10,1000,1))
cinfos.motionType = MotionType.Fixed
cinfos.position = Vec3(0,0,150)
cinfos.restitution = 1
goJulesPhysics:createRigidBody(cinfos)

function boxUpdate(guid, elapsedTime)
	local goPlayer1 = GameObjectManager:getGameObject(guid)
	local vel = Vec3(0,0,0)
	local avel = Vec3(0,0,0)
	if(guid == "p1") then
		if(InputHandler:isPressed(Key.W))then
			vel.z = 120
		end
		if(InputHandler:isPressed(Key.S))then
			vel.z = -120
		end
		if(InputHandler:isPressed(Key.D))then
			vel.y = -120
		end
		if(InputHandler:isPressed(Key.A))then
			vel.y = 120
		end
		if(InputHandler:isPressed(Key.Q))then
			avel.x = -1
		end
		if(InputHandler:isPressed(Key.E))then
			avel.x = 1
		end
	elseif (guid == "p2")then
			if(InputHandler:isPressed(Key.Up))then
			vel.z = 120
		end
		if(InputHandler:isPressed(Key.Down))then
			vel.z = -120
		end
		if(InputHandler:isPressed(Key.Right))then
			vel.y = -120
		end
		if(InputHandler:isPressed(Key.Left))then
			vel.y = 120
		end
		if(InputHandler:isPressed(16))then
			avel.x = -1
		end
		if(InputHandler:isPressed(17))then
			avel.x = 1
		end
	end
	goPlayer1:getPhysicsComponent():getRigidBody():setAngularVelocity(avel)
	goPlayer1:getPhysicsComponent():getRigidBody():setLinearVelocity(vel)
end

local goPlayer1 = GameObjectManager:createGameObject("p1")
local goPlayer1Physics = goPlayer1:createPhysicsComponent()
local cinfop1 = RigidBodyCInfo()
cinfop1.shape = PhysicsFactory:createBox(Vec3(20,1,50))
cinfop1.motionType = MotionType.MotionKeyframed
cinfop1.restitution = 1
cinfop1.friction = 1
cinfop1.position = Vec3(0,200,0)
goPlayer1Physics:createRigidBody(cinfop1)

local goPlayer1s = goPlayer1:createScriptComponent()
goPlayer1s:setUpdateFunction("boxUpdate")


local goPlayer2 = GameObjectManager:createGameObject("p2")
local goPlayer1Physics = goPlayer2:createPhysicsComponent()
local cinfop2 = RigidBodyCInfo()
cinfop2.shape = PhysicsFactory:createBox(Vec3(20,1,50))
cinfop2.motionType = MotionType.MotionKeyframed
cinfop2.restitution = 1
cinfop2.friction = 1
cinfop2.position = Vec3(0,-200,0)
goPlayer1Physics:createRigidBody(cinfop2)
local goPlayer2s = goPlayer2:createScriptComponent()
goPlayer2s:setUpdateFunction("boxUpdate")

function ballUpdate(guid, elapsedTime)
	--DebugRenderer:printText(Vec2(0, 0), "updateBall")
	local goTheBall = GameObjectManager:getGameObject(guid)
	local pos = goTheBall:getPosition()
	goTheBall:setPosition(0,pos.y,pos.z)
	if(InputHandler:wasTriggered(Key.Space))then
		goTheBall:setPosition(0,0,math.random(40,120))
		local av = Vec3(0,math.random(-20,20),math.random(-20,20))
		goTheBall:getPhysicsComponent():getRigidBody():setAngularVelocity(av)
		if(math.random(-1,1)>=0)then
			x = -100
		else
			x = 100
		end
		local ev = Vec3(0,x,math.random(-100,100))
		goTheBall:getPhysicsComponent():getRigidBody():setLinearVelocity(ev)
	end
end

local goTheBall = GameObjectManager:createGameObject("theBall")
local goTheBallPhysics = goTheBall:createPhysicsComponent()
local cinfoTheBall = RigidBodyCInfo()
cinfoTheBall.shape = PhysicsFactory:createSphere(10)
cinfoTheBall.motionType = MotionType.Dynamic
cinfoTheBall.mass = 100.1
cinfoTheBall.restitution = 1
cinfoTheBall.friction = 1
--cinfoTheBall.gravityFactor = 10
goTheBallPhysics:createRigidBody(cinfoTheBall)

local goTheBallScript = goTheBall:createScriptComponent()
goTheBallScript:setUpdateFunction("ballUpdate")



