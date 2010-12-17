#ifndef INC_AJIT_H
#define INC_AJIT_H

#include "allocore/al_Allocore.hpp"
#include "allojit/al_Compiler.hpp"
#include "allojit/al_JitFile.hpp"	

namespace al {
	
class App {
public:

	static App * get();

	Window win;
	GraphicsGL gl;
	Stereographic stereo;
	Camera cam;
	Nav nav;
	rnd::Random<> rng;
	osc::Recv oscRecv;
	osc::Send oscSend;
	std::string path;
	SearchPaths searchpaths;
	JitFile<App> jitfile;

	App(int argc, char * argv[]);
};

} // al::

#endif /* include guard */
