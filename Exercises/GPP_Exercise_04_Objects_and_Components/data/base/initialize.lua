-- TODO documentation

-- For more convenience
function Scripting:registerScript(name, options)
	self:_registerScript(name, options or ScriptLoadOptions.Default)
end

print("Initializing from lua...")


Scripting:registerScript("pong.lua")


print("Finished initializing from lua.")
