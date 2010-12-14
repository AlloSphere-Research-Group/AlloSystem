#ifndef INC_JITFILE_EXAMPLE_H
#define INC_JITFILE_EXAMPLE_H

#include "allocore/al_Allocore.hpp"
#include "allojit/al_Compiler.hpp"
#include "allojit/al_JitFile.hpp"	

namespace al {

/// Mapping from keyboard and mouse controls to a Nav object
struct NavInputControl : public InputEventHandler {

	NavInputControl(Nav * nav): mNav(nav){}
	virtual ~NavInputControl() {}

	void nav(Nav * v){ mNav=v; }

	virtual bool onKeyDown(const Keyboard& k){	 	
	
		double vs = nav().velScale();
		double a = 2.000 * vs;	// rotational speed: degrees per update
		double v = 0.125 * vs;	// speed: units per update

		switch(k.key()){
			case '`':			nav().halt().home(); return false;
			case 's':			nav().halt(); return false;
			case Key::Up:		nav().spinR(-a); return false;
			case Key::Down:		nav().spinR( a); return false;
			case Key::Right:	nav().spinU( a); return false;
			case Key::Left:		nav().spinU(-a); return false;
			case 'q':			nav().spinF( a); return false;
			case 'z':			nav().spinF(-a); return false;
			case 'a':			nav().moveR(-v); return false;
			case 'd':			nav().moveR( v); return false;
			case 'e':			nav().moveU( v); return false;
			case 'c':			nav().moveU(-v); return false;
			case 'x':			nav().moveF(-v); return false;
			case 'w':			nav().moveF( v); return false;
			default:;
		}
		return true;
	}
	virtual bool onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case Key::Up:
			case Key::Down:		nav().spinR(0); return false;
			case Key::Right:
			case Key::Left:		nav().spinU(0); return false;
			case 'q':
			case 'z':			nav().spinF(0); return false;
			case 'a':
			case 'd':			nav().moveR(0); return false;
			case 'e':
			case 'c':			nav().moveU(0); return false;
			case 'x':
			case 'w':			nav().moveF(0); return false;
			default:;
		}
		return true;
	}

	virtual bool onMouseDrag(const Mouse& m){
		if(m.left()){
			nav().turnU(m.dx() * 0.2);
			nav().turnR(m.dy() * 0.2);
		}
		else if(m.right()){
			nav().turnF(m.dx() * 0.2);
			//incBehind(m.dy()*0.005);
		}
		return false;
	}

	Nav& nav(){ return *mNav; }

protected:
	Nav * mNav;
};


struct NavInputControlCosm : public NavInputControl {

	NavInputControlCosm(Nav * nav): NavInputControl(nav){}
	virtual ~NavInputControlCosm() {}

	virtual bool onKeyDown(const Keyboard& k){	 	

		double vs = nav().velScale();
		double a = 1.00 * vs;	// rotational speed: degrees per update
		double v = 0.25 * vs;	// speed: units per update
		
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
			default:;
		}
		return true;
	}

	virtual bool onKeyUp(const Keyboard& k) {
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
	
	virtual bool onMouseDrag(const Mouse& m){ return true; }
};

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
		
		virtual void onDraw(Graphics& gl) {
			World * W = World::get();
			//W->drawables.onDraw(gl);
		}
		
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
	World() {
		win.add(new NavInputControlCosm(&nav));
	}

};


} // al::

#endif /* include */
