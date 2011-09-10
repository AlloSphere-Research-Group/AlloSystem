#include "alloutil/al_World.hpp"

using namespace al;

class MyActor : public Actor{
public:

	MyActor(){}

	virtual void onSound(AudioIOData& io){}
	
	virtual void onAnimate(double dt){}

	virtual void onDraw(Graphics& g, const Viewpoint& v){

	}

};

World w("Simple World");
ViewpointWindow win(0,0, 600,400, w.name());
Viewpoint vp;
MyActor myActor;

int main(){
	w.camera().near(10).far(25);
	w.nav().quat().fromAxisAngle(0, 0,0,1);

	vp.parentTransform(w.nav());
	
	win.add(vp);
	w.add(win, true);
	w.add(myActor);

	w.start();
	return 0;
}
