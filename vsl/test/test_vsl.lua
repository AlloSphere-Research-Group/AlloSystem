print("hello vsl")

require "jit"
jit.off()
jit.opt.start(2)

resume, status, wrap, yield = coroutine.resume, coroutine.status, coroutine.wrap, coroutine.yield
function coro(f, ...)
	jit.on(f, true)
	local c = coroutine.wrap(f)
	c(...)
	return c
end

samplerate = 44100
blocksize = blocksize or 64 

out = {}
for i = 1, blocksize do
	out[i] = 0
end



function enveloper(dur)
	local rate = 1 / (dur*samplerate)
	local a = 0
	while true do
		
		-- fade in
		for e = 0, 1, rate*math.random(100) do
			yield(e)
		end
	
		-- fade out
		for e = 1, 0, -rate do
			yield(e)
		end
	end
end


function hipass()
	local x0, y0, x1, y1 = 0, 0, 0, 0
	local c = 0.9
	while true do
		x0 = yield(y0)
		y0 = c * (y1 + x0 - x1)
		x1 = x0
		y1 = y0
	end
end

function lopass()
	local x0, y0, x1, y1 = 0, 0, 0, 0
	local c = 0.9
	while true do
		x0, c = yield(y0)
		y0 = y1 + c * (x0 - y1)
		x1 = x0
		y1 = y0
	end
end


function noiser(id)
	
	local p = 0
	local f = 100 * (8 + math.random(20))/math.random(5)
	f = f/samplerate
	
	local env = coro(enveloper, 1+id*0.1)
	
	local lp = coro(lopass)
	
	while true do
		yield()
		local a
		for i = 1, blocksize do
			
			-- oscillator:
			-- sine:
			p = p + f
			local x0 = math.sin(math.pi * 2 * p)
			--noise:
			--x0 = math.random() --* e/lim
			
			-- envelope:
			a = env()^30
			
			local y0 = lp(x0, a)
			
			-- ola:
			out[i] = out[i] + y0 * 0.05 * a
		end
	end
end



function echoer()
	local len = 44100
	local read = len/2
	local write = 0
	local echo = {}
	for i = 1, len+1 do
		echo[i] = 0
	end
	local a = 0.3
	
	local hp = coro(hipass)
	
	while true do
		
		for i = 1, blocksize do
			
			local y0 = hp(echo[read])
			
			echo[write] = out[i] + 0.95 * y0
			
			out[i] = out[i] + y0 * a
			
			read = 1 + (read % len)
			write = 1 + (write % len)
		end
		yield()
		
	end
end

local ugens = {}
for i = 1, 40 do
	local n = coro(noiser, i)
	table.insert(ugens, n)
end
table.insert(ugens, coro(echoer))

function tick()
	
	-- zero output buffer:
	for i = 1, blocksize do
		out[i] = 0
	end
	
	-- resume all ugens:
	for i, f in ipairs(ugens) do
		f()
	end
	
	--collectgarbage()
	
end