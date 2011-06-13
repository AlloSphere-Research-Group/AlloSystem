#ifndef INC_AL_CONTROL_GLV_HPP
#define INC_AL_CONTROL_GLV_HPP

#include "allocore/io/al_Window.hpp"
#include "glv_core.h"

namespace al {

struct GLVControl {

	GLVControl(glv::GLV * v): mGLV(v){}

	void glv(glv::GLV * v){ mGLV=v; }
	glv::GLV& glv(){ return *mGLV; }

protected:
	glv::GLV * mGLV;
};

/// Mapping from keyboard and mouse controls to a GLV object
struct GLVInputControl : public GLVControl, public InputEventHandler {

	GLVInputControl(glv::GLV * v): GLVControl(v){}
	virtual ~GLVInputControl(){}
	
	virtual bool onMouseDown(const Mouse& m){
		glv::space_t xrel=m.x(), yrel=m.y();
		glv().setMouseDown(xrel,yrel, m.button(), 0);
		glv().setMousePos(m.x(), m.y(), xrel, yrel);
		return !glv().propagateEvent();
	}
	
	virtual bool onMouseDrag(const Mouse& m){
		return !motionToGLV(m, glv::Event::MouseDrag);
	}

	virtual bool onMouseMove(const al::Mouse& m){
		return !motionToGLV(m, glv::Event::MouseMove);
	}

	virtual bool onMouseUp(const al::Mouse& m){
		glv::space_t xrel, yrel;
		glv().setMouseUp(xrel,yrel, m.button(), 0);
		glv().setMousePos(m.x(), m.y(), xrel, yrel);
		return !glv().propagateEvent();
	}
	
	virtual bool onKeyDown(const Keyboard& k){
		return !keyToGLV(k, true);
	}
	
	virtual bool onKeyUp(const al::Keyboard& k){
		return !keyToGLV(k, false);
	}

protected:
	bool keyToGLV(const al::Keyboard& k, bool down){
		down ? glv().setKeyDown(k.key()) : glv().setKeyUp(k.key());
		const_cast<glv::Keyboard*>(&glv().keyboard())->alt(k.alt());
		const_cast<glv::Keyboard*>(&glv().keyboard())->caps(k.caps());
		const_cast<glv::Keyboard*>(&glv().keyboard())->ctrl(k.ctrl());
		const_cast<glv::Keyboard*>(&glv().keyboard())->meta(k.meta());
		const_cast<glv::Keyboard*>(&glv().keyboard())->shift(k.shift());
		return glv().propagateEvent();
	}
	
	bool motionToGLV(const al::Mouse& m, glv::Event::t e){
		glv::space_t x = m.x(), y = m.y(), relx = x, rely = y;
		glv().setMouseMotion(relx, rely, e);
		glv().setMousePos((int)x, (int)y, relx, rely);
		return glv().propagateEvent();
	}
};


/// Mapping from window events to a GLV object
struct GLVWindowControl : public GLVControl, public WindowEventHandler {

	GLVWindowControl(glv::GLV * v): GLVControl(v){}
	virtual ~GLVWindowControl(){}

	virtual bool onCreate(){
		glv().broadcastEvent(glv::Event::WindowCreate);
		return true;
	}
	
	virtual bool onDestroy(){
		glv().broadcastEvent(glv::Event::WindowDestroy);
		return true;
	}

	virtual bool onFrame(){
		glv().drawGLV(glv().w, glv().h, window().spf());
		//glv().preamble(glv().w, glv().h);
		//glv().drawWidgets(glv().w, glv().h, window().spf());
		return true;
	}

	virtual bool onResize(int dw, int dh){
		glv().extent(glv().width() + dw, glv().height() + dh);
		//printf("%d %d %f %f\n", dw, dh, glv().width(), glv().height());
		glv().broadcastEvent(glv::Event::WindowResize);
		return true;
	}
	
	//virtual bool onVisibility(bool v){ return true; }
};


/// Pose GLV model
struct PoseModel : public glv::Model{
	PoseModel(Pose& p): pose(p){}

	virtual ~PoseModel(){}

	virtual const glv::Data& getData(glv::Data& d) const {
		d.resize(glv::Data::FLOAT, 7);
		d.assignFromArray(pose.pos().elems, 3);
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
