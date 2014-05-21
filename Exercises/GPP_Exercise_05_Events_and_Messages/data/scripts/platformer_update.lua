function update(elapsedTime)
	DebugRenderer:printText(Vec2(-0.9,0.7), "elapsed time" .. elapsedTime)
end

function updateCharacter(guid,elapsedTime)
	DebugRenderer:printText(Vec2(-0.9,0.8), "charupdate")
	local acceleration = 300
	local impulse = Vec3(0,0,0)
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
		character.rb:applyForce(0.5, Vec3(0,0,80000))
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