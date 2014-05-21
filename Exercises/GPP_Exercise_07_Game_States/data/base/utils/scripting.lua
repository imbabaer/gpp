
-- For more convenience
function Scripting:registerScript(name, options)
	self:_registerScript(name, options or ScriptLoadOptions.Default)
end

function Scripting:loadScript(name, options)
	self:_loadScript(name, options or ScriptLoadOptions.Default)
end

function debugBreak(message)
	Scripting:_debugBreak(message or "[LUA DEBUG BREAK]")
end
