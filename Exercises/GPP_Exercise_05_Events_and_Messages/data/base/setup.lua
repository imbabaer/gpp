-- For more convenience
function Scripting:registerScript(name, options)
	self:_registerScript(name, options or ScriptLoadOptions.Default)
end

-- Table for all global events
Events = {}

-- Global update event
Events.Update = _UpdateEvent

-- other utilities

function math.sign(number)
	if number < 0 then
		return -1
	elseif number > 0 then
		return 1
	else
		return 0
	end
end
