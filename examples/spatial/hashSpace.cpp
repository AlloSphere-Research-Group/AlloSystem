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
	:	hashspace(5, 100)
	{
		printf("hi view\n");
		
		WindowEventHandler& h = *this;
		
		win.add(h);
		
		// create objects:
		hashspace.rebuild(1000);
		
		// initialize each with random position
		for (unsigned i=0; i<hashspace.numObjects(); i++) {
			hashspace.hash(i, 
				hashspace.dim()*rng.uniform(),
				hashspace.dim()*rng.uniform(),
				0//hashspace.dim()*rng.uniform()
			);
		}
	}

	virtual void onDraw(Graphics& gl) {
		
		gl.pointSize(1);
		gl.begin(gl.POINTS);
		gl.color(1, 1, 1);
		for (unsigned i=0; i<hashspace.numObjects(); i++) {
			HashSpace::Object& o =  hashspace.object(i);
			gl.vertex(o.pos);
			Vec3d newpos = o.pos;
			for (int j=0; j<2; j++) {
				newpos[j] = al::clip(o.pos[j] + 0.01 * rng.uniformS(), (double)hashspace.dim(), 0.);
			}
			hashspace.hash(i, newpos);
		}
		
		double p = fmod(MainLoop::now(), hashspace.dim());
		double range = 5;
		Vec3d center(p, p, 0);
		gl.color(1, 0, 0);
		gl.vertex(center);
		
		gl.end();
		
		gl.begin(gl.LINE_LOOP);
		gl.color(1, 0, 0);
		for (double a=0; a<M_2PI; a+=0.1) {
			gl.vertex(center + Vec3d(range*cos(a), range*sin(a), 0));
		}
		gl.end();
			
		int MAXRESULTS = 100;
		std::vector<HashSpace::Object *> results;
		results.reserve(MAXRESULTS);
		int numresults = hashspace.query(center, 0, range, results, MAXRESULTS);		
		gl.begin(gl.LINES);
		gl.color(0, 1, 0);
		for (int i=0; i<numresults; i++) {
			gl.vertex(center);
			gl.vertex(results[i]->pos);
		}
		gl.end();
	}
	

	virtual bool onFrame(){ 
	
		Camera cam;
		cam.far(200);
		Pose pose(Vec3d(16, 16, 100));
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
	
	HashSpace hashspace;
};

int main(){
	win.create(Window::Dim(0,0, 600, 400), "HashSpace collisions");
	World w;
	
	MainLoop::start();
	return 0;
}

