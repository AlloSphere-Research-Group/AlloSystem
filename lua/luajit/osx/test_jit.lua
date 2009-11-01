--for k, v in pairs(jit) do print("jit", k, v) end

function array_test( n )

	local x, y = {}, {}

	for i=1,n do
		x[i] = i
		y[i] = 0
	end

	for k=1,1000 do
		for j=n,1,-1 do
			y[j] = y[j] + x[j]
		end
	end

	return y[1] .. " " .. y[n]
end

print(array_test(100000))