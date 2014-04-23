function update(elapsedTime)
	DebugRenderer:printText(Vec2(-0.9,0.7), "elapsed time" .. elapsedTime)
end
acceleration = 1000
boost = false
function updateCharacter(guid,elapsedTime)
	DebugRenderer:printText(Vec2(-0.9,0.8), "charupdate")
	 
	local impulse = Vec3(0,0,0)
	if(InputHandler:isPressed(Key.Shift)) then
		if boost then
			acceleration = acceleration/5
			boost = false
		else
			acceleration = acceleration*5
			boost = true
		end
	end
	if(InputHandler:isPressed(Key.Right)) then
		impulse.x=acceleration
	end
	if(InputHandler:isPressed(Key.Left)) then
		impulse.x=-acceleration
	end
	if(InputHandler:isPressed(Key.Up)) then
		impulse.y=acceleration
	end
	if(InputHandler:isPressed(Key.Down)) then
		impulse.y=-acceleration
	end
	
	if(InputHandler:isPressed(Key.Space) and character.grounded) then
		character.rb:applyForce(1, Vec3(0,0,200))
		character.grounded = false
	end
	
	character.rb:applyLinearImpulse(impulse)
	
end

function collisionCharacter(event)
	if(character.rb.__ptr==event:getBody(event:getSource()).__ptr) then
		print("collision!!")
		character.grounded = true
	end
end