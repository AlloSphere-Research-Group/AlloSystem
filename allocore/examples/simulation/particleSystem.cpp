/*
Allocore Example: Particle System

Description:
This demonstrates how to build a particle system with a simple fountain-like
behavior.

Author(s):
Lance Putnam, 4/25/2011
*/

#include "allocore/al_Allocore.hpp"
using namespace al;

Graphics gl;

struct Particle{

	Particle(): age(0){}

	void update(int ageInc){
		vel += acc;
		pos += vel;
		age += ageInc;
	}

	Vec3f pos, vel, acc;
	int age;
};


template <int N>
struct Emitter {

	Emitter(): tap(0){
		for(int i=0; i<N; ++i) particles[i].age=N;
	}

	template <int M>
	void update(){
		for(int i=0; i<N; ++i){
			particles[i].update(M);
		}

		for(int i=0; i<M; ++i){
			Particle& p = particles[tap];
			
			// fountain
			if(rnd::prob(0.95)){
				p.vel.set(rnd::uniform(-0.1, -0.05), rnd::uniform(0.12, 0.14), rnd::uniform(0.01));
				p.acc.set(0, -0.002, 0);
				
			// spray
			} else {
				p.vel.set(rnd::uniformS(0.01), rnd::uniformS(0.01), rnd::uniformS(0.01));
				p.acc.set(0, 0, 0);
			}
			p.pos.set(4,-2,0);
			

			p.age = 0;
			++tap; if(tap>=N) tap=0;
		}
	}
	
	int size(){ return N; }

	Particle particles[N];
	int tap;
};


Emitter<8000> em1;


struct MyWindow : Window{

	bool onFrame(){

		em1.update<40>();
		
		Color back(0);

		gl.clearColor(back);
		gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);

		gl.viewport(0,0, width(), height());

		gl.matrixMode(gl.PROJECTION);
		gl.loadMatrix(Matrix4d::perspective(45, aspect(), 0.1, 100));

		gl.matrixMode(gl.MODELVIEW);
		gl.loadMatrix(Matrix4d::lookAt(Vec3d(0,0,10), Vec3d(0,0,0), Vec3d(0,1,0)));

		gl.depthTesting(false);
		gl.blending(true);
		gl.blendModeAdd();
		
		gl.antialiasing(gl.NICEST);
		gl.pointSize(6);

		Mesh& m = gl.mesh();

		m.reset();
		m.primitive(gl.POINTS);
		
		for(int i=0; i<em1.size(); ++i){
			Particle& p = em1.particles[i];
			float age = float(p.age) / em1.size();

			m.vertex(p.pos);
			rnd::Random<> rng;
			m.color(Color(HSV(0.6, rng.uniform(), 1-age), 0.4));
		}
		
		gl.draw(m);
		return true;
	}
};

MyWindow win1;

int main(){
	win1.add(new StandardWindowKeyControls);
	win1.create(Window::Dim(800, 600));

	MainLoop::start();
	return 0;
}
