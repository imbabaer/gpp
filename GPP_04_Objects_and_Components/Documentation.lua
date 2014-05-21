--[[

### File structure

The global script code will be executed only once. This means your
initialization code can be placed right in the executed script.
For example you can create some GameObjects,
register your update functions for them and they are
afterwards used by the game engine to simulate your world.


### GameObjects

GameObjects can be created via the global GameObjectManager.
Already existing GameObjects can be retrieved by name.

	local go = GameObjectManager:createGameObject("GameObjectName")
	local go = GameObjectManager:getGameObject("GameObjectName")

Afterwards, you can use available member functions on objects,
attach components, etc.
Additionally, you can set and get an object's position with the member
functions

	go:setPosition(Vec3(0.0, 0.0, 0.0))
	local position = go:getPosition()

	
### Components

Components can be created and attached to GameObjects directly
with a given GameObject. The usage schema looks like

	local xxxComponent = go:createXxxComponent()
	local xxxComponent = go:getXxxComponent()

	
## ScriptComponent

The script component is needed to be able to register update functions
for your objects. This is done in the following manner:
	
	local scriptComponent = go:createScriptComponent()
	
	scriptComponent:setUpdateFunction("go_update")
or
	go:getScriptComponent():setUpdateFunction("go_update")
	
Where "go_update" is the name of an existing Lua function that takes
two parameters: The corresponding GameObject's GUID and the elapsed time
in milliseconds:

	function go_update(gameObjectName, elapsedMilliseconds)
		-- TODO place update code here
	end


## PhysicsComponent

PhysicsComponents own a RigidBody, that you can create
with a given PhysicsComponent. However, a RigidBody needs a set
of informations that describe its physical behaviour. This is done via
a RigidBodyCInfo object. The most important RigidBodyCInfo attributes are listed below.

	local physicsComponent = go:createPhysicsComponent()
	local cinfo = RigidBodyCInfo()
	cinfo.shape = PhysicsFactory:createBox(1, 1, 1)
	cinfo.mass = 10
	cinfo.motionType = MotionType.Dynamic
	cinfo.restitution = 1
	cinfo.friction = 1
	cinfo.angularDamping = 1
	cinfo.linearVelocity = Vec3(1, 1, 1)
	cinfo.angularVelocity = Vec3(1, 0, 1)
	physicsComponent:createRigidBody(cinfo)

The MotionType is a global enum data structure:

	MotionType.Dynamic
	MotionType.MotionKeyframed
	MotionType.Fixed

RigidBodies are static by default.

The only available Shapes for this lesson are boxes and spheres. You
can create them via the PhysicsFactory global. The createBox method
takes a Vec3, defining the half extends of a box. The function createSphere expects a radius.

	boxShape = PhysicsFactory:createBox(Vec3(1, 1, 1))
	sphereShape = PhysicsFactory:createSphere(1)


### KEYS

The global InputHandler lets you query the keyboard state.

	InputHandler:isPressed(0x53)
	InputHandler:wasTriggered(0x53)
	
The parameters the two functions take are virtual key codes, which you can look
up here: http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

All keys are defined in a global Lua table called 'Keys'. For example:

	Keys.Space
	Keys.Return
	Keys.A
	Keys._0		-- number key identifiers begin with an underscore

You can simply call, for example:
	
	InputHandler:isPressed(Keys.Left)

	
### ON-SCREEN TEXT

While your GameObjects will be drawn automatically, you can use the
global DebugRenderer to print Text on a certain screen position.

	DebugRenderer:printText(Vec2(0, 0), "Hello World")

The positions on the screen are defined in normalized screen space,
where (0, 0) is the center. Consequently, (-1, -1) is the bottom left
and (1, 1) is the top right.

	
--]]
