#include "system/al_Compiler.hpp"

typedef double (*vfptr)(double x);

std::string src = "\n"
	"#include <math.h>\n"
	"#include <stdio.h>\n"
	"#include <stdarg.h>\n"
	"\n"
	"#ifdef __cplusplus \n"
	"extern \"C\" { \n"
	"#endif \n"
	" \n"
	"double zap(double x) { \n"
	"	printf(\"%f -> %f \\n\", x, sin(x)); \n"
	"	return sqrt(x); \n"
	"} \n"
	" \n"
	"#ifdef __cplusplus \n"
	"} \n"
	"#endif \n"
	"";

int main (int argc, const char * argv[]) {


	#define PATHMAX 1024
	char cwd[PATHMAX];
	char allopath[PATHMAX];
	char clangpath[PATHMAX];
	getcwd(cwd, PATHMAX);
	strcpy(allopath, cwd);
	strcat(allopath, "/../../include");
	printf("including path %s\n", allopath);
	strcpy(clangpath, cwd);
	strcat(clangpath, "/../../dev/osx/lib/llvm/clang/1.1/include");
	printf("including path %s\n", clangpath);
	
	

	al::Compiler C;
	C.options.user_includes.push_back(allopath);
	C.options.system_includes.push_back(clangpath);
	vfptr f;
	
	
	bool ok = C.compile(src);
	f = (vfptr)C.getfunctionptr("zap");
	
	
	printf("%d \n", ok);
	for (int i=1; i<10; i++) printf("%d %f\n", i, f(i));
	
	
	return 0;
}

