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
///
/// @ingroup allocore
class NavInputControl : public InputEventHandler {
public:
	NavInputControl(const NavInputControl& v);

	NavInputControl(Nav& nav, double vscale = 0.125, double tscale = 2.);

	virtual ~NavInputControl(){}

	virtual bool onKeyDown(const Keyboard& k);
	virtual bool onKeyUp(const Keyboard& k);
	virtual bool onMouseDrag(const Mouse& m);

	Nav& nav(){ return *mNav; }
	const Nav& nav() const { return *mNav; }
	NavInputControl& nav(Nav& v){ mNav=&v; return *this; }

	double vscale() const { return mVScale; }
	NavInputControl& vscale(double v) { mVScale=v; return *this; }

	double tscale() const { return mTScale; }
	NavInputControl& tscale(double v) { mTScale=v; return *this; }

	void useMouse(bool use){ mUseMouse = use; }

protected:
	Nav * mNav;
	double mVScale, mTScale;
	bool mUseMouse;
};


class NavInputControlCosm : public NavInputControl {
public:
	NavInputControlCosm(Nav& nav, double vscale = 0.125, double tscale = 2.);

	virtual ~NavInputControlCosm() {}

	virtual bool onKeyDown(const Keyboard& k);
	virtual bool onKeyUp(const Keyboard& k);
	virtual bool onMouseDrag(const Mouse& m);
};

} // al::

#endif
