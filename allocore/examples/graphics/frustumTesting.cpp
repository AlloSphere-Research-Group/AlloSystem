/*
Allocore Example: Frustum Testing

Description:
This demonstrates how to test if points in within the view frustum.

Author:
Lance Putnam, March 2015
*/

#include "allocore/io/al_App.hpp"
#include "allocore/math/al_Frustum.hpp"
using namespace al;

class MyApp : public App{
public:

	MyApp(){
		lens().near(10).far(25);
		initWindow();
	}

	void onDraw(Graphics& g, const Viewpoint& v){

		// First, we get the frustum in world space coordinates
		Frustumd fr;
		v.lens().frustum(fr, v.worldTransform(), v.viewport().aspect());

		//printf("ntl: %g %g %g\n", fr.ntl[0], fr.ntl[1], fr.ntl[2]);
		//printf("ftl: %g %g %g\n", fr.ftl[0], fr.ftl[1], fr.ftl[2]);

		Mesh& m = g.mesh();

		// Draw some lines for testing
		m.reset();
		m.primitive(Graphics::LINES);

		for(int i=0; i<16; ++i){
			float ang = float(i)/16 * M_PI;
			float x = cos(ang)*2;
			float y = sin(ang)*2;
			m.vertex( x, y, -11);
			m.vertex(-x,-y, -12);
		}

		for(int i=0; i<m.vertices().size(); ++i){

			// Test if vertex position is within frustum. Frustum::testPoint
			// returns a non-zero value if its argument is within the frustum.
			int r = fr.testPoint(m.vertices()[i]);

			// Color vertices inside the frustum green and those outside red.
			m.color(r ? RGB(0,1,0) : RGB(1,0,0));
		}

		g.lineWidth(8);
		g.nicest();
		g.draw();


		// Draw rectangle across frustum diagonal
		m.reset();
		m.primitive(Graphics::LINE_LOOP);
		m.color(RGB(0.5));
		m.vertex(fr.nbl);
		m.vertex(fr.fbr);
		m.vertex(fr.ntr);
		m.vertex(fr.ftl);
		g.draw();
	}

};


int main(){
	MyApp().start();
}
