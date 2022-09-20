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

/// @addtogroup allocore
/// @{

/// Mapping from keyboard and mouse controls to a Nav object
class NavInputControl : public InputEventHandler {
public:
	NavInputControl(const NavInputControl& v);

	NavInputControl(Nav& nav, float vscale = 0.125, float tscale = 2., float mouseSens=0.3);

	virtual ~NavInputControl(){}

	virtual bool onKeyDown(const Keyboard& k) override;
	virtual bool onKeyUp(const Keyboard& k) override;
	virtual bool onMouseDrag(const Mouse& m) override;

	Nav& nav(){ return *mNav; }
	const Nav& nav() const { return *mNav; }
	NavInputControl& nav(Nav& v){ mNav=&v; return *this; }

	/// Mouse rotation sensitivity in degrees/pixel
	NavInputControl& mouseSens(float v) { mMouseSens=v; return *this; }
	float mouseSens() const { return mMouseSens; }

	/// Whether to use mouse control
	NavInputControl& useMouse(bool use){ mUseMouse = use; return *this; }

	/// Whether to use mouse control
	NavInputControl& useKeyboard(bool use){ mUseKeyboard = use; return *this; }

	NavInputControl& vscale(float v) { mVScale=v; return *this; }
	float vscale() const { return mVScale; }

	NavInputControl& tscale(float v) { mTScale=v; return *this; }
	float tscale() const { return mTScale; }

protected:
	Nav * mNav;
	float mVScale, mTScale, mMouseSens;
	bool mUseMouse = true;
	bool mUseKeyboard = true;
};


class NavInputControlCosm : public NavInputControl {
public:
	virtual bool onKeyDown(const Keyboard& k) override;
	virtual bool onKeyUp(const Keyboard& k) override;
	virtual bool onMouseDrag(const Mouse& m) override;
};

/// @} // end allocore group

} // al::

#endif
