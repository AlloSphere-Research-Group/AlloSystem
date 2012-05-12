#include "allocore/al_Allocore.hpp"
#include "allocore/spatial/al_HashSpace.hpp"
#include "allocore/math/al_Functions.hpp"
//#include "alloutil/

#include <stdio.h>

#include <map>

using namespace al;

Window win;
Graphics gl;
rnd::Random<> rng;
Stereographic stereo;

// the space has 2^6 (64) voxels per side (i.e., for each of the 3 sides) and 10000 objects:
HashSpace space(6, 20000);
unsigned maxradius = space.maxRadius();

// a query object to be re-used for finding neighbors:
HashSpace::Query qmany(1000);
HashSpace::Query qnearest(10);

class World : public WindowEventHandler, public InputEventHandler, public Drawable {
public:
	World() {
		win.append(*(WindowEventHandler *)this);
		win.append(*(InputEventHandler *)this);
		
		radius = maxradius * 0.1;
		
		// initialize objects with random position
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
		//printf("%f\n", );
		return false;
	}

	virtual void onDraw(Graphics& gl) {
		al_sec t = al_time();
		
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
				double x = 0.5 + o.pos.x * 0.05;
				double y = 0.5 + o.pos.y * 0.05;
				
				double speed = 0.2 * sin(t + atan2(y,x));
				
				space.move(id, o.pos + Vec3d(speed*rng.uniformS(), speed*rng.uniformS(), 0.));
				gl.vertex(o.pos);
			}
		gl.end();
		
		double limit = space.maxRadius()*space.maxRadius()/4.;
		
		// draw nearest neighbor links:
		gl.color(0.2, 1., 0.2);
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
			} else{
				//printf("no nieghbor\n");
			}
		}
		gl.end();
		
//		// draw region of interest:
//		gl.begin(gl.LINE_LOOP);
//			gl.color(1, 0.3, 0);
//			for (double a=0; a<M_2PI; a+=0.1) {
//				gl.vertex(center + Vec3d(range*cos(a), range*sin(a), 0));
//			}
//		gl.end();
//		gl.begin(gl.LINE_LOOP);
//			gl.color(1, 0.3, 0);
//			for (double a=0; a<M_2PI; a+=0.1) {
//				gl.vertex(center + Vec3d(range2*cos(a), range2*sin(a), 0));
//			}
//		gl.end();
//		
//		// get neighbors:
//		qall.clear();
//		qall(space, center, range, range2);
//
//		// draw neighbors:
//		gl.color(0, 0.5, 1.);
//		gl.begin(gl.POINTS);
//			gl.pointSize(2.);
//			for (unsigned i=0; i<qall.size(); i++) {
//				gl.vertex(qall[i]->pos);
//			}
//		gl.end();
	}
	

	virtual bool onFrame(){ 
		al_sec t = al_time();
		double mx, my;
		mx = space.dim() * win.mouse().x() / (double)win.width();
		my = space.dim() * win.mouse().y() / (double)win.height();
		
		Viewport vp(win.width(), win.height());
		
		//Lens lens;
		//lens.far(200);
		//Pose pose(Vec3d(space.maxRadius(), space.maxRadius(), space.dim()*2));
		//stereo.draw(gl, lens, pose, vp, *(Drawable *)this);
		
		
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
		
		
		
		gl.pointSize(3);
		gl.color(0.2, 0.5, 0.5);
		gl.begin(gl.POINTS);
		for (unsigned id=0; id<space.numObjects(); id++) {
			HashSpace::Object& o =  space.object(id);
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
		for (unsigned i=0; i<results; i++) {
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
			} else{
				//printf("no nieghbor\n");
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

