
#include "io/al_File.hpp"
#include "system/al_Time.hpp"
#include "system/al_Compiler.hpp"

#include <iostream>

using al::SearchPaths;
using al::File;

using al::Compiler;
using al::JIT;

typedef int (*main_fptr)(int argc, char * const argv[]);

int main (int argc, char * const argv[]) {

	SearchPaths paths(argc, argv);
	
	// path to clang headers:
	std::string clang_headers = paths.appPath() + "/../../externals/allocore/dev/osx/lib/llvm/clang/2.8/include";
	
	int lim = 2;
	Compiler cc[lim];
	JIT * jit[lim];
	for (int i=0; i<lim; i++) jit[i] = NULL;
	
	std::string jitcode_path = paths.appPath() + "/simple_jitcode.cpp";
	File jitcode_file(jitcode_path.data());
	if (jitcode_file.open()) {
		std::string code = jitcode_file.readAll();
		jitcode_file.close();
	
		// run it a few times:
		for (int i=0; i<lim; i++) {
			printf("\n\nstarting pass %d\n", i);
			cc[i].cpp(true);
			cc[i].system_include(clang_headers);
			
			if (cc[i].compile(code)) {
				cc[i].optimize();
				jit[i] = cc[i].jit();
			} else {
				printf("couldn't compile\n");
			}
			
		}
		
		for (int i=0; i<lim; i++) {
			main_fptr f = (main_fptr)(jit[i]->getfunctionptr("main"));
			if (f) {
				f(argc, argv);
			}
		}
		
		for (int i=0; i<lim; i++) {
			if (jit[i]) delete jit[i];
			printf("finished pass %d\n\n\n", i);
		}
	}
	
	printf("test complete\n");
	
	al_sleep(1.);
	
    return 0;
}
