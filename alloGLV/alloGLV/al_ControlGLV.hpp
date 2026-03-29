#ifndef INC_AL_CONTROL_GLV_HPP
#define INC_AL_CONTROL_GLV_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	A collection of utility classes for using GLV with AlloCore's window

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/io/al_Window.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "GLV/glv_core.h"
#include "GLV/glv_buttons.h"

namespace al {

/// Base class for mapping window and input events to a GLV controller
class GLVControl {
public:
	///
	GLVControl(glv::GLV& v);

	/// Set GLV controller
	GLVControl& glv(glv::GLV& v);

	/// Get mutable GLV controller
	glv::GLV& glv(){ return *mGLV; }

protected:
	glv::GLV * mGLV;
};



/// Mapping from keyboard and mouse controls to a GLV controller
class GLVInputControl : public GLVControl, public InputEventHandler {
public:
	///
	GLVInputControl(glv::GLV& v);
	~GLVInputControl() override {}

	bool onMouseDown(const Mouse& m) override;
	bool onMouseUp(const al::Mouse& m) override;

	bool onMouseDrag(const Mouse& m) override {
		return !motionToGLV(m, glv::Event::MouseDrag);
	}

	bool onMouseMove(const al::Mouse& m) override {
		return !motionToGLV(m, glv::Event::MouseMove);
	}

	bool onKeyDown(const Keyboard& k) override {
		return !keyToGLV(k, true);
	}

	bool onKeyUp(const al::Keyboard& k) override {
		return !keyToGLV(k, false);
	}

protected:
	bool keyToGLV(const al::Keyboard& k, bool down);
	bool motionToGLV(const al::Mouse& m, glv::Event::t e);
};



/// Mapping from window events to a GLV controller
class GLVWindowControl : public GLVControl, public WindowEventHandler {
public:
	///
	GLVWindowControl(glv::GLV& v);
	~GLVWindowControl() override {}

	bool onCreate() override;
	bool onDestroy() override;
	bool onResize(int w, int h) override;

	bool onFrame() override;
};



/// A glv::GLV subclass that can be easily bound to an al::Window
class GLVBinding : public glv::GLV{
public:

	GLVBinding();

	/// Bind GLV GUI to window

	/// By default, the GLV input event handler is attached to the front of the
	/// window handler list and the GLV window event handler to the end of the
	/// window handler list. This means that the GUI will receive input events
	/// first and be drawn last.
	void bindTo(Window& win);

private:
	GLVWindowControl mWindowCtrl;
	GLVInputControl mInputCtrl;
};



/// A GLV that can be detached into its own window from a parent window
class GLVDetachable : public glv::GLV {
public:

	///
	GLVDetachable();

	/// @param[in] parent	parent window
	GLVDetachable(Window& parent);

	/// Get button for detaching/attaching GUI
	glv::Button& detachedButton(){ return mDetachedButton; }

	/// Get parent window
	Window& parentWindow(){ return *mParentWindow; }

	/// Set parent window
	GLVDetachable& parentWindow(Window& v);

	/// Get detached window
	Window& detachedWindow(){ return mDetachedWindow(); }
	const Window& detachedWindow() const { return mDetachedWindow(); }

	/// Get whether GUI is detached from parent window
	bool detached() const { return detachedWindow().created(); }

	/// Get whether GUI is detached from parent window
	GLVDetachable& detached(bool v);

	/// Toggle whether GUI is detached from parent window
	GLVDetachable& detachedToggle(){ return detached(!detached()); }

private:
	Window * mParentWindow;
	glv::Lazy<Window> mDetachedWindow;
	GLVInputControl mInputControl;
	GLVWindowControl mWindowControl;
	glv::Button mDetachedButton;

	void addGUI(Window& w);
	void remGUI(Window& w);
	void init();
};



/// Pose GLV model
struct PoseModel : public glv::Model{

	///
	PoseModel(Pose& p);

	~PoseModel() override {}

	const glv::Data& getData(glv::Data& d) const override;
	void setData(const glv::Data& d) override;

	Pose& pose;
};


/// Nav GLV model
struct NavModel : public glv::Model{

	///
	NavModel(Nav& n);

	~NavModel() override {}

	const glv::Data& getData(glv::Data& d) const override;
	void setData(const glv::Data& d) override;

	Nav& nav;
};

} // al::

#endif
