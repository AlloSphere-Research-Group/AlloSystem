
//#include "allocore/graphics/al_BufferObject.hpp"
#include "allocore/graphics/al_GPUObject.hpp"
#include "allocore/graphics/al_Image.hpp"
#include "allocore/graphics/al_Isosurface.hpp"
#include "allocore/graphics/al_Model.hpp"
#include "allocore/graphics/al_Light.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "allocore/graphics/al_Texture.hpp"

#include "allocore/io/al_AudioIO.hpp"
#include "allocore/io/al_File.hpp"
#include "allocore/io/al_Socket.hpp"

#include "allocore/math/al_Complex.hpp"
#include "allocore/math/al_Constants.hpp"
#include "allocore/math/al_Frustum.hpp"
#include "allocore/math/al_Functions.hpp"
#include "allocore/math/al_Generators.hpp"
#include "allocore/math/al_Interpolation.hpp"
#include "allocore/math/al_Matrix4.hpp"
#include "allocore/math/al_Plane.hpp"
#include "allocore/math/al_Quat.hpp"
#include "allocore/math/al_Vec.hpp"

#include "allocore/protocol/al_OSC.hpp"
#include "allocore/protocol/al_OSCAPR.hpp"
#include "allocore/protocol/al_Serialize.hpp"

#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/sound/al_Reverb.hpp"

#include "allocore/spatial/al_Camera.hpp"
#include "allocore/spatial/al_CoordinateFrame.hpp"

#include "allocore/system/al_Config.h"
#include "allocore/system/al_MainLoop.h"
#include "allocore/system/al_MainLoop.hpp"
#include "allocore/system/al_Printing.hpp"
#include "allocore/system/al_Thread.hpp"
#include "allocore/system/al_Time.h"
#include "allocore/system/al_Time.hpp"

#include "allocore/types/al_Buffer.hpp"
#include "allocore/types/al_Color.hpp"
#include "allocore/types/al_Conversion.hpp"
#include "allocore/types/al_MsgQueue.hpp"
#include "allocore/types/al_MsgTube.hpp"
#include "allocore/types/al_Array.hpp"

#include "ajit.hpp"


#include <iostream>

/*
	For each cpp file, create a filewatcher & jit. 
	
	Use the filewatcher to re-jit and run onload() each time the file is edited?
		+ run onclose() before it is reloaded
	
	Set up a few basic resources (e.g. window draw loop) 
	
	Create extern some handlers to register new objects with the app
		+ unregister old ones
		
	Even, add a handler to add new files to watch/jit.
	
	
	Still need some kind of GC for the JIT; what if we create a process that extends way into the future?
		JIT object definitely needs reference counting.
	
	
*/


extern "C" void callbackToApp(std::string msg) {
	printf("callback: %s", msg.data());
}

using namespace al;

typedef int (*dumb_fptr)();
typedef void (*onevent_fptr)(World * data, JIT * jit);

typedef void (*doFrame_fptr)(Graphics &gl);

std::string clang_headers_path;
std::string allo_headers_path;
std::string ajit_headers_path;
std::string example_headers_path;
std::string app_path;

World * World::get() { 
	static World * W = new World;
	return W; 
}

World :: World() : gl(new GraphicsBackendOpenGL) /*, rootZone(NULL) */ {}

bool World::InputControl :: onKeyDown(const Keyboard& k){	 	
	World * W = World::get();
	
	static double a = 1;		// rotational speed: degrees per update
	static double v = 0.25;		// speed: units per update
	
	switch(k.key()){
		case '`':			nav().halt().home(); return false;
		case 'w':			nav().spinR(-a); return false;
		case 'x':			nav().spinR( a); return false;
		case Key::Right:	nav().spinU( a); return false;
		case Key::Left:		nav().spinU(-a); return false;
		case 'a':			nav().spinF( a); return false;
		case 'd':			nav().spinF(-a); return false;
		case ',':			nav().moveR(-v); return false;
		case '.':			nav().moveR( v); return false;
		case '\'':			nav().moveU( v); return false;
		case '/':			nav().moveU(-v); return false;
		case Key::Up:		nav().moveF( v); return false;
		case Key::Down:		nav().moveF(-v); return false;
		case Key::Tab:		W->stereo.stereo(!W->stereo.stereo()); return false;
		default:;
	}
	return true;
}

