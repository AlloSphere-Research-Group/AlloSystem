#ifndef INCLUDE_AJIT_H
#define INCLUDE_AJIT_H

#include "protocol/al_Graphics.hpp"
#include "protocol/al_GraphicsBackendOpenGL.hpp"
#include "graphics/al_Stereographic.hpp"
#include "io/al_WindowGL.hpp"
#include "math/al_Random.hpp"

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
		InputControl(World * W) : NavInputControl(&W->cam) {}
		bool onKeyDown(const Keyboard& k);
		bool onKeyUp(const Keyboard& k);		
		bool onMouseDrag(const Mouse& m){ return true; }
	};

	struct WorldWindow : public WindowGL{
		WorldWindow() : doFrame(doFrameNothing) {}
		void onFrame();
		static void doFrameNothing(void *) {};
		draw_fptr doFrame;
	};
	
	WorldWindow win;
	gfx::Graphics gl;
	gfx::Stereographic stereo;
	Camera cam;
	Zone rootZone;
	
private:
	World();
};






} // al::


#endif /* include guard */
