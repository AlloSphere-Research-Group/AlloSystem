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
			case Key::Up:		nav().spinR(-a); break;
			case Key::Down:		nav().spinR( a); break;
			case Key::Right:	nav().spinU( a); break;
			case Key::Left:		nav().spinU(-a); break;
			case 'q':			nav().spinF( a); break;
			case 'z':			nav().spinF(-a); break;
			case 'a':			nav().moveR(-v); break;
			case 'd':			nav().moveR( v); break;
			case 'e':			nav().moveU( v); break;
			case 'c':			nav().moveU(-v); break;
			case 'x':			nav().moveF(-v); break;
			case 'w':			nav().moveF( v); break;
			default:;
		}
	}
	void onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case Key::Up:
			case Key::Down:		nav().spinR(0); break;
			case Key::Right:
			case Key::Left:		nav().spinU(0); break;
			case 'q':
			case 'z':			nav().spinF(0); break;
			case 'a':
			case 'd':			nav().moveR(0); break;
			case 'e':
			case 'c':			nav().moveU(0); break;
			case 'x':
			case 'w':			nav().moveF(0); break;
			default:;
		}
	}

	void onMouseDrag(const Mouse& m){
		if(m.left()){
			nav().turnU( m.dx() * 0.1);
			nav().turnR( m.dy() * 0.1);
		}
		else if(m.right()){
			nav().turnF(m.dx() * 0.2);
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
			case 'w':			nav().spinR(-a); break;
			case 'x':			nav().spinR( a); break;
			case Key::Right:	nav().spinU( a); break;
			case Key::Left:		nav().spinU(-a); break;
			case 'a':			nav().spinF( a); break;
			case 'd':			nav().spinF(-a); break;
			case ',':			nav().moveR(-v); break;
			case '.':			nav().moveR( v); break;
			case '\'':			nav().moveU( v); break;
			case '/':			nav().moveU(-v); break;
			case Key::Up:		nav().moveF( v); break;
			case Key::Down:		nav().moveF(-v); break;
			default:;
		}
	}
	void onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case 'w':
			case 'x':			nav().spinR(0); break;
			case Key::Right:
			case Key::Left:		nav().spinU(0); break;
			case 'a':
			case 'd':			nav().spinF(0); break;
			case ',':
			case '.':			nav().moveR(0); break;
			case '\'':
			case '/':			nav().moveU(0); break;
			case Key::Up:
			case Key::Down:		nav().moveF(0); break;
			default:;
		}
	}
	void onMouseDown(const Mouse& m){}
	void onMouseUp(const Mouse& m){}
	void onMouseDrag(const Mouse& m){}
	//void onMouseMove(const Mouse& m){}
};



} // al::
