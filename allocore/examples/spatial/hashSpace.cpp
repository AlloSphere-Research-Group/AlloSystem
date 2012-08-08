#include "allocore/al_Allocore.hpp"
#include "allocore/spatial/al_HashSpace.hpp"
#include "allocore/math/al_Functions.hpp"

/*
Allocore Example: Hashspace for nearest neighbor lookup.

Description:
Demonstration of neighbor and region lookup in Hashspace.
This example only uses a 2D slice, but it works just as well in 3D.

Author:
Graham Wakefield 2012
*/


#include <stdio.h>

#include <map>

using namespace al;

Window win;
Graphics gl;
rnd::Random<> rng;
Stereographic stereo;

// the space has 2^6 (64) voxels per side 
// (i.e., for each of the 3 sides) and up to 10000 objects:
HashSpace space(6, 10000);
unsigned maxradius = space.maxRadius();

// a query object to be re-used for finding neighbors
// it will match up to 500 neighbors within a radius.
HashSpace::Query qmany(500);

// this is used for finding the nearest neighbor. 
// it will consider 6 matches and return the best.
HashSpace::Query qnearest(6);

class World : public WindowEventHandler, public InputEventHandler {
public:
	World() {
		win.append(*(WindowEventHandler *)this);
		win.append(*(InputEventHandler *)this);
		
		radius = maxradius * 0.1;
		
		// initialize all objects with random position
		// just 2D for this demo
		for (unsigned id=0; id<space.numObjects(); id++) {
			space.move(id, 
				space.dim()*rng.uniform()*rng.uniform(),
				space.dim()*rng.uniform()*rng.uniform(),
				0
			);
		}
	}
	
	virtual bool onMouseDrag(const Mouse& m) {
		double dx = m.dx()/(double)win.width();
		radius += dx * maxradius;
		return false;
	}

	virtual bool onFrame(){ 
		al_sec t = al_time();
		double mx, my;
		mx = space.dim() * win.mouse().x() / (double)win.width();
		my = space.dim() * win.mouse().y() / (double)win.height();
		
		Viewport vp(win.width(), win.height());
		
		gl.viewport(vp);
		gl.clear(Graphics::COLOR_BUFFER_BIT | Graphics::DEPTH_BUFFER_BIT);
		gl.projection(Matrix4d::ortho(0, space.dim(), space.dim(), 0, -1, 1));
		gl.modelView(Matrix4d::identity());
		gl.depthTesting(0);
		
		// draw space grid:
		gl.color(0.2, 0.2, 0.2, 1);
		gl.begin(gl.LINES);
		for (unsigned x=0; x<=space.dim(); x++) {
			gl.vertex(x, 0, 0);
			gl.vertex(x, space.dim(), 0);
		}
		for (unsigned y=0; y<=space.dim(); y++) {
			gl.vertex(0, y, 0);
			gl.vertex(space.dim(), y, 0);
		}
		gl.end();
		
		gl.pointSize(2);
		gl.color(0.2, 0.5, 0.5);
		gl.begin(gl.POINTS);
		for (unsigned id=0; id<space.numObjects(); id++) {
			HashSpace::Object& o =  space.object(id);
			
			// jiggle the objects around:
			double x = 0.5 + o.pos.x * 0.05;
			double y = 0.5 + o.pos.y * 0.05;
			double speed = 0.2 * sin(t + atan2(y,x));
			space.move(id, o.pos + Vec3d(speed*rng.uniformS(), speed*rng.uniformS(), 0.));
			
			gl.vertex(o.pos);
		}
		gl.end();
	
		qmany.clear();
		int results = qmany(space, Vec3d(mx, my, 0), radius);
		
		// draw active points:
		gl.color(1, 0.5, 0.2);
		gl.begin(gl.POINTS);
		for (int i=0; i<results; i++) {
			gl.vertex(qmany[i]->pos);
		}
		gl.end();
		
		// draw a circle around the mouse:
		gl.color(0.6, 0.1, 0.1, 1);
		gl.begin(gl.LINE_LOOP);
		unsigned res = 30;
		for (unsigned i=0; i<=res; i++) {
			double p = M_2PI * i/(double)res;
			gl.vertex(mx + sin(p)*radius, my + cos(p)*radius, 0);
		}
		gl.end();
		
		// draw nearest neighbor links:
		double limit = space.maxRadius()*space.maxRadius()/4.;
		gl.color(0.2, 0.6, 0.2);
		gl.begin(gl.LINES);
		for (unsigned id=0; id<space.numObjects(); id++) {
			HashSpace::Object& o =  space.object(id);
			HashSpace::Object * n = qnearest.nearest(space, &o);			
			if (n) {
				Vec3d& v = n->pos;
				// don't draw if it is too long:
				if ((o.pos - v).magSqr() < limit) {
					gl.vertex(o.pos);
					gl.vertex(v);
				}
			}
		}
		gl.end();
		
		return true;
	}		

	virtual bool onCreate(){ return true; }
	virtual bool onDestroy(){ return true; }
	virtual bool onResize(int dw, int dh){ return true; }
	virtual bool onVisibility(bool v){ return true; }	
	
	double radius;
};

int main(){
	win.create(Window::Dim(0,0, 600, 600), "HashSpace collisions");
	win.add(new StandardWindowKeyControls());
	World w;
	
	MainLoop::start();
	return 0;
}

