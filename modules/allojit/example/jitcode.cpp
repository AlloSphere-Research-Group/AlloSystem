/*
	Code to be compiled by JIT engine:
*/

#include "ajit.hpp"
#include "math/al_Functions.hpp"

#include <iostream>

#include "remapped.h"

using namespace al;

static const int NUM_AGENTS = 1000;
static const double dim = 16;


class Agent : public Nav, public JitObject {
public:
	Agent(JIT * jit) : Nav(), JitObject(jit) {
		pos(dim/2, dim/2, dim/2);
		move(0, 0, 0.1);
		spin(rng.uniformS(), rng.uniformS(), rng.uniformS());
	}
	
	void draw(gfx::Graphics &gl) {
		step();
		// wrap pos:
		Vec3d p = pos();
		for (int i=0; i<3; i++) {
			p[i] = wrap(p[i], dim);
		}
		pos(p);
		
		gl.pushMatrix();
		gl.translate(pos());
		Matrix4d m;
		gl.multMatrix(m.fromQuatGL(quat()));
		gl.begin(gfx::TRIANGLES);
			float ds = 0.1;
			gl.color(1,1,0);
			gl.normal(Vec3d(1, 1, 0).normalize());
			gl.vertex(    0,	0, ds*2);
			gl.vertex( ds/2,	0, -ds);
			gl.vertex(	0,		ds/2, -ds);
			
			gl.normal(Vec3d(-1, 1, 0).normalize());
			gl.vertex(    0,	0, ds*2);
			gl.vertex(-ds/2,	0, -ds);
			gl.vertex(    0,	ds/2, -ds);
		gl.end();
		gl.popMatrix();
	}
};

class AgentSystem : public JitObject {
public:
	
	AgentSystem(World * W, JIT * jit) : JitObject(jit), w(W) {
		for (int i=0; i<NUM_AGENTS; i++) {
			agents.push_back(new Agent(jit));
		}
	}
	
	~AgentSystem() {
		w->win.doFrame = NULL; //World::WorldWindow::doFrameNothing;
		printf("delete AS %p\n", w->win.doFrame);
	}
	
	void draw(gfx::Graphics &gl) {
		for (int i=0; i<NUM_AGENTS; i++) {
			agents[i]->draw(gl);
		}
		
		// remove some:
		if (agents.size()) {
			for (int i=0; i<1; i++) {
				delete *agents.begin();
				agents.erase(agents.begin());
			}
			printf("agents %d\n", agents.size());
			
		}
		
		if (agents.size() == 0) {
			//onclose(w, mJIT);
			delete this;
		}
	}
	
	World * w;
	std::vector<Agent *> agents;
};

AgentSystem * AS;

double r = 0;

void doFrame(void * udata) {
	World * W = (World *)udata;
	gfx::Graphics &gl = W->gl;
	
	gfx::GraphicsBackend * g = gl.backend();
	g->enableLighting(true);
	g->enableLight(true, 0);
	g->enableDepthTesting(true);
	
	
	double divisions = 8;
	double div_size = dim/divisions;
	gl.begin(gfx::LINES);
	gl.color(1, 1, 1);
	for (int x=0; x<=dim; x+= div_size) {
		// the front Y bars
		gl.normal(0, 0, -1);
		gl.vertex(x, 0, 0);
		gl.vertex(x, dim, 0);
		// the back Y bars
		gl.normal(0, 0, 1);
		gl.vertex(x, 0, dim);
		gl.vertex(x, dim, dim);
		// the bottom Z bars
		gl.normal(0, -1, 0);
		gl.vertex(x, 0, 0);
		gl.vertex(x, 0, dim);
		// the top Z bars
		gl.normal(0, 1, 0);
		gl.vertex(x, dim, 0);
		gl.vertex(x, dim, dim);
	}
	for (int y=0; y<=dim; y+= div_size) {
		// the front X bars
		gl.normal(0, 0, -1);
		gl.vertex(0, y, 0);
		gl.vertex(dim, y, 0);
		// the back X bars
		gl.normal(0, 0, 1);
		gl.vertex(0, y, dim);
		gl.vertex(dim, y, dim);
		// the left Z bars
		gl.normal(-1, 0, 0);
		gl.vertex(0, y, 0);
		gl.vertex(0, y, dim);
		// the right Z bars
		gl.normal(1, 0, 0);
		gl.vertex(dim, y, 0);
		gl.vertex(dim, y, dim);
	}
	for (int z=0; z<=dim; z+= div_size) {
		// the bottom X bars
		gl.normal(0, -1, 0);
		gl.vertex(0, 0, z);
		gl.vertex(dim, 0, z);
		// the top X bars
		gl.normal(0, 1, 0);
		gl.vertex(0, dim, z);
		gl.vertex(dim, dim, z);
		// the left Y bars
		gl.normal(-1, 0, 0);
		gl.vertex(0, 0, z);
		gl.vertex(0, dim, z);
		// the right Y bars
		gl.normal(1, 0, 0);
		gl.vertex(dim, 0, z);
		gl.vertex(dim, dim, z);
	}
	gl.end();
	r-=0.8;
	
	
	AS->draw(gl);
}


/*
	Callbacks from JIT engine:
*/
extern "C" void onload(World * W, JIT * jit) {
	printf("onload\n");
	std::cout << "Hello, from a jitted World!\n";
	
	W->win.doFrame = &doFrame;
	//W->cam.pos(dim/2, dim/2, dim/2);
	
	AS = new AgentSystem(W, jit);
	
	//jit->dump();
	
	remapped();
}
