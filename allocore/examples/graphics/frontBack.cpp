/*
Allocore Example: Front and back views 

Description:
This example demonstrates how to render both front and back views from
a single navigation point.

Author:
Lance Putnam, 8/29/2011
*/

#include "allocore/io/al_App.hpp"

using namespace al;

class MyApp : public App{
public:

	virtual void onDraw(Graphics& g, const Viewpoint& v){
		Mesh& m = g.mesh();
		
		m.reset();
		m.primitive(g.LINES);
		
		for(int j=0; j<4; ++j){
			int Nv = addSphere(m, (j+1)*2, 20, 20);
			for(int i=0; i<Nv; ++i){
				float v = float(i)/Nv;
				m.color(HSV(0.2*v, 1-v*0.5, 1));
			}
		}
		g.draw(m);
	}
};

MyApp app;
Viewpoint vpF, vpB;
ViewpointWindow win(Window::Dim(800,600));

int main(){

	// Configure the front and back viewpoints
	vpF.parentTransform(app.nav());
	vpB.parentTransform(app.nav());

	vpF.stretch(1, 0.5).anchor(0, 0.5);
	vpB.stretch(1, 0.5).anchor(0, 0.0);
	vpB.transform().quat().fromAxisAngle(M_PI, 0,1,0);
	
	win.add(vpF).add(vpB);
	app.add(win);

	app.start();
	return 0;
}

