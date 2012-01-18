#ifndef INCLUDE_AL_CONTROL_GLV_HPP
#define INCLUDE_AL_CONTROL_GLV_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	A collection of utility classes for using GLV with AlloCore's window

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include "allocore/io/al_Window.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "GLV/glv_core.h"
#include "GLV/glv_buttons.h"

namespace al {

/// Base class for mapping window and input events to a GLV controller
struct GLVControl {

	///
	GLVControl(glv::GLV& v): mGLV(&v){}

	/// Set GLV controller
	GLVControl& glv(glv::GLV& v){ mGLV=&v; return *this; }
	
	/// Get mutable GLV controller
	glv::GLV& glv(){ return *mGLV; }

protected:
	glv::GLV * mGLV;
};



/// Mapping from keyboard and mouse controls to a GLV controller
struct GLVInputControl : public GLVControl, public InputEventHandler {

	///
	GLVInputControl(glv::GLV& v): GLVControl(v){}
	virtual ~GLVInputControl(){}

	virtual bool onMouseDown(const Mouse& m);
	virtual bool onMouseUp(const al::Mouse& m);

	virtual bool onMouseDrag(const Mouse& m){
		return !motionToGLV(m, glv::Event::MouseDrag);
	}

	virtual bool onMouseMove(const al::Mouse& m){
		return !motionToGLV(m, glv::Event::MouseMove);
	}

	virtual bool onKeyDown(const Keyboard& k){
		return !keyToGLV(k, true);
	}

	virtual bool onKeyUp(const al::Keyboard& k){
		return !keyToGLV(k, false);
	}

protected:
	bool keyToGLV(const al::Keyboard& k, bool down);

	bool motionToGLV(const al::Mouse& m, glv::Event::t e){
		glv::space_t x = m.x(), y = m.y(), relx = x, rely = y;
		glv().setMouseMotion(relx, rely, e);
		glv().setMousePos((int)x, (int)y, relx, rely);
		return glv().propagateEvent();
	}
};



/// Mapping from window events to a GLV controller
struct GLVWindowControl : public GLVControl, public WindowEventHandler {

	///
	GLVWindowControl(glv::GLV& v): GLVControl(v){}
	virtual ~GLVWindowControl(){}

	virtual bool onCreate();
	virtual bool onDestroy();
	virtual bool onResize(int dw, int dh);
	//virtual bool onVisibility(bool v){ return true; }

	virtual bool onFrame(){
		glv().drawGLV(glv().w, glv().h, window().spfActual());
		//glv().preamble(glv().w, glv().h);
		//glv().drawWidgets(glv().w, glv().h, window().spf());
		return true;
	}
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
	Window& detachedWindow(){ return mDetachedWindow; }

	/// Get whether GUI is detached from parent window
	bool detached() const { return mDetachedWindow.created(); }

	/// Get whether GUI is detached from parent window
	GLVDetachable& detached(bool v);
	
	/// Toggle whether GUI is detached from parent window
	GLVDetachable& detachedToggle(){ return detached(!detached()); }

private:
	Window * mParentWindow;
	Window mDetachedWindow;
	GLVInputControl mInputControl;
	GLVWindowControl mWindowControl;
	glv::Button mDetachedButton;

	void addGUI(Window& w);
	void remGUI(Window& w);
	void init();
};



/// Pose GLV model
struct PoseModel : public glv::Model{
	PoseModel(Pose& p): pose(p){}

	virtual ~PoseModel(){}

	virtual const glv::Data& getData(glv::Data& d) const {
		d.resize(glv::Data::FLOAT, 7);
		d.assignFromArray(pose.pos().elems(), 3);
		d.assignFromArray(&pose.quat()[0], 4, 1, 3);
//		double a[4];
//		pose.quat().toAxisAngle(a[0], a[1],a[2],a[3]);
//		d.assignFromArray(a, 4, 1, 3);
		return d;
	}

	virtual void setData(const glv::Data& d){
		pose.pos(d.at<float>(0), d.at<float>(1), d.at<float>(2));
		pose.quat().set(d.at<float>(3), d.at<float>(4), d.at<float>(5), d.at<float>(6));
//		pose.quat().fromAxisAngle(d.at<float>(3), d.at<float>(4), d.at<float>(5), d.at<float>(6));
	}

	Pose& pose;
};


} // al::

#endif
