require "clang"

print(clang)
for k, v in pairs(clang) do 
	print("clang", k, v) 
end


local src = [==[

double foo(int x) {
	return x*2;
}

int main(int ac, char ** av) {
	
	return (int)foo(4);
}

]==]

print(clang.compile(src, "test"))



