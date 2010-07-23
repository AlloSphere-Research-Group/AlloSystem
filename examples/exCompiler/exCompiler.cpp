#include <stdio.h>

#include "system/al_Compiler.hpp"
#include "io/al_File.hpp"

#define MAX_PATH 1024

typedef int (*mainptr)(int ac, char ** av);

int main(int argc, char * argv[]) {
	
	char allo_root_path[MAX_PATH];
	char clang_includes_path[MAX_PATH];
	char allo_includes_path[MAX_PATH];
	char test_input_path[MAX_PATH];
	
	getcwd(allo_root_path, MAX_PATH);
	strcat(allo_root_path, "/../../");
	
	strcpy(allo_includes_path, allo_root_path);
	strcat(allo_includes_path, "include/");
	
	strcpy(clang_includes_path, allo_root_path);
	strcat(clang_includes_path, "dev/osx/lib/llvm/clang/1.1/include");
	
	strcpy(test_input_path, allo_root_path);
	strcat(test_input_path, "examples/exCompiler/input.cpp");
	
	printf("allo_root_path %s\n", allo_root_path);
	printf("allo_includes_path %s\n", allo_includes_path);
	printf("clang_includes_path %s\n", clang_includes_path);
	printf("test_input_path %s\n", test_input_path);
	
	al::File input(test_input_path, "r", true);
	char * code = input.readAll();
	
	printf("to compile:\n%s\n", code);
	
	al::Compiler cc;
	
	/* add clang include path */
	cc.options.system_includes.push_back(clang_includes_path);
	/* add allocore include path */
	cc.options.system_includes.push_back(allo_includes_path);
	
	/* compile code */
	cc.compile(code);
	
	/* show IR */
	cc.dump();
	
	al::JIT * jit = cc.jit();	
	
	mainptr mp = (mainptr)jit->getfunctionptr("main");
	
	mp(NULL, NULL);
	
	return 0;
}
