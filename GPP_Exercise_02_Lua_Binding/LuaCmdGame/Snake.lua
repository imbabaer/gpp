
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

sleeptime = 500

gamestates = { running = 0, gameover = 1 }

-- utility functions

-- creates an object that has the members x, y, preciseX, and preciseY
-- the former 2 are floor'd int values, while the latter are floats
function createPosition(input)
	newPos = input or { x = 0.0, y = 0.0 }
	return {
		preciseX = newPos.preciseX or newPos.x or 0.0,
		preciseY = newPos.preciseY or newPos.y or 0.0,
		x = math.floor(newPos.x) or math.floor(newPos.preciseX) or 0,
		y = math.floor(newPos.y) or math.floor(newPos.preciseY) or 0,
	}
end

-- variables
score = 0

currentGameState = gamestates.running

level = {
	borderSymbol = "#",
	emptySymbol = " ",
	_gridTemplate = { -- don't access this during runtime!
		{ "#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#"," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ","#" },
		{ "#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#","#" },
	},
	grid = nil, -- will be set in updating the level
	innerBounds = nil, -- will be set in initialize()
	tileDimensions = { width = 2, height = 1 },

	-- methods
	initialize = function()
		level.innerBounds = { left = 2, right = #level._gridTemplate[1] - 1, upper = 2, lower = #level._gridTemplate - 1 }
		level.cleanGrid()
	end,
	update = function(elapsedSeconds)
		level.cleanGrid()
	end,
	cleanGrid = function()
		-- copy the grid template
		level.grid = {}
		for rowNumber, row in ipairs(level._gridTemplate) do
			level.grid[rowNumber] = {}
			for colNumber, tile in ipairs(row) do
				level.grid[rowNumber][colNumber] = tile
			end
		end
	end
}

directions = {
	north = { name = "north", x =  0,                          y = -level.tileDimensions.height, inverse = nil },
	south = { name = "south", x =  0,                          y =  level.tileDimensions.height, inverse = nil },
	west =  { name = "west",  x = -level.tileDimensions.width, y =  0,                           inverse = nil },
	east =  { name = "east",  x =  level.tileDimensions.width, y =  0,                           inverse = nil }
}
directions.north.inverse = directions.south
directions.south.inverse = directions.north
directions.west.inverse = directions.east
directions.east.inverse = directions.west

-- entities

snake = {
	speed = 2.0,
	length = function() return #snake.bodyParts end,
	position = createPosition{ x = 5.0 * level.tileDimensions.width, y = 5.0 * level.tileDimensions.height },
	direction = directions.east,
	newDirection = directions.east,

	-- drawing
	headSymbol = "x",
	bodySymbol = "o",

	-- body
	bodyParts = {}, -- is initialized in snake.initialize()

	-- methods
	initialize = function()
		-- set up initial body part
		local newBodyPart = {
			newBodyPart = createPosition(snake.position),
			symbol = snake.bodySymbol
		}
		snake.appendBodyPart(newBodyPart)
	end,
	update = function(elapsedSeconds)
		-- set the snake's direction if the new requested direction is valid
		if snake.newDirection ~= snake.direction.inverse then
			snake.direction = snake.newDirection
		end

		-- update position
		local oldSnakePosition = createPosition(snake.position)

		snake.position = createPosition{
			x = snake.position.preciseX + snake.direction.x * snake.speed * elapsedSeconds,
			y = snake.position.preciseY + snake.direction.y * snake.speed * elapsedSeconds
		}

		-- update position of body parts
		local bodyParts = snake.bodyParts

		-- iterate over all body parts in reverse
		-- TODO: once everything seems to run somehow stable, try removing the following 'if'
		if #bodyParts > 1 then
			for i=#bodyParts, 2, -1 do
				bodyParts[i].position = createPosition(bodyParts[i-1].position)
			end
		end
		bodyParts[1].position = createPosition(oldSnakePosition)

		-- set position of body parts in grid
		for id,bodyPart in ipairs(bodyParts) do
			local y = bodyPart.position.y
			local x = bodyPart.position.x
			local symbol = bodyPart.symbol
			level.grid[y][x] = symbol
		end

		-- check if head WOULD BE in a valid position or if there is an apple
		local symbolAtNewPosition = level.grid[snake.position.y][snake.position.x]
		if symbolAtNewPosition == apple.symbol then
			apple.doReposition = true
			snake.appendBodyPart()
			score = score + 1
		elseif symbolAtNewPosition ~= level.emptySymbol then
			currentGameState = gamestates.gameover
			return
		end
		level.grid[snake.position.y][snake.position.x] = snake.headSymbol

	end,
	appendBodyPart = function(input)
		local bodyParts = snake.bodyParts
		local lastBodyPart = bodyParts[#bodyParts]
		local newBody = input or {
			position = createPosition(lastBodyPart.position),
			symbol = snake.bodySymbol
		}
		table.insert(bodyParts, newBody)
	end
}

apple = {
	symbol = "@",
	score = 42,
	position = createPosition{ x = -1, y = -1 },

	-- methods
	initialize = function()
		apple.reposition()
	end,
	update = function(elapsedSeconds)
		--set position of apple in level grid
		level.grid[apple.position.y][apple.position.x] = apple.symbol
	end,
	reposition = function()
		apple.doReposition = false
		function positionIsValid()
			return apple.position.x > level.innerBounds.left
			and apple.position.x < level.innerBounds.right
			and apple.position.y > level.innerBounds.upper
			and apple.position.y < level.innerBounds.lower
			and level.grid[apple.position.y][apple.position.x] == level.emptySymbol
		end

		-- get random but valid position
		while not positionIsValid() do
			apple.position = createPosition{
				x = math.random(level.innerBounds.left, level.innerBounds.right) * level.tileDimensions.width,
				y = math.random(level.innerBounds.upper, level.innerBounds.lower) * level.tileDimensions.height,
			}
		end
	end
}

function handleInput()
	local newDirection = nil
	if wasKeyPressed(VK_LEFT) then
		newDirection = directions.west
	end
	if wasKeyPressed(VK_RIGHT) then
		newDirection = directions.east
	end
	if wasKeyPressed(VK_UP) then
		newDirection = directions.north
	end
	if wasKeyPressed(VK_DOWN) then
		newDirection = directions.south
	end

	if newDirection ~= nil then
		snake.newDirection = newDirection
	end
end

-- is called once upon start-up
function initialize()
	setSleepTime(sleeptime)

	-- TODO set random seed here

	level.initialize()
	snake.initialize()
	apple.initialize()
end

-- is called once at the beginning of every frame
-- returns either EXIT_GAME or CONTINUE_GAME to continue or exit the game, respectively.
function update()

	local elapsedSeconds = sleeptime / 1000 -- not ideal but whatever

	if currentGameState == gamestates.running then
		handleInput()
		level.update(elapsedSeconds)
		apple.update(elapsedSeconds)
		snake.update(elapsedSeconds)
		if apple.doReposition then
			apple.reposition()
		end
	end

	if wasKeyPressed(VK_Q) then
		return EXIT_GAME
	end
	return CONTINUE_GAME
end

-- is called once at the end of every frame
function draw()

	print("***** snake.lua *****")
	print("score " .. tostring(score))

	if currentGameState == gamestates.running then
		-- draw the grid
		for rowNumber, row in ipairs(level.grid) do
			for columnNumber, tile in ipairs(row) do
				io.write(tile)
			end
			io.write("\n")
		end
	elseif currentGameState == gamestates.gameover then
		-- draw gameover screen
		print("\n\n#              GAME OVER\n")
		print("\n#              Press Q to quit...")
	end

end
