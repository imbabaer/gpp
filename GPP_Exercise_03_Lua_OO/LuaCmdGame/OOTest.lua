
--[[
	API overview

	-- set the sleep time
	-- milliseconds: the sleep time in milliseconds
	setSleepTime(milliseconds)

	-- checks if a key is down
	-- VK_CODE: virtual-key code => http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
	-- returns true if the key is currently down, otherwise false
	bool isKeyDown( VK_CODE )

	-- checks if a key was just pressed
	-- VK_CODE: virtual-key code => http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
	-- returns true if the key was just pressed, otherwise false
	bool wasKeyPressed( VK_CODE )
]]

-- virtual-key codes from http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
VK_LEFT		= 0x25
VK_RIGHT	= 0x27
VK_UP		= 0x26
VK_DOWN		= 0x28
VK_SPACE	= 0x20
VK_RETURN	= 0x0D
VK_Q		= 0x51

-- global variables
MAX_X = 28
MAX_Y = 12

-- is called once upon start-up
function initialize()

	Player:setName("P")
	Player:setPosition(Vec2(MAX_X/2, MAX_Y/2))
	
	setSleepTime(50)
end

-- is called once at the beginning of every frame
-- returns either EXIT_GAME or CONTINUE_GAME to continue or exit the game, respectively.
function update()

	local move = Vec2(0, 0)
	if (isKeyDown(VK_LEFT)) then move.x = -1 end
	if (isKeyDown(VK_RIGHT)) then move.x = 1 end
	if (isKeyDown(VK_UP)) then move.y = -1 end
	if (isKeyDown(VK_DOWN)) then move.y = 1 end

	local newPosition = Player:getPosition() + move
	if (newPosition.x < 0) then newPosition.x = 0 end
	if (newPosition.x > MAX_X) then newPosition.x = MAX_X end
	if (newPosition.y < 0) then newPosition.y = 0 end
	if (newPosition.y > MAX_Y) then newPosition.y = MAX_Y end
	
	Player:setPosition(newPosition)
	
	if (wasKeyPressed(VK_Q)) then
		return EXIT_GAME
	end
	return CONTINUE_GAME
end

-- is called once at the end of every frame
function draw()

	print("\n***** OOTest.lua *****")	-- print automatically adds "\n"
	io.write("\npress q to quit\n")		-- io.write does not add an "\n"
	io.write("press the arrow keys to move\n")

	io.write("\n")
	local position = Player:getPosition()
	for y = 0, position.y - 1 do
		for x = 0, MAX_X do
			io.write(" ")
		end
		io.write("\n")
	end
	for x = 0, position.x - 1 do
		io.write(" ")
	end
	io.write(Player:getName())
	for x = position.x + 1, MAX_X do
		io.write(" ")
	end
	for y = position.y + 1, MAX_Y + 1 do
		for x = 0, MAX_X do
			io.write(" ")
		end
		io.write("\n")
	end	

	io.write("\n")
	directionZero = position - Vec2(0, 0)
	print("distance from { 0,  0}: " .. string.format("%2.2f", directionZero:length()))
	directionMax = position - Vec2(MAX_X, MAX_Y)
	print("distance from {" .. MAX_X .. ", " .. MAX_Y .. "}: " .. string.format("%2.2f", directionMax:length()))
end

