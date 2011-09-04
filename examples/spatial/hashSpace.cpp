#include "allocore/al_Allocore.hpp"
#include "allocore/spatial/al_HashSpace.hpp"
#include "allocore/math/al_Functions.hpp"

#include <stdio.h>

#include <map>

using namespace al;

Window win;

class World : public WindowEventHandler, public Drawable {
public:
	World() 
	:	space(6, 10000), query(10000)
	{
		WindowEventHandler& h = *this;
		win.add(h);
		
		// initialize each with random position
		for (unsigned i=0; i<space.numObjects(); i++) {
			space.move(i, 
				space.dim()*rng.uniform(),
				space.dim()*rng.uniform(),
				0 //space.dim()*rng.uniform()
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
		gl.begin(gl.POINTS);
		for (unsigned i=0; i<space.numObjects(); i++) {
			HashSpace::Object& o =  space.object(i);
			
			Vec3d newpos = o.pos;
			for (int j=0; j<2; j++) {
				newpos[j] = al::clip(o.pos[j] + 0.1 * rng.uniformS(), (double)space.dim(), 0.);
			}
			space.move(i, newpos);
			
			gl.color(1, 0, 0);
			gl.vertex(o.pos);
		}
		gl.color(1, 1, 0);
		gl.vertex(center);
		gl.end();
		
		// draw region of interest:
		gl.begin(gl.LINE_LOOP);
		gl.color(1, 0, 0);
		for (double a=0; a<M_2PI; a+=0.1) {
			gl.vertex(center + Vec3d(range*cos(a), range*sin(a), 0));
		}
		gl.end();
		gl.begin(gl.LINE_LOOP);
		gl.color(1, 0, 0);
		for (double a=0; a<M_2PI; a+=0.1) {
			gl.vertex(center + Vec3d(range2*cos(a), range2*sin(a), 0));
		}
		gl.end();
		
		// get neighbors:
		query.clear();
		int results = query.query(space, center, range, range2);

		gl.begin(gl.POINTS);
		gl.pointSize(2.);
		for (unsigned i=0; i<results; i++) {
			gl.color(0, 0.5, 0);
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

	
	GraphicsGL gl;
	rnd::Random<> rng;
 	Stereographic stereo;
	
	HashSpace space;
	HashSpace::Query query;
};

int main(){
	win.create(Window::Dim(0,0, 600, 400), "HashSpace collisions");
	World w;
	
	MainLoop::start();
	return 0;
}

