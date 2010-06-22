#include "system/al_Compiler.hpp"
#include "system/al_Time.hpp"

#include "io/al_AudioIO.hpp"

#include <list>


//struct AudioProcess {
//	void (*onProcess)(AudioProcess * handle, void * userdata);
//	void (*onFree)(AudioProcess * handle, void * userdata);
//	void * userdata;
//	int isDone;
//};
//
//std::list<AudioProcess *> audio_processes;
//
//extern "C" void audioProcessRegister(AudioProcess * proc) {
//	proc->isDone = 0;
//	// TODO: this should be in the audio thread:
//	audio_processes.push_back(proc);
//}
//
//extern "C" void audioProcessUnregister(AudioProcess * proc) {
//	proc->isDone = 1;
//}
//
//void callback(al::AudioIOData &io) {
//	printf(".");
//	for (std::list<AudioProcess *>::iterator it = audio_processes.begin(); it != audio_processes.end(); ) {
//		AudioProcess * handle = *it;
//		if (handle->isDone) {
//			it = audio_processes.erase(it);
//			// TODO: notify main thread to call (handle->onFree)(handle, handle->userdata);
//		} else {
//			(handle->onProcess)(handle, handle->userdata);
//			it++;
//		}
//	}
//}
//
//
//al::AudioIO audio(64, 44100., callback);

typedef int (*vfptr)(int ac, char ** av);

char * src = "\n"
	"#include <math.h>\n"
	"#include <stdio.h>\n"
	"#include <stdarg.h>\n"
	"\n"
	"class Bar { Bar() { printf(\"MADE A Bar!\"); } }; \n"
	"Bar bar; \n"
	"#ifdef __cplusplus \n"
	"extern \"C\" { \n"
	"#endif \n"
	"int x = 3; \n"
	"\n"
	"extern int test2(int x); \n"
	" \n"
	"int z00(int ac, char ** av) { \n"
	"	printf(\"%f %d\\n\", sin(3.14), x); \n"
	"	test2(0); \n"
	"	return 0; \n"
	"} \n"
	" \n"
	"#ifdef __cplusplus \n"
	"} \n"
	"#endif \n"
	"";

int main (int argc, const char * argv[]) {
	char * code;
	if (argc > 1) {
		// read string from path argv[1];
		FILE* file;
		file = fopen(argv[1], "r");
		if (NULL != file) {
			FILE *fp;
			long len;
			char *buf;
			fp=fopen(argv[1],"r");
			fseek(fp,0,SEEK_END); //go to end
			len=ftell(fp); //get position at end (length)
			fseek(fp,0,SEEK_SET); //go to beg.
			buf=(char *)malloc(len+2); //malloc buffer
			fread(buf,len,1,fp); //read into buffer
			fclose(fp);
			buf[len] = '\0';
			//printf("compiling %s\n", buf);
			code = buf;
		}
	} else {
		//printf("compiling %s\n", src);
		code = src;
	}

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
	
	
	bool ok = C.compile(code);
	al::JIT * jit = C.jit();
	f = (vfptr)jit->getfunctionptr("main");
	
	
	printf("compiled? %d (f=%p) \n", ok, f);
	if (f) {
		f(0, 0);
	}
	C.compile(src);
	al::JIT * jit2 = C.jit();
	
	f = (vfptr)jit2->getfunctionptr("z00");
	f(0, 0);
	
//	audio.print();	
//	printf("open %d\n", audio.open());
//	printf("start %d\n", audio.start());
//	al_sleep(2);							
//	audio.printError();
	
	return 0;
}

