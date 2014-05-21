--[[
	GEP_00_Lua.lua
	
	For documentation see: http://www.lua.org/manual/5.2/manual.html#3
]]

print("hello Lua!")
i = 4
print(i)
print(type(i))
i = "whats up??"
print(i)
print(type(i))
i = false
print(i)
print(type(i))

s = "the answer is: "
a = 42
print(s .. a)
print(s .. tostring(a))
s, a= "the answer is: ", 47
print(s .. tostring(a))

t = {9,8}
print(t[1],t[2])

m={}
m["profession"]="agent"
m["name"]="smith"
m["age"]="49"

print(m["profession"], m["name"], m["age"])
print(m.profession, m.name, m.age)

t = {profession = "farmer", name = "john", age = 65}
print (t.profession, t.name, t.age)

vec3 ={x =0,y=5,z=46}

print(vec3.x, vec3.y,vec3.z)

tm = {3,4,5 ,a=6,b=9,c="e"}
print(tm[1],tm[2], tm[3], tm.a, tm.b, tm.c)

t = nil
m = nil
tm = nil


function doWhatIWant()
	print("doWhatIWant()")

end
doWhatIWant()

function ente(i)
	for a=0,i do
	print("ente")
	end
end
ente(15)

function sum(a,b)
	return a+b
end

function sum_avg(a,b)
-- multi assignment, returns 2 values
	return sum(a,b),sum(a,b)/2
end

s,a = sum_avg(3,4)

print(sum_avg(3,4))

a, b = 4,5
function scopeTest()
	a = 6
	local b = 8
end
print(a,b)
scopeTest()
print(a,b)

a,b,c = true, true, false

if a and c then
	print("a and c is true")
elseif b and c then
	print("b and c is true ")
elseif a or c then
	print("a or c is true ")
elseif b or c then 
	print("b or c is true ")
else 
	print("all false")
end

print (not true)
print(true ~= false)

i = 1 
while i<= 5 do	
	print(i)
	i = i+1
end

i =1 
repeat 
	print(i)
	i = i+1
until i == 6

for j = 1, 5 do -- start, stopo

print("something"..j)
end

for k = 0, 20, 5 do --start, stop, step
	print (k)
end

t = {1,2,3, x = 4, y =5, z = 6, "seven"}
for i,v in ipairs(t) do
	print (i,v)
end
for k, v in pairs(t) do
	print(type(k),type(v))
	print (k,v)	
end

enemy = {}
enemy["attack"] = function(what)
	print("attacking " .. what)
end
enemy.attack("player")

enemy["hp"] =1
function enemy:heal()
	self.hp = self.hp + 20
	print("HEAL!!!")
end

function enemy:status()
	print("i have " .. self.hp .. " hp")
end
enemy:status()
enemy:heal() -- member functions has to be called with :
enemy:status()

function average(...)
	local sum, argc = 0,0
	for _, v in ipairs{...} do
		sum = sum + v
		argc = argc + 1
	end
	return sum / argc
end

print(average(1,2,3))
print(average(1,2,3,4,5,6,7,7464536))

--require "math" -- not needed, because in our lua thing this is all included
--require "string"

print(math.sqrt(2))
print(string.sub("find the third word",10,14))

code = [[ 
	print("a literal string")
	print("with to lines")
]]

print (code)
--penis = loadstring(code)
