

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
VK_LEFT_STRING = "<"
VK_RIGHT	= 0x27
VK_RIGHT_STRING = ">"
VK_UP		= 0x26
VK_UP_STRING = "^"
VK_DOWN		= 0x28
VK_DOWN_STRING = "v"
VK_SPACE	= 0x20
VK_RETURN	= 0x0D
VK_Q		= 0x51
VK_R 	    = 0x52


BLANK = [[
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
xx                         xx              
xx                         xx                                            
xx                         xx                                            
xx                         xx                                            
xx                         xx                                            
xx                         xx                                            
xx                         xx                                            
xx                         xx                                            
xx                         xx                                            
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
]]


ARROWUP = [[
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
xx          // \\          xx              
xx         /// \\\         xx                                            
xx        ///   \\\        xx                                            
xx       ///     \\\       xx                                            
xx      ///       \\\      xx                                            
xx     ///         \\\     xx                                            
xx    ///           \\\    xx                                            
xx   ///             \\\   xx                                            
xx  ///               \\\  xx                                            
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
]]
ARROWDOWN = [[
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
xx  \\\               ///  xx              
xx   \\\             ///   xx                                            
xx    \\\           ///    xx                                            
xx     \\\         ///     xx                                            
xx      \\\       ///      xx                                            
xx       \\\     ///       xx                                            
xx        \\\   ///        xx                                            
xx         \\\ ///         xx                                            
xx          \\ //          xx                                            
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
]]
ARROWLEFT = [[
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
xx            ///          xx              
xx         ///             xx                                            
xx      ///                xx                                            
xx   ///                   xx                                            
xx |||                     xx                                            
xx   \\\                   xx                                            
xx      \\\                xx                                            
xx         \\\             xx                                            
xx            \\\          xx                                            
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
]]

ARROWRIGHT = [[
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
xx          \\\            xx              
xx             \\\         xx                                            
xx                \\\      xx                                            
xx                   \\\   xx                                            
xx                     ||| xx                                            
xx                   ///   xx                                            
xx                ///      xx                                            
xx             ///         xx                                            
xx          ///            xx                                            
xxxxxxxxxxxxxxxxxxxxxxxxxxxxx                                            
]]

FAILED = [[
XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
XX        LOOOSER!!        XX
XXXXXXXXXXXXXXXXXXXXXXXXXXXXX
]]

SUCCESSED = [[
OOOOOOOOOOOOOOOOOOOOOOOOOOOOO
OOO       WINNER!!!       OOO
OOOOOOOOOOOOOOOOOOOOOOOOOOOOO
]]

STATUSBLANK = [[
-----------------------------
-----------------------------
-----------------------------
]]

OUTPUTFRAME = [[]]
STATUSOUTPUT = [[]]
output = "               "
iterations = 1
digit = 1
valueRow = {}
inputRow = {}
showPattern = true
q = 1

function nextRound()
	iterations = iterations + 1
	q = q + 1
	initRow()
	--showValueRow()
end

function initRow()
	--OUTPUTFRAME = BLANK
	--STATUSOUTPUT = STATUSBLANK
	valueRow = {}
	math.randomseed(os.time())
	
	for i=1,iterations do
		key = math.random(37,40)
		valueRow[i] = key
	end

end

function printRow( row )
	
	for k, v in pairs(row) do
		if v == 37 then
			io.write(VK_LEFT_STRING)
		elseif v == 38 then
			io.write(VK_UP_STRING)
		elseif v == 39 then
			io.write(VK_RIGHT_STRING)
		elseif v == 40 then
			io.write(VK_DOWN_STRING)
		end
			
	end
end
-- is called once upon start-up
function initialize1()
	print("\n this is initialize()")
	-- TODO put your initialization code here
	initRow()
	--showValueRow()
	--setSleepTime(100)
end

function restartGame()
	os.execute("cls")
	output = "               "
	iterations = 1
	q =1
	digit = 1
	valueRow = {}
	inputRow = {}
	initRow()
	showPattern = true
