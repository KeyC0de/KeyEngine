--print("Lua script launched from KeyEngine!")


file_result = 1 + 1 + math.cos(3.14 / 2)
file_result = file_result + 100


--struct Player
--player = {}
--player["Title"] = "Mr"
--player["Name"] = "KeyC0de"
--player["Family"] = "badass"
--player["Level"] = 99
-- or:
player = { Title = "Mr", Name = "KeyC0de", Family = "badass", Level = 99 }


function getSum(a, b)
	print("[LUA:] function getSum(" .. a .. ", " .. b .. ") called")	-- #TODO: for some reason this isn't printed
	return a + b
end

function callCFunction(a, b)
	print("[LUA:] function `callCFunction` called")

	luaA = a / 2
	luaB = b * 4
	luaRetValue = callableFromLua_mySum( luaA, luaB )

	return luaRetValue
end
