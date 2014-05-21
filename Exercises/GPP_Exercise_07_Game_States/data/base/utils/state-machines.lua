
-- used by the state and state machine creator function
local function setLeaveCondition(instance, cinfo)
	local leaveCondition = cinfo.leaveCondition or function() return false end
	assert(type(leaveCondition) == "function", "A leave condition must be a function!")
	instance:setLeaveCondition(leaveCondition)
end

-- used by the state and state machine creator function
local function setAllEventListeners(instance, cinfo)
	-- if no listeners are specified, return
	if not cinfo.eventListeners then return end

	local event = nil
	local eventNames = {
		{ name = "enter", getter = "getEnterEvent" },
		{ name = "leave", getter = "getLeaveEvent" },
		{ name = "update", getter = "getUpdateEvent" },
	}

	for _, eventName in ipairs(eventNames) do
		local listeners = cinfo.eventListeners[eventName.name]
		if listeners then
			event = instance[eventName.getter](instance)
			for _, listener in ipairs(listeners) do
				assert(listener, "The listener must not be nil!")
				assert(type(listener) == "function", "A listener must be a function!")
				event:registerListener(listener)
			end
		end
	end
end

-- used by the state machine creator function
local function createAllStates(instance, cinfo)
	-- if there are no states, return
	if not cinfo.states then return end

	for _, state in ipairs(cinfo.states) do
		-- Creates the state instance add it to the state machine
		state:create(instance)
	end
end

-- used by the state machine creator function
local function createAllTransitions(instance, cinfo)
	for _,transition in ipairs(cinfo.transitions) do
		-- If there is no condition, make one that always returns true
		local condition = transition.condition or function() return true end
		instance:addTransition(transition.from,
								transition.to,
								condition)
	end
end

State = {}
setmetatable(State, State)
function State:__call(cinfo)
	cinfo.type = "state"

	function cinfo:create(factory)
		local instance = nil
		instance = factory:createState(self.name)
		instance.type = self.type

		setLeaveCondition(instance, self)
		setAllEventListeners(instance, self)

		return instance
	end

	return cinfo
end

-- state machine creation helper
StateMachine = {}
setmetatable(StateMachine, StateMachine)
function StateMachine:__call(cinfo)
	cinfo.type = "stateMachine"

	function cinfo:create(factory)
		local instance = nil

		if factory then
			instance = factory:createStateMachine(self.name)
		else
			instance = Game:getStateMachineFactory():create(self.name)
		end
		instance.type = self.type

		--setLeaveCondition(instance, self)
		setAllEventListeners(instance, self)
		createAllStates(instance, self)
		createAllTransitions(instance, self)

		return instance
	end

	return cinfo
end
