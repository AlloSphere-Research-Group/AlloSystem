#ifndef INCLUDE_AL_CONTROL_NAV_HPP
#define INCLUDE_AL_CONTROL_NAV_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without 
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice, 
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright 
		notice, this list of conditions and the following disclaimer in the 
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its 
		contributors may be used to endorse or promote products derived from 
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Key/Mouse events to control 3D navigation

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

#include "allocore/io/al_Window.hpp"
#include "allocore/spatial/al_Pose.hpp"

namespace al {

/// Mapping from keyboard and mouse controls to a Nav object
struct NavInputControl : public InputEventHandler {

	NavInputControl(const NavInputControl& v)
	:	mNav(v.mNav), mVScale(v.vscale()), mTScale(v.tscale())
	{}

	NavInputControl(Nav& nav, double vscale = 0.125, double tscale = 2.)
	:	mNav(&nav), mVScale(vscale), mTScale(tscale)
	{}

	virtual ~NavInputControl(){}

	virtual bool onKeyDown(const Keyboard& k){	 	

		if(k.ctrl()) return true;

		double vs = nav().velScale();
		double a = mTScale * vs * M_DEG2RAD;	// rotational speed: degrees per update
		double v = mVScale * vs;				// speed: units per update

		if(k.alt()) v *= 10;
		if(k.shift()) v *= 0.1;

		switch(k.key()){
			case '`':				nav().halt().home(); return false;
			case 's':				nav().halt(); return false;
			case Keyboard::UP:		nav().spinR( a); return false;
			case Keyboard::DOWN:	nav().spinR(-a); return false;
			case Keyboard::RIGHT:	nav().spinU(-a); return false;
			case Keyboard::LEFT:	nav().spinU( a); return false;
			case 'q':				nav().spinF( a); return false;
			case 'z':				nav().spinF(-a); return false;
			case 'a':				nav().moveR(-v); return false;
			case 'd':				nav().moveR( v); return false;
			case 'e':				nav().moveU( v); return false;
			case 'c':				nav().moveU(-v); return false;
			case 'x':				nav().moveF(-v); return false;
			case 'w':				nav().moveF( v); return false;
			default:;
		}
		return true;
	}
	virtual bool onKeyUp(const Keyboard& k) {
		switch (k.key()) {
			case Keyboard::UP:
			case Keyboard::DOWN:	nav().spinR(0); return false;
			case Keyboard::RIGHT:
			case Keyboard::LEFT:	nav().spinU(0); return false;
			case 'q':
			case 'z':				nav().spinF(0); return false;
			case 'a':
			case 'd':				nav().moveR(0); return false;
			case 'e':
			case 'c':				nav().moveU(0); return false;
			case 'x':
			case 'w':				nav().moveF(0); return false;
			default:;
		}
		return true;
	}

	virtual bool onMouseDrag(const Mouse& m){
		if(m.left()){
			nav().turnU(-m.dx() * 0.2 * M_DEG2RAD);
			nav().turnR(-m.dy() * 0.2 * M_DEG2RAD);
		}
		else if(m.right()){
			nav().turnF( m.dx() * 0.2 * M_DEG2RAD);
			//incBehind(m.dy()*0.005);
		}
		return false;
	}

	Nav& nav(){ return *mNav; }
	const Nav& nav() const { return *mNav; }
	NavInputControl& nav(Nav& v){ mNav=&v; return *this; }
	
	double vscale() const { return mVScale; }
	NavInputControl& vscale(double v) { mVScale=v; return *this; }
	
	double tscale() const { return mTScale; }
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
		double a = mTScale * vs * M_DEG2RAD;	// rotational speed: degrees per update
		double v = mVScale * vs;				// speed: units per update
		
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

	virtual bool onKeyUp(const Keyboard& k) {
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
	
	virtual bool onMouseDrag(const Mouse& m){ return true; }
};

} // al::

#endif
