local utilsDir = Scripting:getImportantScriptsRoot() .. "utils/"

-- executes important scripts that live in the data/base/utils/ dir
-- Note: this function is local to this script!
local function include(utilsFile)
	local scriptFile = utilsDir .. utilsFile
	dofile(scriptFile)
end

include("debugging.lua")
include("null.lua")
include("logging.lua")
include("math.lua")
include("scripting.lua")
include("events.lua")
include("state-machines.lua")
