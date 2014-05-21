
score = 0
function update(elapsedTime)
	DebugRenderer:printText(Vec2(-0.9, 0.8), "score " .. score)
	DebugRenderer:printText(Vec2(-0.9, 0.75), "elapsedTime " .. string.format("%.2f", elapsedTime))
	
	if (not character.alive) then
		character.go:setPosition(Vec3(0, 0, 200))
		character.rb:setLinearVelocity(Vec3(0, 0, 0))
		character.go:setComponentStates(ComponentState.Active)
		character.alive = true
	end
	
	local characterPosition = character.go:getPosition()
	if (not enemy.alive) then
		local enemyPosition = Vec3(0, 0, 55)
		enemy.direction = Vec3(0, 0, 0)
		if (characterPosition.x >= 0) then
			enemyPosition.x = -math.random(250, 500)
			enemy.direction.x = (0.5 + (math.random() * 0.5))
		end
		if (characterPosition.x < 0) then
			enemyPosition.x = math.random(250, 500)
			enemy.direction.x = -(0.5 + (math.random() * 0.5))
		end		
		if (characterPosition.y >= 0) then
			enemyPosition.y = -math.random(250, 500)
			enemy.direction.y = (0.5 + (math.random() * 0.5))
		end
		if (characterPosition.y < 0) then
			enemyPosition.y = math.random(250, 500)
			enemy.direction.y = -(0.5 + (math.random() * 0.5))
		end
		if (math.random() <= 0.5) then
			enemy.direction.x = -enemy.direction.x
		else
			enemy.direction.y = -enemy.direction.y
		end
		enemy.go:setPosition(enemyPosition)
		enemy.rb:setLinearVelocity(Vec3(0, 0, 0))
		enemy.go:setComponentStates(ComponentState.Active)
		enemy.alive = true
		enemy.direction = enemy.direction:normalized()
	end
	
	local lookAt = Vec3(characterPosition.x, characterPosition.y, 50)
	camera.cc:lookAt(lookAt)
	camera.cc:setPosition(lookAt + camera.posOffset)
end

function updateCharacter(guid, elapsedTime)
	DebugRenderer:printText(Vec2(-0.9, 0.70), "updateCharacter")
	local acceleration = 100
	local jumpPower = 120000
	local impulse = Vec3(0, 0, 0)
	if (InputHandler:isPressed(Key.Up)) then
		impulse.y = acceleration
	end
	if (InputHandler:isPressed(Key.Down)) then
		impulse.y = -acceleration
	end
	if (InputHandler:isPressed(Key.Left)) then
		impulse.x = -acceleration
	end
	if (InputHandler:isPressed(Key.Right)) then
		impulse.x = acceleration
	end
	
	if ((InputHandler:wasTriggered(Key.Space) or InputHandler:wasTriggered(Key.J)) and character.grounded) then
		impulse.z = jumpPower
		character.grounded = false
	end
	character.rb:applyLinearImpulse(impulse)
end

function updateEnemy(guid, elapsedTime)
	local enemyPosition = enemy.go:getPosition()
	if (enemyPosition.x > 650 or enemyPosition.x < -650) then
		enemy.direction.x = -enemy.direction.x;
	end
	if (enemyPosition.y > 650 or enemyPosition.y < -650) then
		enemy.direction.y = -enemy.direction.y;
	end
	local velocity = enemy.direction:mulScalar(100)
	enemy.rb:setLinearVelocity(velocity)
--	DebugRenderer:printText(Vec2(-0.9, 0.65), "enemyPosition x " .. enemyPosition.x .. ", y " .. enemyPosition.y .. ", z " .. enemyPosition.z)
end

function collisionCharacter(event)
	if (event:getSource() == 0) then
		if (character.rb.__ptr == event:getBody(0).__ptr) then
			if (ground.rb.__ptr == event:getBody(1).__ptr) then
				character.grounded = true
			elseif (enemy.rb.__ptr == event:getBody(1).__ptr) then
				local characterPosition = character.go:getPosition()
				local enemyPosition = enemy.go:getPosition()
				if (characterPosition.z >= enemyPosition.z + enemy.halfHeight) then
					event:accessVelocities(0)
					character.rb:applyForce(4, Vec3(0, 0, 18000))
					event:updateVelocities(0)					
					enemy.go:setComponentStates(ComponentState.Inactive)
					enemy.alive = false
					score = score + 1
				else
					character.go:setComponentStates(ComponentState.Inactive)
					character.alive = false
					score = 0
				end
			end
		end
	end
end
