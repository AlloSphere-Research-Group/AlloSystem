#include "allocore/io/al_ControlNav.hpp"

namespace al {

NavInputControl::NavInputControl(const NavInputControl& v)
:	mNav(v.mNav),
	mVScale(v.vscale()), mTScale(v.tscale()), mMouseSens(v.mouseSens()),
	mUseMouse(true)
{}

NavInputControl::NavInputControl(Nav& nav, float vscale, float tscale, float mouseSens)
:	mNav(&nav), mVScale(vscale), mTScale(tscale), mMouseSens(mouseSens), mUseMouse(true)
{}


bool NavInputControl::onKeyDown(const Keyboard& k){

	if(k.ctrl()) return true;

	double a = mTScale * M_DEG2RAD;	// rotational speed: rad/sec
	double v = mVScale;				// speed: world units/sec

	if(k.alt()){
		switch(k.key()){
		case Keyboard::UP:  nav().pullBack(nav().pullBack()*0.8); return false;
		case Keyboard::DOWN:nav().pullBack(nav().pullBack()/0.8); return false;
		}
	}

	if(k.alt()) v *= 10;
	if(k.shift()) v *= 0.1;

	switch(k.key()){
		case '`':				nav().halt().home(); return false;
		case 's':				nav().halt(); return false;
		case Keyboard::UP:		nav().spinR( a); return false;
		case Keyboard::DOWN:	nav().spinR(-a); return false;
		case Keyboard::RIGHT:	nav().spinU(-a); return false;
		case Keyboard::LEFT:	nav().spinU( a); return false;
		case 'q': case 'Q':		nav().spinF( a); return false;
		case 'z': case 'Z':		nav().spinF(-a); return false;
		case 'a': case 'A':		nav().moveR(-v); return false;
		case 'd': case 'D':		nav().moveR( v); return false;
		case 'e': case 'E':		nav().moveU( v); return false;
		case 'c': case 'C':		nav().moveU(-v); return false;
		case 'x': case 'X':		nav().moveF(-v); return false;
		case 'w': case 'W':		nav().moveF( v); return false;
		default:;
	}
	return true;
}

bool NavInputControl::onKeyUp(const Keyboard& k) {
	switch(k.key()){
		case Keyboard::UP:
		case Keyboard::DOWN:	nav().spinR(0); return false;
		case Keyboard::RIGHT:
		case Keyboard::LEFT:	nav().spinU(0); return false;
		case 'q': case 'Q':
		case 'z': case 'Z':		nav().spinF(0); return false;
		case 'a': case 'A':
		case 'd': case 'D':		nav().moveR(0); return false;
		case 'e': case 'E':
		case 'c': case 'C':		nav().moveU(0); return false;
		case 'x': case 'X':
		case 'w': case 'W':		nav().moveF(0); return false;
		default:;
	}
	return true;
}

bool NavInputControl::onMouseDrag(const Mouse& m){
	if(mUseMouse){
		if(m.left()){
			nav().turnU(-m.dx() * (mMouseSens * M_DEG2RAD));
			nav().turnR(-m.dy() * (mMouseSens * M_DEG2RAD));
			return false;
		}
		else if(m.right()){
			nav().turnF( m.dx() * (mMouseSens * M_DEG2RAD));
			nav().pullBack(nav().pullBack() + m.dy()*0.02);
			return false;
		}
	}
	return true;
}



NavInputControlCosm::NavInputControlCosm(Nav& nav, double vscale, double tscale)
:	NavInputControl(nav, vscale, tscale)
{}

bool NavInputControlCosm::onKeyDown(const Keyboard& k){

	double a = mTScale * M_DEG2RAD;	// rotational speed: rad/sec
	double v = mVScale;				// speed: world units/sec

	if(k.ctrl()) v *= 0.1;
	if(k.alt()) v *= 10;

	if(k.ctrl()) a *= 0.1;
	if(k.alt()) a *= 10;

	switch(k.key()){
		case '`':				nav().halt().home(); return false;
		case 'w':				nav().spinR( a); return false;
		case 'x':				nav().spinR(-a); return false;
		case Keyboard::RIGHT:	nav().spinU( -a); return false;
		case Keyboard::LEFT:	nav().spinU( a); return false;
		case 'a':				nav().spinF( a); return false;
		case 'd':				nav().spinF(-a); return false;
		case ',':				nav().moveR(-v); return false;
		case '.':				nav().moveR( v); return false;
		case '\'':				nav().moveU( v); return false;
		case '/':				nav().moveU(-v); return false;
		case Keyboard::UP:		nav().moveF( v); return false;
		case Keyboard::DOWN:	nav().moveF(-v); return false;
		default:;
	}
	return true;
}

bool NavInputControlCosm::onKeyUp(const Keyboard& k) {
	switch (k.key()) {
		case 'w':
		case 'x':				nav().spinR(0); return false;
		case Keyboard::RIGHT:
		case Keyboard::LEFT:	nav().spinU(0); return false;
		case 'a':
		case 'd':				nav().spinF(0); return false;
		case ',':
		case '.':				nav().moveR(0); return false;
		case '\'':
		case '/':				nav().moveU(0); return false;
		case Keyboard::UP:
		case Keyboard::DOWN:	nav().moveF(0); return false;
		default:;
	}
	return true;
}

bool NavInputControlCosm::onMouseDrag(const Mouse& m){
	return true;
}

} // al::
