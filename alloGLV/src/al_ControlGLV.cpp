#include "alloGLV/al_ControlGLV.hpp"

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
	//if(R->mouse().isDown()) return;
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
		glv::Rect ru = unionOfChildren();
		enable(glv::DrawBack);
		{
			glv::View * cv = child;
			while(cv){
				cv->posAdd(-ru.l, -ru.t);
				cv = cv->sibling;
			}
		}
		//ru.print();
		//posAdd(-ru.l, -ru.t);
		//detachedWindow().create(Window::Dim(ru.w, ru.h));
		int pl=0, pt=0;
		if(mParentWindow){
			pl = parentWindow().dimensions().l;
			pt = parentWindow().dimensions().t;
			//printf("%d %d\n", pl, pt);
		}
		detachedWindow().create(Window::Dim(pl, pt, ru.w, ru.h));
		addGUI(detachedWindow());
	}
	else if(detached()){ // is currently detached, attach back to parent, if any
		remGUI(detachedWindow());
		detachedWindow().destroy();
		if(mParentWindow){
			disable(glv::DrawBack);
			pos(0,0);
			addGUI(parentWindow());
		}
	}

	// This is a hack to ensure all GLV mouse button states are "up" (false).
	// Because the GLV changes windows between mouse down and mouse up calls,
	// the mouse relative position gets into an inconsistent state on mouse up.
	const int mx = mouse().x();
	const int my = mouse().y();
	for(int i=0; i<GLV_MAX_MOUSE_BUTTONS; ++i){
		glv::space_t x=mx,y=my;
		setMouseUp(x,y, i, 1);
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
