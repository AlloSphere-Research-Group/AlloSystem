#include "example_using_jitfile.hpp"

using namespace al;

World * World::get() { 
	static World * W = new World;
	return W; 
}

int main (int argc, char * const argv[]) {
	World * W = World::get();
	SearchPaths& searchpaths = W->searchpaths;
	Camera &cam = W->cam;
	Nav &nav = W->nav;
    Window& win = W->win;
	Stereographic& stereo = W->stereo;
	
	searchpaths.addAppPaths(argc, argv);
	std::string sourcepath = searchpaths.appPath() + "../examples/allojit";
	searchpaths.addSearchPath(sourcepath);

	//JIT::verbose(true);

	cam.fovy(40);
	nav.pos().set(0, 0, -2);
	nav.smooth(0.8);

    win.create(Window::Dim(320,240), "Window 1", 100, DisplayMode::DefaultBuf);

	FilePath jitcode_path = searchpaths.find("jitfile.cpp");
	printf("loading %s\n", jitcode_path.filepath().data());
	
	JitFile<World> jitfile(jitcode_path.filepath(), World::get());
	
	Compiler& cc = jitfile.compiler();
	cc.system_include(searchpaths.appPath() + "lib/llvm/clang/2.8/include");
	cc.include(searchpaths.appPath() + "include");
	cc.include(sourcepath);

	MainLoop::start();
    return 0;
}