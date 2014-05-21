logMessage("Initializing state machine sample ...")

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

logMessage("Creating state A")

local myState = State{
	name = "A",
	leaveCondition = function() return true end, -- never leave
	eventListeners = {
		enter = {
			function(enterData) logMessage("A: First enter listener") return EventResult.Handled end,
			function(enterData) logMessage("A: Second enter listener") return EventResult.Handled end,
		},
		leave = {
			function(leaveData) logMessage("A: First leave listener") return EventResult.Handled end,
			function(leaveData) logMessage("A: Second leave listener") return EventResult.Handled end,
		},
		update = {
			function(updateData) logMessage("A: First update listener") return EventResult.Handled end,
			function(updateData) logMessage("A: Second update listener") return EventResult.Handled end,
		},
	}
}:create(state_gameRunning)


logMessage("Creating state machine innerFsm")

local myInnerStateMachine = StateMachine{
	name = "innerFsm",
	leaveCondition = function() return true end, -- never leave
	eventListeners = {
		enter = {
			function(enterData) logMessage("innerFsm: First enter listener") return EventResult.Handled end,
			function(enterData) logMessage("innerFsm: Second enter listener") return EventResult.Handled end,
		},
		leave = {
			function(leaveData) logMessage("innerFsm: First leave listener") return EventResult.Handled end,
			function(leaveData) logMessage("innerFsm: Second leave listener") return EventResult.Handled end,
		},
		update = {
			function(updateData) logMessage("innerFsm: First update listener") return EventResult.Handled end,
			function(updateData) logMessage("innerFsm: Second update listener") return EventResult.Handled end,
		},
	},
	states = {
		State{
			name = "inner_A",
			leaveCondition = function() return true end, -- never leave
			eventListeners = {
				enter = {
					function(enterData) logMessage("inner_A: First enter listener") return EventResult.Handled end,
					function(enterData) logMessage("inner_A: Second enter listener") return EventResult.Handled end,
				},
				leave = {
					function(leaveData) logMessage("inner_A: First leave listener") return EventResult.Handled end,
					function(leaveData) logMessage("inner_A: Second leave listener") return EventResult.Handled end,
				},
				update = {
					function(updateData) logMessage("inner_A: First update listener") return EventResult.Handled end,
					function(updateData) logMessage("inner_A: Second update listener") return EventResult.Handled end,
				},
			}
		},
		State{
			name = "inner_B",
			leaveCondition = function() return InputHandler:wasTriggered(Key.Escape) end, -- leave on ESCAPE
			eventListeners = {
				enter = {
					function(enterData) logMessage("inner_B: First enter listener") return EventResult.Handled end,
					function(enterData) logMessage("inner_B: Second enter listener") return EventResult.Handled end,
				},
				leave = {
					function(leaveData) logMessage("inner_B: First leave listener") return EventResult.Handled end,
					function(leaveData) logMessage("inner_B: Second leave listener") return EventResult.Handled end,
				},
				update = {
					function(updateData) logMessage("inner_B: First update listener") return EventResult.Handled end,
					function(updateData) logMessage("inner_B: Second update listener") return EventResult.Handled end,
				},
			}
		},
	},
	transitions = {
		{ from = "__enter", to = "inner_A", condition = function() return true end },
		{ from = "inner_A", to = "inner_B", condition = function() return true end },
		{ from = "inner_B", to = "__leave", condition = function() return true end },
	},
}:create(state_gameRunning)

logMessage("Creating 'root' transitions")

state_gameRunning:addTransition("__enter", "A", function() return true end)
state_gameRunning:addTransition("A", "innerFsm", function() return true end)
state_gameRunning:addTransition("innerFsm", "__leave", function() return true end)

logMessage("Finished state machine sample initialization.")
