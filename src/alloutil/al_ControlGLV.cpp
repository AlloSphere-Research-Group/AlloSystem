#include "alloutil/al_ControlGLV.hpp"

namespace al{

bool GLVInputControl::onMouseDown(const Mouse& m){
	glv::space_t xrel=m.x(), yrel=m.y();
	glv().setMouseDown(xrel,yrel, m.button(), 0);
	glv().setMousePos(m.x(), m.y(), xrel, yrel);
	return !glv().propagateEvent();
}

bool GLVInputControl::onMouseUp(const al::Mouse& m){
	glv::space_t xrel, yrel;
	glv().setMouseUp(xrel,yrel, m.button(), 0);
	glv().setMousePos(m.x(), m.y(), xrel, yrel);
	return !glv().propagateEvent();
}

bool GLVInputControl::keyToGLV(const al::Keyboard& k, bool down){
	down ? glv().setKeyDown(k.key()) : glv().setKeyUp(k.key());
	const_cast<glv::Keyboard*>(&glv().keyboard())->alt(k.alt());
	const_cast<glv::Keyboard*>(&glv().keyboard())->caps(k.caps());
	const_cast<glv::Keyboard*>(&glv().keyboard())->ctrl(k.ctrl());
	const_cast<glv::Keyboard*>(&glv().keyboard())->meta(k.meta());
	const_cast<glv::Keyboard*>(&glv().keyboard())->shift(k.shift());
	return glv().propagateEvent();
}



bool GLVWindowControl::onCreate(){
	glv().broadcastEvent(glv::Event::WindowCreate);
	return true;
}

bool GLVWindowControl::onDestroy(){
	glv().broadcastEvent(glv::Event::WindowDestroy);
	return true;
}

bool GLVWindowControl::onResize(int dw, int dh){
	glv().extent(glv().width() + dw, glv().height() + dh);
	//printf("GLVWindowControl onResize: %d %d %f %f\n", dw, dh, glv().width(), glv().height());
	glv().broadcastEvent(glv::Event::WindowResize);
	return true;
}




GLVDetachable::GLVDetachable()
:	glv::GLV(0,0), 
	mParentWindow(NULL), mInputControl(*this), mWindowControl(*this)
{
	init();
}

GLVDetachable::GLVDetachable(Window& parent)
:	glv::GLV(0,0), 
	mInputControl(*this), mWindowControl(*this)
{
	parentWindow(parent);
	init();
}

static void ntDetachedButton(const glv::Notification& n){
	GLVDetachable * R = n.receiver<GLVDetachable>();
	if(n.sender<glv::Button>()->getValue()){
		R->detached(true);
	}
	else{
		R->detached(false);
	}
}

void GLVDetachable::init(){
	mDetachedButton.attach(ntDetachedButton, glv::Update::Value, this);
	mDetachedButton.disable(glv::Momentary);
	mDetachedButton.symbolOn(glv::draw::viewChild);
	mDetachedButton.symbolOff(glv::draw::viewSibling);
	mDetachedButton.disable(glv::DrawBorder);
	stretch(1,1);
	this->disable(glv::DrawBack);
}

void GLVDetachable::addGUI(Window& w){
	w.prepend(mInputControl);
	w.append(mWindowControl);
}

void GLVDetachable::remGUI(Window& w){
	w.remove(mInputControl);
	w.remove(mWindowControl);
}

GLVDetachable& GLVDetachable::detached(bool v){
	if(v && !detached()){			// is not detached
		if(mParentWindow){
			remGUI(parentWindow());
		}
		enable(glv::DrawBack);
		addGUI(detachedWindow());
		glv::Rect ru = unionOfChildren();
		detachedWindow().create(Window::Dim(ru.w, ru.h));
	}
	else if(detached()){			// is currently detached, attach back to parent, if any
		remGUI(detachedWindow());
		detachedWindow().destroy();
		if(mParentWindow){
			disable(glv::DrawBack);
			addGUI(parentWindow());
		}
	}
	return *this;
}

GLVDetachable& GLVDetachable::parentWindow(Window& v){
	if(&v != mParentWindow){
		if(!detached()){
			if(mParentWindow){
				remGUI(parentWindow());
			}
			mParentWindow = &v;
			disable(glv::DrawBack);
			addGUI(parentWindow());
			//printf("%d\n", parentWindow().created());
		}
		else{
			mParentWindow = &v;
		}
	}
	return *this;
}

} // al::
