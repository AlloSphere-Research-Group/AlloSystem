#ifndef INC_AL_CONTROL_NAV_HPP
#define INC_AL_CONTROL_NAV_HPP

#include "allocore/io/al_Window.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

/// Mapping from keyboard and mouse controls to a Nav object
struct NavInputControl : public InputEventHandler {

	NavInputControl(Nav& nav, double vscale = 0.125, double tscale = 2.)
	: mNav(&nav), mVScale(vscale), mTScale(tscale) {}
	virtual ~NavInputControl() {}

	void nav(Nav * v){ mNav=v; }

	virtual bool onKeyDown(const Keyboard& k){	 	

		if(k.ctrl()) return true;

		double vs = nav().velScale();
		double a = mTScale * vs;	// rotational speed: degrees per update
		double v = mVScale * vs;	// speed: units per update

		if(k.alt()) v *= 10;
		if(k.shift()) v *= 0.1;

		switch(k.key()){
			case '`':			nav().halt().home(); return false;
			case 's':			nav().halt(); return false;
			case Key::Up:		nav().spinR( a); return false;
			case Key::Down:		nav().spinR(-a); return false;
			case Key::Right:	nav().spinU(-a); return false;
			case Key::Left:		nav().spinU( a); return false;
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
			nav().turnU(-m.dx() * 0.2);
			nav().turnR(-m.dy() * 0.2);
		}
		else if(m.right()){
			nav().turnF(m.dx() * 0.2);
			//incBehind(m.dy()*0.005);
		}
		return false;
	}

	Nav& nav(){ return *mNav; }
	
	double vscale() { return mVScale; }
	NavInputControl& vscale(double v) { mVScale=v; return *this; }
	
	double tscale() { return mTScale; }
	NavInputControl& tscale(double v) { mTScale=v; return *this; }

protected:
	Nav * mNav;
	double mVScale, mTScale;
};


struct NavInputControlCosm : public NavInputControl {

	NavInputControlCosm(Nav& nav, double vscale = 0.125, double tscale = 2.): NavInputControl(nav, vscale, tscale){}
	virtual ~NavInputControlCosm() {}

	virtual bool onKeyDown(const Keyboard& k){	 	

		double vs = nav().velScale();
		double a = mTScale * vs;	// rotational speed: degrees per update
		double v = mVScale * vs;	// speed: units per update
		
		if(k.ctrl()) v *= 0.1;
		if(k.alt()) v *= 10;
		
		switch(k.key()){
			case '`':			nav().halt().home(); return false;
			case 'w':			nav().spinR(-a); return false;
			case 'x':			nav().spinR( a); return false;
//			case Key::Right:	nav().spinU( a); return false;
//			case Key::Left:		nav().spinU(-a); return false;
			case Key::Right:	nav().spinU( -a); return false;
			case Key::Left:		nav().spinU( a); return false;
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

} // al::

#endif