bool World::InputControl :: onKeyUp(const Keyboard& k) {
	switch (k.key()) {
		case 'w':
		case 'x':			nav().spinR(0); return false;
		case Key::Right:
		case Key::Left:		nav().spinU(0); return false;
		case 'a':
		case 'd':			nav().spinF(0); return false;
		case ',':
		case '.':			nav().moveR(0); return false;
		case '\'':
		case '/':			nav().moveU(0); return false;
		case Key::Up:
		case Key::Down:		nav().moveF(0); return false;
		default:;
	}
	return true;
}

bool World::WorldWindow :: onFrame() {
	World * W = World::get();
	W->nav.step();
	
	Viewport vp(width(), height());
	W->stereo.draw(W->gl, W->cam, W->nav, vp, *W);

	//printf("sweep\n");	
	JIT::sweep();
	//printf("end onFrame\n");
	
	return true;
}





class JitFile {
public:

	static JitFile * create(std::string path) {
		if (File::exists(path.data())) {
			return new JitFile(path);
		} else {
			printf("could not create JitFile (no file %s)\n", path.data());
			return NULL;
		}
	}

	JitFile(std::string path) 
	: mFilePath(path.data()), mMtime(0)
	{
		mCC.cpp(true);
		mCC.system_include(clang_headers_path);
		mCC.include(app_path);
		mCC.include(allo_headers_path);
		mCC.include(ajit_headers_path);
		mCC.include(example_headers_path);
		
		mCC.header("remapped.h",
			"#include <stdio.h> \n void remapped() { printf(\"remapped!\\n\"); }");
		
		tick(MainLoop::now());
	}
	
	void tick(al_sec t) {
		al_sec mtime = File::modified(mFilePath.data());
		if (mMtime != mtime) {
			mMtime = mtime;
			
			World * W = World::get();
			
			// use GC system instead:
			//if (mJit) { 
//				mJit->gccheck();
//				onevent_fptr f = (onevent_fptr)(mJit->getfunctionptr("onclose"));
//				if (f) {
//					f(W, mJit);
//				} else {
//					printf("onclose handler not found\n");
//				}
//				mJit->release();
//				printf("RELEASED\n");
//				delete mJit;
//				mJit = 0;
//			}
			
			File file(mFilePath.data(), "r");
			if (file.open()) {
				std::string code = file.readAll();
				file.close();
				if (mCC.compile(code)) {
					mCC.optimize();
					//mCC.dump();
					JIT * jit = mCC.jit();
					if (jit) {
						onevent_fptr f = (onevent_fptr)(jit->getfunctionptr("onload"));
						if (f) {
							f(W, jit);
						} else {
							printf("onload handler not found\n");
						}
					}
				} else {
					printf("couldn't compile\n");
				}
			} else {
				printf("didn't open file\n");
			}
		
		}
		MainLoop::queue().send(t+0.1, this, &JitFile::tick);
	}
	
	std::string mFilePath;
	al_sec mMtime;
	Compiler mCC;
};



int main (int argc, char * const argv[]) {

	World * W = World::get();
	
	JIT::verbose(true);
	
	W->win.add(new StandardWindowKeyControls);
	W->win.add(new World::InputControl(W));
	W->win.create(Window::Dim(720,480), "Window 1", 40);

	SearchPaths paths(argc, argv);
	clang_headers_path = paths.appPath() + "../../dev/osx/lib/llvm/clang/2.8/include";
	allo_headers_path = paths.appPath() + "../../include";
	ajit_headers_path = paths.appPath() + "../../experimental/allojit/src";
	example_headers_path = paths.appPath() + "../../experimental/allojit/example";
	
	paths.addSearchPath(example_headers_path);
	
	FilePath jitcode_path = paths.find("jitcode.cpp");
	printf("loading %s\n", jitcode_path.filepath().data());
	JitFile::create(jitcode_path.filepath());
    
	MainLoop::start();
	return 0;
}
