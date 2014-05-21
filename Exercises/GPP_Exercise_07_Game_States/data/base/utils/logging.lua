
function logMessage(...)
	local message = ""
	for _,arg in ipairs({...}) do
		message = message .. arg
	end
	print(message)
end

function prettyPrint(...)
	local args = { ... }
	for _,arg in ipairs(args) do
		if type(arg) == "table" then
			for k,v in pairs(arg) do
				print(k,v)
			end
		else
			print(arg)
		end
	end
end
