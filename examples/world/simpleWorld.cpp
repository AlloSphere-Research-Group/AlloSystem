#include "alloutil/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	virtual void onSound(AudioIOData& io){}
	
	virtual void onAnimate(double dt){}

	virtual void onDraw(Graphics& g, const Viewpoint& v){

	}

};

MyApp app;

int main(){
	app.camera().near(10).far(25);
	app.nav().quat().fromAxisAngle(0, 0,0,1);

	app.initWindow(Window::Dim(600,400));

	app.start();
	return 0;
}
