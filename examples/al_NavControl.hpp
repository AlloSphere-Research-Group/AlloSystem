#include "io/al_WindowGL.hpp"
#include "spatial/al_CoordinateFrame.hpp"

namespace al {

/// Mapping from keyboard and mouse controls to a Nav object
struct NavInputControl : InputEventHandler {

	NavInputControl(Nav * nav): mNav(nav){}

	void nav(Nav * v){ mNav=v; }

	void onKeyDown(const Keyboard& k){	 	
	
		static double a = 1;		// rotational speed: degrees per update
		static double v = 0.125;		// speed: units per update
		
		switch(k.key()){
			case '`':			nav().halt().home(); break;
			case 's':			nav().halt(); break;
			case Key::Up:		nav().spinX(-a); break;
			case Key::Down:		nav().spinX( a); break;
			case Key::Right:	nav().spinY( a); break;
			case Key::Left:		nav().spinY(-a); break;
			case 'q':			nav().spinZ( a); break;
			case 'z':			nav().spinZ(-a); break;
			case 'a':			nav().moveX(-v); break;
			case 'd':			nav().moveX( v); break;
			case 'e':			nav().moveY( v); break;
			case 'c':			nav().moveY(-v); break;
			case 'x':			nav().moveZ(-v); break;
			case 'w':			nav().moveZ( v); break;
			default:;
		}
	}
	void onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case Key::Up:
			case Key::Down:		nav().spinX(0); break;
			case Key::Right:
			case Key::Left:		nav().spinY(0); break;
			case 'q':
			case 'z':			nav().spinZ(0); break;
			case 'a':
			case 'd':			nav().moveX(0); break;
			case 'e':
			case 'c':			nav().moveY(0); break;
			case 'x':
			case 'w':			nav().moveZ(0); break;
			default:;
		}
	}

	void onMouseDrag(const Mouse& m){
		if(m.left()){
			nav().turnY( m.dx() * 0.1);
			nav().turnX( m.dy() * 0.1);
		}
		else if(m.right()){
			nav().turnZ(m.dx() * 0.2);
			//incBehind(m.dy()*0.005);
		}
	}

	Nav& nav(){ return *mNav; }

protected:
	Nav * mNav;
};


struct NavInputControlCosm : NavInputControl {

	NavInputControlCosm(Nav * nav): NavInputControl(nav){}

	void onKeyDown(const Keyboard& k){	 	
	
		static double a = 1;		// rotational speed: degrees per update
		static double v = 0.25;		// speed: units per update
		
		switch(k.key()){
			case '`':			nav().halt().home(); break;
			case 'w':			nav().spinX(-a); break;
			case 'x':			nav().spinX( a); break;
			case Key::Right:	nav().spinY( a); break;
			case Key::Left:		nav().spinY(-a); break;
			case 'a':			nav().spinZ( a); break;
			case 'd':			nav().spinZ(-a); break;
			case ',':			nav().moveX(-v); break;
			case '.':			nav().moveX( v); break;
			case '\'':			nav().moveY( v); break;
			case '/':			nav().moveY(-v); break;
			case Key::Up:		nav().moveZ( v); break;
			case Key::Down:		nav().moveZ(-v); break;
			default:;
		}
	}
	void onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case 'w':
			case 'x':			nav().spinX(0); break;
			case Key::Right:
			case Key::Left:		nav().spinY(0); break;
			case 'a':
			case 'd':			nav().spinZ(0); break;
			case ',':
			case '.':			nav().moveX(0); break;
			case '\'':
			case '/':			nav().moveY(0); break;
			case Key::Up:
			case Key::Down:		nav().moveZ(0); break;
			default:;
		}
	}
	void onMouseDown(const Mouse& m){}
	void onMouseUp(const Mouse& m){}
	void onMouseDrag(const Mouse& m){}
	//void onMouseMove(const Mouse& m){}
};



} // al::
