#include <iostream>

#include "allocore/io/al_File.hpp"
#include "allojit/al_Compiler.hpp"

using namespace al;

SearchPaths searchpaths;

std::string code = ""
	"#include \"allocore/system/al_MainLoop.hpp\" \n"
	"#include <stdio.h> \n"
	"extern \"C\" int test(double x) { \n"
	"	printf(\"test %f\\n\", x); \n"
	"	printf(\"at time: %f\\n\", al::MainLoop::now()); \n"
	"	return x*2; \n"
	"}";
	
typedef int (*Fptr)(double x);

int main (int argc, char * const argv[]) {
    // insert code here...
	searchpaths.addAppPaths(argc, argv);
	
	JIT * jit;

	printf("%s\n", searchpaths.appPath().c_str());
	
	Compiler cc;
	cc.cpp(true);
	cc.system_include(searchpaths.appPath() + "/../lib/llvm/clang/2.8/include");
	cc.include(searchpaths.appPath() + "/../include");
	
	// compile code
	if (cc.compile(code)) {
		// optimize it
		cc.optimize();
		// print out LLVM IR
		cc.dump();
		// load into execution engine (returns a JIT object)
		// the compiler is reset, and can be re-used
		// the JIT can be used to instantiate functions
		// these functions will be deleted when the JIT is deleted
		jit = cc.jit();
		if (jit) {
			jit->retain();
			// grab a function from the JIT object
			// this is where just-in-time machine code generation actually occurs
			Fptr fptr = (Fptr)(jit->getfunctionptr("test"));
			if (fptr) {
				// call the generated function
				fptr(3.141);
			} else {
				printf("failed to retrieve function pointer\n");
			}
			// done with jitted code, delete it:
			jit->release();
			JIT::sweep();
		} else {
			printf("failed to create JIT\n");
		}
	}
	
    std::cout << "Hello, World!\n";
    return 0;
}
