#include "allocore/io/al_ControlNav.hpp"

namespace al {

NavInputControl::NavInputControl(const NavInputControl& v)
:	NavInputControl(*v.mNav, v.vscale(), v.tscale(), v.mouseSens())
{}

NavInputControl::NavInputControl(Nav& nav, float vscale, float tscale, float mouseSens)
:	mNav(&nav), mVScale(vscale), mTScale(tscale), mMouseSens(mouseSens)
{}


bool NavInputControl::onKeyDown(const Keyboard& k){
	if(mUseKeyboard && !k.ctrl()){
		auto bubble = mBubbleKeyboardEvents;
		double a = mTScale * M_DEG2RAD;	// rotational speed: rad/sec
		double v = mVScale;				// speed: world units/sec

		if(k.alt()){
			switch(k.key()){
			case Keyboard::UP:  nav().pullBack(nav().pullBack()*0.8); return bubble;
			case Keyboard::DOWN:nav().pullBack(nav().pullBack()/0.8); return bubble;
			}
		}

		if(k.alt()) v *= 10;
		if(k.shift()) v *= 0.1;

		switch(k.key()){
			case '`':				nav().halt().home(); return bubble;
			case 's':				nav().halt(); return bubble;
			case Keyboard::UP:		nav().spinR( a); return bubble;
			case Keyboard::DOWN:	nav().spinR(-a); return bubble;
			case Keyboard::RIGHT:	nav().spinU(-a); return bubble;
			case Keyboard::LEFT:	nav().spinU( a); return bubble;
			case 'q': case 'Q':		nav().spinF( a); return bubble;
			case 'z': case 'Z':		nav().spinF(-a); return bubble;
			case 'a': case 'A':		nav().moveR(-v); return bubble;
			case 'd': case 'D':		nav().moveR( v); return bubble;
			case 'e': case 'E':		nav().moveU( v); return bubble;
			case 'c': case 'C':		nav().moveU(-v); return bubble;
			case 'x': case 'X':		nav().moveF(-v); return bubble;
			case 'w': case 'W':		nav().moveF( v); return bubble;
			default:;
		}
	}
	return true;
}

bool NavInputControl::onKeyUp(const Keyboard& k){
	if(mUseKeyboard){
		auto bubble = mBubbleKeyboardEvents;
		switch(k.key()){
			case Keyboard::UP:
			case Keyboard::DOWN:	nav().spinR(0); return bubble;
			case Keyboard::RIGHT:
			case Keyboard::LEFT:	nav().spinU(0); return bubble;
			case 'q': case 'Q':
			case 'z': case 'Z':		nav().spinF(0); return bubble;
			case 'a': case 'A':
			case 'd': case 'D':		nav().moveR(0); return bubble;
			case 'e': case 'E':
			case 'c': case 'C':		nav().moveU(0); return bubble;
			case 'x': case 'X':
			case 'w': case 'W':		nav().moveF(0); return bubble;
			default:;
		}
	}
	return true;
}

bool NavInputControl::onMouseDrag(const Mouse& m){
	if(mUseMouse){
		auto bubble = mBubbleMouseEvents;

		if(m.left()){
			nav().turnU(-m.dx() * (mMouseSens * M_DEG2RAD));
			nav().turnR(-m.dy() * (mMouseSens * M_DEG2RAD));
			return bubble;
		}
		else if(m.right()){
			nav().turnF( m.dx() * (mMouseSens * M_DEG2RAD));
			nav().pullBack(nav().pullBack() + m.dy()*0.02);
			return bubble;
		}
	}
	return true;
}

} // al::
