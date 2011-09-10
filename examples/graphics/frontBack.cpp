/*
Allocore Example: Front and back views 

Description:
This example demonstrates how to render both front and back views from
a single navigation point.

Author:
Lance Putnam, 8/29/2011
*/

#include "alloutil/al_World.hpp"

using namespace al;

class MyActor : public Actor{
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

World w("Front and Back Views");
Viewpoint vpF, vpB;
MyActor myActor;
ViewpointWindow win(0,0, 800,600, w.name());

int main(){

	// Configure the front and back viewpoints
	vpF.parentTransform(w.nav());
	vpB.parentTransform(w.nav());

	vpF.stretch(1, 0.5).anchor(0, 0.5);
	vpB.stretch(1, 0.5).anchor(0, 0.0);
	vpB.transform().quat().fromAxisAngle(M_PI, 0,1,0);
	
	win.add(vpF).add(vpB);
	w.add(win, true);
	
	// Note: this will call our drawing routine for each viewpoint
	w.add(myActor);

	w.start();
	return 0;
}

