/*
Allocore Example: Hashspace for nearest neighbor lookup.

Description:
Demonstration of neighbor and region lookup in Hashspace.
This example only uses a 2D slice, but it works just as well in 3D.

Author:
Graham Wakefield 2012
Reworked into al::App by Lance Putnam 2021
*/

#include <cmath>
#include <cstdio>
#include "allocore/io/al_App.hpp"
#include "allocore/math/al_Random.hpp"
#include "allocore/spatial/al_HashSpace.hpp"
using namespace al;

class MyApp : public App{
public:

	// the space has 2^6 (64) voxels per side
	// (i.e., for each of the 3 sides) and up to 10000 objects:
	HashSpace space{6, 10000};

	// a query object to be re-used for finding neighbors
	// it will match up to 500 neighbors within a radius.
	HashSpace::Query qmany{500};

	// this is used for finding the nearest neighbor.
	// it will consider 6 matches and return the best.
	HashSpace::Query qnearest{6};

	double radius;

	MyApp() {
		radius = space.maxRadius() * 0.1;

		// initialize all objects with random position
		// just 2D for this demo
		for (unsigned id=0; id<space.numObjects(); id++) {
			space.move(id,
				space.dim()*rnd::uniform()*rnd::uniform(),
				space.dim()*rnd::uniform()*rnd::uniform(),
				0
			);
		}

		initWindow(Window::Dim(800,800));
	}

	void onMouseDrag(const Mouse& m) override {
		double dx = m.dx()/(double)window().width();
		radius += dx * space.maxRadius();
	}

	void onDraw(Graphics& g) override {
		auto t = animateTime();
		double mx = space.dim() * mouseX1();
		double my = space.dim() * mouseY1();

		g.projection(Matrix4d::ortho(0, space.dim(), space.dim(), 0, -1, 1));
		g.modelView(Matrix4d::identity());
		g.depthTesting(0);

		// draw space grid:
		g.color(0.2, 0.2, 0.2, 1);
		g.begin(g.LINES);
		for (unsigned x=0; x<=space.dim(); x++) {
			g.vertex(x, 0, 0);
			g.vertex(x, space.dim(), 0);
		}
		for (unsigned y=0; y<=space.dim(); y++) {
			g.vertex(0, y, 0);
			g.vertex(space.dim(), y, 0);
		}
		g.end();

		g.pointSize(2);
		g.color(0.2, 0.5, 0.5);
		g.begin(g.POINTS);
		for (unsigned id=0; id<space.numObjects(); id++) {
			HashSpace::Object& o =  space.object(id);

			// jiggle the objects around:
			double x = 0.5 + o.pos.x * 0.05;
			double y = 0.5 + o.pos.y * 0.05;
			double speed = 0.2 * sin(t + atan2(y,x));
			space.move(id, o.pos + Vec3d(speed*rnd::uniformS(), speed*rnd::uniformS(), 0.));

			g.vertex(o.pos);
		}
		g.end();

		qmany.clear();
		int results = qmany(space, Vec3d(mx, my, 0), radius);

		// draw active points:
		g.color(1, 0.5, 0.2);
		g.begin(g.POINTS);
		for (int i=0; i<results; i++) {
			g.vertex(qmany[i]->pos);
		}
		g.end();

		// draw a circle around the mouse:
		g.color(0.6, 0.1, 0.1, 1);
		g.begin(g.LINE_LOOP);
		unsigned res = 30;
		for (unsigned i=0; i<=res; i++) {
			double p = M_2PI * i/(double)res;
			g.vertex(mx + sin(p)*radius, my + cos(p)*radius, 0);
		}
		g.end();

		// draw nearest neighbor links:
		double limit = space.maxRadius()*space.maxRadius()/4.;
		g.color(0.2, 0.6, 0.2);
		g.begin(g.LINES);
		for (unsigned id=0; id<space.numObjects(); id++) {
			HashSpace::Object& o =  space.object(id);
			HashSpace::Object * n = qnearest.nearest(space, &o);
			if (n) {
				Vec3d& v = n->pos;
				// don't draw if it is too long:
				if ((o.pos - v).magSqr() < limit) {
					g.vertex(o.pos);
					g.vertex(v);
				}
			}
		}
		g.end();
	}
};

int main(){
	MyApp().start();
}
