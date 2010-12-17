#include "ajit.hpp"

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
	
	if (argc>=1) {
		FilePath source(argv[1]);
		printf("%s, %s\n", source.path().c_str(), source.file().c_str());
	}
	
	for (int i=0; i<argc; i++) {
		printf("%d %s\n", i, argv[i]);
	}

	//win.add(new NavInputControlCosm(&nav));
#ifdef USE_STEREO
	win.create(Window::Dim(360,480), "ajit", 100, DisplayMode::DefaultBuf|DisplayMode::StereoBuf);
	stereo.mode(gfx::Stereographic::Active);
	stereo.stereo(true);
#else
    win.create(Window::Dim(360,480), "ajit", 100, DisplayMode::DefaultBuf);
#endif

	//FilePath jitcode_path = searchpaths.find("cnm.cpp");
//	Compiler& cc = jitfile.compiler();
//	cc.system_include(searchpaths.appPath() + "../allocore/build/lib/llvm/clang/2.8/include");
//	cc.include(searchpaths.appPath() + "../allocore/build/include");
//	cc.include(searchpaths.appPath() + "../cnm");
//	jitfile.path(jitcode_path.filepath());
}


int main(int argc, char * argv[])
{
	app = new App(argc, argv);

    printf("ready\n");
	
	MainLoop::start();
    return 0;
}