#ifndef INCLUDE_AJIT_H
#define INCLUDE_AJIT_H

#include "allocore/graphics/al_Graphics.hpp"
#include "allocore/graphics/al_GraphicsBackendOpenGL.hpp"
#include "allocore/graphics/al_Stereographic.hpp"
#include "allocore/io/al_Window.hpp"
#include "allocore/math/al_Random.hpp"

#include "al_Compiler.hpp"

#include "al_NavControl.hpp"
#include "al_Zone.hpp"

typedef void (*draw_fptr)(void *);

namespace al {

static rnd::Random<> rng;

class World;

/*
	Container for all AOT installed elements
*/
class World {
public:
	/// get the world singleton
	static World * get();
	
	struct InputControl : public NavInputControl {
		InputControl(World * W) : NavInputControl(&W->nav) {}
		bool onKeyDown(const Keyboard& k);
		bool onKeyUp(const Keyboard& k);		
		bool onMouseDrag(const Mouse& m){ return true; }
	};

	struct WorldWindow : public Window{
		WorldWindow() : doFrame(doFrameNothing) {}
		bool onFrame();
		static void doFrameNothing(void *) {};
		draw_fptr doFrame;
	};
	
	WorldWindow win;
	Graphics gl;
	Stereographic stereo;
	Camera cam;
	Nav nav;
	//Zone rootZone;
	
private:
	World();
};






} // al::


#endif /* include guard */
