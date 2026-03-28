#ifndef INC_AL_CONTROL_NAV_HPP
#define INC_AL_CONTROL_NAV_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Key/Mouse events to control 3D navigation

	Author(s):
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

	~NavInputControl() override {}

	bool onKeyDown(const Keyboard& k) override;
	bool onKeyUp(const Keyboard& k) override;
	bool onMouseDrag(const Mouse& m) override;

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

	NavInputControl& bubbleMouseEvents(bool v){ mBubbleMouseEvents = v; return *this; }
	NavInputControl& bubbleKeyboardEvents(bool v){ mBubbleKeyboardEvents = v; return *this; }

	NavInputControl& vscale(float v) { mVScale=v; return *this; }
	float vscale() const { return mVScale; }

	NavInputControl& tscale(float v) { mTScale=v; return *this; }
	float tscale() const { return mTScale; }

protected:
	Nav * mNav;
	float mVScale, mTScale, mMouseSens;
	bool mUseMouse = true;
	bool mUseKeyboard = true;
	bool mBubbleMouseEvents = false;
	bool mBubbleKeyboardEvents = false;
};

/// @} // end allocore group

} // al::
#endif
