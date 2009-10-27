require "delta"
for k, v in pairs(delta) do 
	--print("delta", k, v) 
	_G[k] = v
end

-- create a sub-scheduler:
local s = scheduler()
print(s)
for k, v in pairs(getmetatable(s)) do 
	--print("scheduler", k, v) 
end

-- spawn a task in the main scheduler:
go(function()
	while true do
		print("lua time is: " .. now())
		-- trigger the sub-scheduler:
		s.advance(2)
		wait(1)
	end
end)

-- spawn a task in the sub-scheduler:
s.go(function()
	while true do
		print("s time is: " .. s.now())
		s.wait(1)
	end
end)


require "audio"
for k, v in pairs(audio) do 
	print("audio", k, v) 
end

