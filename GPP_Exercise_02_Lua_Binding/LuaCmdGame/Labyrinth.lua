
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

-- is called once upon start-up
function initialize()

	playField =
	{
		{"X","X","X","X","X","X","X","X","X","X","X","X","X","X","X"},
		{"O"," ","X"," "," "," ","X","X"," "," "," "," ","X"," "," "},
		{"X"," ","X"," ","X"," ","X"," "," ","X","X"," "," "," ","X"},
		{"X"," ","X"," ","X"," ","X"," ","X","X","X","X","X","X","X"},
		{"X"," ","X"," ","X"," ","X"," "," "," "," ","X","X"," ","X"},
		{"X"," "," "," ","X"," ","X"," ","X","X"," "," "," "," ","X"},
		{"X"," ","X","X","X"," ","X","X","X","X","X","X"," ","X","X"},
		{"X"," ","X"," ","X"," ","X"," ","X","X"," "," "," "," ","X"},
		{"X"," ","X"," "," "," ","X"," "," ","X"," ","X","X"," ","X"},
		{"X"," ","X"," ","X","X","X","X"," "," "," "," ","X"," ","X"},
		{"X","X","X"," ","X"," ","X","X","X","X","X"," ","X"," ","X"},
		{"X"," "," "," ","X"," "," "," "," "," "," "," ","X"," ","X"},
		{"X"," ","X","X","X"," ","X","X"," ","X","X"," ","X"," ","X"},
		{"X"," "," "," "," "," "," ","X"," ","X"," "," ","X"," ","X"},
		{"X","X","X","X","X","X","X","X","X","X","X","X","X","X","X"}
	}
	playerPos = {x=1, y=2}
	gameState = 0

	setSleepTime(100)
end

-- is called once at the beginning of every frame
-- returns either EXIT_GAME or CONTINUE_GAME to continue or exit the game, respectively.
function update()

	if (gameState == 0) then
		if (isKeyDown(VK_LEFT)) then
			if (playerPos.x > 1 and playField[playerPos.y][playerPos.x-1] == " ") then
				playField[playerPos.y][playerPos.x] = " "
				playerPos.x = playerPos.x - 1
				playField[playerPos.y][playerPos.x] = "O"
			end
		end
		if (isKeyDown(VK_UP)) then
			if (playerPos.y > 1 and playField[playerPos.y-1][playerPos.x] == " ") then
				playField[playerPos.y][playerPos.x] = " "
				playerPos.y = playerPos.y - 1
				playField[playerPos.y][playerPos.x] = "O"
			end
		end
		if (isKeyDown(VK_RIGHT)) then
			if (playerPos.x < 15 and playField[playerPos.y][playerPos.x+1] == " ") then
				playField[playerPos.y][playerPos.x] = " "
				playerPos.x = playerPos.x + 1
				playField[playerPos.y][playerPos.x] = "O"
			end
		end
		if (isKeyDown(VK_DOWN)) then
			if (playerPos.y < 15 and playField[playerPos.y+1][playerPos.x] == " ") then
				playField[playerPos.y][playerPos.x] = " "
				playerPos.y = playerPos.y + 1
				playField[playerPos.y][playerPos.x] = "O"
			end
		end
		if (playerPos.x == 15 and playerPos.y == 2) then
			gameState = 1
		end
	end

	if (wasKeyPressed(VK_Q)) then
		return EXIT_GAME
	end
	return CONTINUE_GAME
end

-- is called once at the end of every frame
function draw()

	print("\n***** Labyrinth.lua *****")

	print("")
	for y, v in ipairs(playField) do
		io.write("     ")
		for x, v in ipairs(playField[y]) do
			io.write(playField[y][x])
		end
		print("")
	end
	if (gameState == 1) then
		print("")
		print("     you won!")
		print("     press q to quit")
	end
end
