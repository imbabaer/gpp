logMessage("Initializing state_machine.lua ...")

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

local stateMachine_gameRunning = Game:getStateMachine():getStateMachine("gameRunning")

local state_A = stateMachine_gameRunning:createState("A")


function updateStateA(updateEventData)
	--TODO actual update of A
	logMessage("Updating A. Elapsed time = " .. updateEventData:getElapsedTime() .. " ms")
	
	--logWarning()
	--logError()
	return EventResult.Handled
end

state_A:getUpdateEvent():registerListener(updateStateA)
state_A:setLeaveCondition(function() return InputHandler:wasTriggered(Key.B) end)

local state_B= State({
	name = "B",
	eventListeners = {
		update = {
			function(updateEventData)
				--TODO actual update of B
				logMessage("Updating B part 1. Elapsed time = " .. updateEventData:getElapsedTime() .. " ms")
				return EventResult.Handled
			end,
			function(updateEventData)
				--TODO actual update of B
				logMessage("Updating B part 2. Elapsed time = " .. updateEventData:getElapsedTime() .. " ms")
				return EventResult.Handled
			end
		}
	}
}):create(stateMachine_gameRunning)

local state_C = StateMachine({
	name = "fsm_C",
	states = {
		State{
			name = "CB",
			leaveCondition = function() return InputHandler:wasTriggered(Key.L) end,
			eventListeners = {
				update = {
						function(updateEventData)
							--TODO actual update of B
							logMessage("Updating B part 1. Elapsed time = " .. updateEventData:getElapsedTime() .. " ms")
							return EventResult.Handled
						end,
						function(updateEventData)
							--TODO actual update of B
							logMessage("Updating B part 2. Elapsed time = " .. updateEventData:getElapsedTime() .. " ms")
							return EventResult.Handled
						end
					}
				}
			}
		},
		transitions = {
			{from = "__enter", to = "CB"},
			{from = "CB", to = "__leave"}
		}
	
}):create(stateMachine_gameRunning)

stateMachine_gameRunning:addTransition("__enter", "A",function() return true end)
stateMachine_gameRunning:addTransition("A", "__leave",function() return false end)
stateMachine_gameRunning:addTransition("A", "fsm_C",function() return true end)
stateMachine_gameRunning:addTransition("fsm_C", "__leave",function() return true end)


logMessage("Finished initializing state_machine.lua")
