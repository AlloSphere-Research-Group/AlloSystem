#include "allojit/ajit.hpp"

using namespace al;

App * app;

App :: App * get() {
	return app;
}

App :: App(int argc, char * argv[])
:	oscRecv(12001, 0),
	oscSend(12000, "192.168.1.3"),
	jitfile("", this)
	{
	searchpaths.addAppPaths(argc, argv);
	
	for (int i=0; i<argc; i++) {
		printf("argv[%d]: %s\n", i, argv[i]);
	}
	printf("using app path: %s\n", searchpaths.appPath().c_str());
	
	if (argc>1) {
		FilePath source(argv[1]);
		printf("%s, %s\n", source.path().c_str(), source.file().c_str());
		
		searchpaths.addSearchPath(source.path());
		path = source.path();
		
		Compiler& cc = jitfile.compiler();
		cc.system_include(searchpaths.appPath() + "../lib/llvm/clang/2.8/include");
		cc.include(searchpaths.appPath() + "../include");
		cc.include(source.path());
		jitfile.path(source.filepath());
		
	} else {
		printf("No input file specified.\n");
		exit(0);
	}

	//win.add(new NavInputControlCosm(&nav));
#ifdef USE_STEREO
	win.create(Window::Dim(360,480), "ajit", 100, DisplayMode::DefaultBuf|DisplayMode::StereoBuf);
	stereo.mode(gfx::Stereographic::Active);
	stereo.stereo(true);
#else
    win.create(Window::Dim(360,480), "ajit", 100, DisplayMode::DefaultBuf);
#endif
}

// just a hack
//int al_main_platform_enter(double) {}
//void al_main_platform_attach(double) {}

int main(int argc, char * argv[])
{
	app = new App(argc, argv);

    printf("ready\n");
	
	MainLoop::start();
    return 0;
}