end

function resetInput()
	inputRow = {}
	digit = 1
end

function inputDone(hex)
	showPattern = false
	--io.write("\n"..digit.."                   ")
	--io.write("\n " .. hex .. "\n")
	inputRow[digit]=hex
	
	if digit == iterations then
		if evaluateInput() then
			nextRound()
			resetInput()
		end
	else
		digit = digit + 1
		STATUSOUTPUT = STATUSBLANK
	end
	
	--STATUSOUTPUT = STATUSBLANK
end

function showValueRow( row )
	if showPattern == true then
		for k = 1, iterations do
			if row[k] == 37 then
				io.write(VK_LEFT_STRING)
			elseif row[k] == 38 then
				io.write(VK_UP_STRING)
			elseif row[k] == 39 then
				io.write(VK_RIGHT_STRING)
			elseif row[k] == 40 then
				io.write(VK_DOWN_STRING)
			end
			--os.execute("timeout 1 >nul")	
		end
	else
	end	
--[[

	--for y =1, q do
		if q == iterations then
			showPattern = false
		end
		if(valueRow[q] == 37) then
			OUTPUTFRAME = ARROWLEFT
		elseif(valueRow[q] == 38) then
			OUTPUTFRAME = ARROWUP
		elseif(valueRow[q] == 39) then
			OUTPUTFRAME = ARROWRight
		elseif(valueRow[q] == 40) then
			OUTPUTFRAME = ARROWDOWN	
		end
		q = q + 1
	--os.execute("timeout 1 >nul")
		
	--end
]]
end

function evaluateInput()
	
	b = true
	for i=1,iterations do
		if valueRow[i] == inputRow[i] and b == true then
			b = true
			STATUSOUTPUT = SUCCESSED
		else
			b = false
			STATUSOUTPUT = FAILED
		end
	end
	
	if b == true then
		STATUSOUTPUT = SUCCESSED
		return true
	else
		STATUSOUTPUT = FAILED
		restartGame()
		return false
	end
	
	--return b
end

function printLevel()
	print("\nLevel: ".. iterations)
end

-- is called once at the beginning of every frame
-- returns either EXIT_GAME or CONTINUE_GAME to continue or exit the game, respectively.
function update()
	print("\n this is update()")
	-- TODO put your update code here
	if(isKeyDown( VK_SPACE )) then
		output = "xo"
	elseif(isKeyDown( VK_LEFT)) then
		OUTPUTFRAME = ARROWLEFT
		inputDone(VK_LEFT)
	elseif(isKeyDown( VK_RIGHT)) then
		OUTPUTFRAME = ARROWRIGHT
		inputDone(VK_RIGHT)
	elseif(isKeyDown( VK_UP)) then
		OUTPUTFRAME = ARROWUP
		inputDone(VK_UP)
	elseif(isKeyDown( VK_DOWN)) then
		OUTPUTFRAME = ARROWDOWN
		inputDone(VK_DOWN)
	else	
		--OUTPUTFRAME = BLANK
	end
	
	
	
	
	
	if (wasKeyPressed(VK_Q)) then
		return EXIT_GAME
	end
	if (wasKeyPressed(VK_R)) then
		restartGame()
	end
	
	return CONTINUE_GAME
end

-- is called once at the end of every frame
function draw()
	print("\n this is draw()")
	printRow(valueRow)
	printLevel()
	print("\n***** EmptyCmdGame.lua *****")	-- print automatically adds "\n"
	io.write("\npress q to quit\n")			-- io.write does not add an "\n"
	if showPattern then
		--showValueRow()
		io.write(OUTPUTFRAME)
		--os.execute("timeout 2 >nul")
	else
		io.write(OUTPUTFRAME)
	end
	io.write("\n")
	io.write(STATUSOUTPUT)
	io.write("\n")
	printRow(inputRow)
	-- TODO put your drawing code here
end


