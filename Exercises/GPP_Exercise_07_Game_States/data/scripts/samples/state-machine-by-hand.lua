print("Initializing state machine (by hand) sample ...")

-- Physics World
do
	local cinfo = WorldCInfo()
	cinfo.gravity = Vec3(0, 0, 0)
	cinfo.worldSize = 2000.0
	local world = PhysicsFactory:createWorld(cinfo)
	PhysicsSystem:setWorld(world)
end

-- Camera
cam = GameObjectManager:createGameObject("cam")
cam.cc = cam:createCameraComponent()
cam.cc:setPosition(Vec3(0.0, -300.0, 0.0))
cam.cc:lookAt(Vec3(0.0, 0.0, 0.0))
cam.cc:setState(ComponentState.Active)

local state_gameRunning = Game:getStateMachine():getStateMachine("gameRunning")
local state_A = state_gameRunning:createState("A")
state_A:setLeaveCondition(function() return false end)
state_gameRunning:addTransition("__enter", "A", function() return true end)
state_gameRunning:addTransition("A", "__leave", function() return true end)

state_A:getUpdateEvent():registerListener(function(updateData)
	if InputHandler:wasTriggered(Key.Escape) then
		updateData:setUpdateStepBehavior(StateUpdateStepBehavior.LeaveWithNoConditionChecks)
	end
	return EventResult.Handled
end)

print("Finished state machine (by hand) sample initialization.")