-- Vec2 unit test
function test_Vec2()

	if (type(Vec2) ~= "function") then
		print("Vec2: error: global function Vec2 is not defined")
		return -1
	end
	
	local v1 = Vec2(3, 5)
	if (type(v1.x) ~= "number" or  type(v1.y) ~= "number") then
		print("Vec2(3, 5): error: table fields {x, y} are not set properly")
		return -1
	end
	if (v1.x ~= 3 or v1.y ~= 5) then
		print("Vec2(): error: table fields {x, y} have wrong values, expected {3, 5}, got {" .. v1.x .. ", " .. v1.y .. "}")
		return -1
	end
	if (type(getmetatable(v1)) ~= "table") then
		print("Vec2(): error: meta table is not set correctly")
		return -1
	end
	
	local v2 = Vec2({x=7, y=9})
	if (type(v2.x) ~= "number" or  type(v2.y) ~= "number") then
		print("Vec2({x=7, y=9}): error: table fields {x, y} are not set properly")
		return -1
	end
	if (v2.x ~= 7 or v2.y ~= 9) then
		print("Vec2({x=7, y=9}): error: table fields {x, y} have wrong values, expected {7, 9}, got {" .. v2.x .. ", " .. v2.y .. "}")
		return -1
	end
	if (type(getmetatable(v2)) ~= "table") then
		print("Vec2({x=7, y=9}): error: meta table is not set correctly")
		return -1
	end
	
	local v3 = Vec2(v2)
	if (type(v3.x) ~= "number" or  type(v3.y) ~= "number") then
		print("Vec2(v2): error: table fields {x, y} are not set properly")
		return -1
	end
	if (v3.x ~= 7 or v3.y ~= 9) then
		print("Vec2(v2): error: table fields {x, y} have wrong values, expected {7, 9}, got {" .. v3.x .. ", " .. v3.y .. "}")
		return -1
	end
	if (type(getmetatable(v3)) ~= "table") then
		print("Vec2(v2): error: meta table is not set correctly")
		return -1
	end
	if (getmetatable(v1) ~= getmetatable(v2) or getmetatable(v1) ~= getmetatable(v3)) then
		print("v1, v2, v3: error: the tables should all have the same meta table")
		return -1
	end
	
	local Vec2Meta = getmetatable(v1)
	if (Vec2Meta.__index ~= Vec2Meta) then
		print("Vec2Meta: error: field __index is not set correctly")
		return -1
	end
	if (type(Vec2Meta.length) ~= "function" or type(Vec2Meta.__add) ~= "function" or type(Vec2Meta.__sub) ~= "function") then
		print("Vec2Meta: error: the functions are not set correctly")
		return -1
	end
	local length = Vec2(3, 5):length()
	if (length < 5.82 or length >= 5.84) then
		print("Vec2(3, 5):length(): error: result is wrong, expected 5.83, got " .. length)
		return -1
	end
	
	v4 = Vec2(2, 4) + Vec2(3, 5)
	if (v4.x ~= 5 or v4.y ~= 9) then
		print("Vec2(2, 4) + Vec2(3, 5): error: result is wrong, expected {5, 9}, got {" .. v4.x .. ", " .. v4.y .. "}")
		return -1
	end
	v5 = Vec2(7, 2) - Vec2(4, 8)
	if (v5.x ~= 3 or v5.y ~= -6) then
		print("Vec2(7, 2) - Vec2(4, 8): error: result is wrong, expected {3, -6}, got {" .. v5.x .. ", " .. v5.y .. "}")
		return -1
	end

	print("test_Vec2: all tests passed!")
	return 0
end

-- Player unit test
function test_Player()

	if (type(Player) ~= "table") then
		print("Player: error: global table Player is not defined")
		print(type(Player));
		return -1
	end
	if (type(Player.__ptr) ~= "userdata") then
		print("Player.__ptr: error: pointer is not set correctly")
		return -1
	end
	
	if (type(getmetatable(Player)) ~= "table") then
		print("getmetatable(Player): error: meta table is not set correctly")
		return -1
	end
	local PlayerMeta = getmetatable(Player)
	if (PlayerMeta.__index ~= PlayerMeta) then
		print("PlayerMeta: error: field __index is not set correctly")
		return -1
	else
	end
	if (type(PlayerMeta.getName) ~= "function" or type(PlayerMeta.setName) ~= "function" or type(PlayerMeta.getPosition) ~= "function" or type(PlayerMeta.setPosition) ~= "function") then
		print("PlayerMeta: error: the functions are not set correctly")
		return -1
	end

	if (Player:getName() ~= "") then
		print("Player:getName(): error: the initial name must be an empty string")
		return -1
	end
	name = "Destoroyah"
	Player:setName(name)
	if (Player:getName() ~= name) then
		print("Player:setName(): error: the name was not set correctly")
		return -1
	end
	v = Vec2(0, 0)
	Vec2Meta = getmetatable(v)
	if (type(Player:getPosition()) ~= table and getmetatable(Player:getPosition()) ~= Vec2Meta) then
		print("Player:getPosition(): error: the function does not return a proper Vec2 instance")
		print(type(Player:getPosition()))
		return -1		
	end
	position = Player:getPosition()
	if (position.x ~= 0 or position.y ~= 0) then
		print("Player:getPosition(): error: the initial position is wrong, expected {0, 0}, got {" .. position.x .. ", " .. position.y .."}")
		return -1
	end
	newPosition = Vec2(5, 1)
	Player:setPosition(newPosition)
	position = Player:getPosition()
	if (position.x ~= newPosition.x or position.y ~= newPosition.y) then
		print("Player:setPosition(): error: the position is not set correctly")
		print(position.x)
		print(position.y)
		return -1
	end
	
	print("test_Player: all tests passed!")
	return 0
end
