#ifndef INC_JITFILE_EXAMPLE_H
#define INC_JITFILE_EXAMPLE_H

#include "allocore/al_Allocore.hpp"
#include "allojit/al_Compiler.hpp"
#include "allojit/al_JitFile.hpp"	

namespace al {

/*
	Container for AOT installed elements
*/
class World {
public:
	/// get the world singleton
	static World * get();
	
	class WorldWindow : public Window, public Drawable {
	public:
		virtual ~WorldWindow() {}
		virtual bool onFrame() {
			World * W = World::get();
			W->nav.step();
			
			int w = dimensions().w;
			int h = dimensions().h;
			Pose pose(W->nav);
			W->stereo.draw(W->gl, W->cam, pose, Viewport(0, 0, w, h), *this);
			
			JIT::sweep();
			return true;
		}
		
		virtual void onDraw(Graphics& gl) {}
		
		virtual bool onKeyDown(const Keyboard& key) {
			World * W = World::get();
			switch (key.key()) {
				case Key::Escape:
					fullScreen(!fullScreen());
					break;
				case Key::Tab:
					W->stereo.stereo(!W->stereo.stereo());
					break;
				default:
					break;
			}
			return true;
		}
	};
	
	WorldWindow win;
	GraphicsGL gl;
	Stereographic stereo;
	Camera cam;
	Nav nav;
	SearchPaths searchpaths;
	
private:
	World() {}

};


} // al::

#endif /* include */
