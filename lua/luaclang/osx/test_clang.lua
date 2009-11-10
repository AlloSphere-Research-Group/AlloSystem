require "clang"
--for k, v in pairs(clang) do print("clang", k, v) end


local src = [==[

double foo(int x) {
	printf("foo %i\n", x);
	return x*2;
}

int main(int ac, char ** av) {
	
	return (int)foo(4);
}

]==]

local mod = clang.compile(src, "src")

print(mod:call("foo", 3))

local src2 = [==[

double zap(double x) {
	printf("zap %f\n", x);
	return foo((int)x);
}	

]==]

local mod2 = clang.compile(src2, "src2")
mod2:linkto(mod)

print(mod2)


print(mod2:call("zap", 3))





