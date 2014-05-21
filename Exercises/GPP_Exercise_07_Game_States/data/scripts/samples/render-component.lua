
print("Initializing render component test...")

-- Physics World
do
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, -9.81)
	cinfo.worldSize = 2000.0
	local world = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(world)
end

-- Camera
cameraObject = GameObjectManager:createGameObject("cameraObject")
cameraObject.cc = cameraObject:createCameraComponent()
cameraObject.cc:setPosition(Vec3(0.0, -300.0, 0.0))
cameraObject.cc:lookAt(Vec3(0.0, 0.0, 0.0))
cameraObject.cc:setState(ComponentState.Active)

distanceFromCamera = 100.0

-- PhysicsDebugView
PhysicsSystem:setDebugDrawingEnabled(true)

do
	renderedObject = GameObjectManager:createGameObject("renderedObject")

	-- render component
	renderedObject.render = renderedObject:createRenderComponent()
	renderedObject.render:setPath("data/models/ball.thModel")
end

Events.Update:registerListener(function(elapsedTime)
	local speed = 0.1
	local movement = Vec3()

	if InputHandler:isPressed(Key.Up)       then movement.z =  speed * elapsedTime end
	if InputHandler:isPressed(Key.Down)     then movement.z = -speed * elapsedTime end
	if InputHandler:isPressed(Key.Left)     then movement.x =  speed * elapsedTime end
	if InputHandler:isPressed(Key.Right)    then movement.x = -speed * elapsedTime end
	if InputHandler:isPressed(Key.Add)      then distanceFromCamera = distanceFromCamera - speed * elapsedTime end
	if InputHandler:isPressed(Key.Subtract) then distanceFromCamera = distanceFromCamera + speed * elapsedTime end

	cameraObject.cc:move(movement)
	cameraObject.cc:setPosition(cameraObject.cc:getPosition():normalized():mulScalar(distanceFromCamera))
	cameraObject.cc:lookAt(Vec3(0.0, 0.0, 0.0))

	return EventResult.Handled
end)

print("Finished render component test initialization.")
