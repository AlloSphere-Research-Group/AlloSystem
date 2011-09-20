#include "allocore/al_Allocore.hpp"
#include "allocore/spatial/al_HashSpace.hpp"
#include "allocore/math/al_Functions.hpp"

#include <stdio.h>

#include <map>

using namespace al;

Window win;
Graphics gl;
rnd::Random<> rng;
Stereographic stereo;

// the space has 2^6 voxels per side (i.e., for each of the 3 sides) and 10000 objects:
HashSpace space(6, 10000);

// a query object to be re-used for finding neighbors:
HashSpace::Query query(space.numObjects());

class World : public WindowEventHandler, public Drawable {
public:
	World() {
		WindowEventHandler& h = *this;
		win.add(h);
		
		// initialize objects with random position
		// just 2D for this demo
		for (unsigned id=0; id<space.numObjects(); id++) {
			space.move(id, 
				space.dim()*rng.uniform(),
				space.dim()*rng.uniform(),
				0
			);
		}
	}

	virtual void onDraw(Graphics& gl) {
		
		// range of query:
		double range = fmod(MainLoop::now() * 2., space.maxRadius());
		double range2 = range/2;
		
		// center of query:
		double p = fmod(MainLoop::now(), space.dim());
		Vec3d center(p, p, 0);
		
		// draw points:
		gl.pointSize(1);
		gl.color(1, 0.3, 0);
		gl.begin(gl.POINTS);
			for (unsigned id=0; id<space.numObjects(); id++) {
				HashSpace::Object& o =  space.object(id);
				
				space.move(id, o.pos + Vec3d(0.1*rng.uniformS(), 0.1*rng.uniformS(), 0.));
				gl.vertex(o.pos);
			}
		gl.end();
		
		// draw region of interest:
		gl.begin(gl.LINE_LOOP);
			gl.color(1, 0.3, 0);
			for (double a=0; a<M_2PI; a+=0.1) {
				gl.vertex(center + Vec3d(range*cos(a), range*sin(a), 0));
			}
		gl.end();
		gl.begin(gl.LINE_LOOP);
			gl.color(1, 0.3, 0);
			for (double a=0; a<M_2PI; a+=0.1) {
				gl.vertex(center + Vec3d(range2*cos(a), range2*sin(a), 0));
			}
		gl.end();
		
		// get neighbors:
		query.clear();
		query(space, center, range, range2);

		// draw neighbors:
		gl.color(0, 0.5, 1.);
		gl.begin(gl.POINTS);
			gl.pointSize(2.);
			for (unsigned i=0; i<query.size(); i++) {
				gl.vertex(query[i]->pos);
			}
		gl.end();
	}
	

	virtual bool onFrame(){ 
	
		Camera cam;
		cam.far(200);
		Pose pose(Vec3d(space.maxRadius(), space.maxRadius(), space.dim()*3));
		Viewport vp(win.width(), win.height());
		
		stereo.draw(gl, cam, pose, vp, *(Drawable *)this);
		
		return true;
	}		

	virtual bool onCreate(){ return true; }
	virtual bool onDestroy(){ return true; }
	virtual bool onResize(int dw, int dh){ return true; }
	virtual bool onVisibility(bool v){ return true; }	
};

int main(){
	win.create(Window::Dim(0,0, 600, 400), "HashSpace collisions");
	World w;
	
	MainLoop::start();
	return 0;
}

