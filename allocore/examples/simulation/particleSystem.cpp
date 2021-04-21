/*
Allocore Example: Particle System

Description:
This demonstrates how to build a particle system with a simple fountain-like
behavior.

Author(s):
Lance Putnam, 4/25/2011
*/

#include "allocore/io/al_App.hpp"
using namespace al;

struct Particle{

	Vec3f pos, vel, acc;
	int age = 0;

	void update(int ageInc){
		vel += acc;
		pos += vel;
		age += ageInc;
	}
};


template <int N>
struct Emitter {

	Particle particles[N];
	int tap = 0;

	Emitter(){
		for(auto& p : particles) p.age=N;
	}

	template <int M>
	void update(){
		for(auto& p : particles) p.update(M);

		for(int i=0; i<M; ++i){
			auto& p = particles[tap];

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
};


class MyApp : public App {
public:

	Emitter<8000> em1;
	Mesh mesh;

	MyApp(){
		nav().pullBack(16);
		initWindow();
	}

	void onAnimate(double dt) override {
		em1.update<40>();

		mesh.reset();
		mesh.primitive(Graphics::POINTS);

		for(int i=0; i<em1.size(); ++i){
			auto& p = em1.particles[i];
			float age = float(p.age) / em1.size();

			mesh.vertex(p.pos);
			mesh.color(HSV(0.6, rnd::uniform(), (1-age)*0.4));
		}
	}

	void onDraw(Graphics& g) override {
		g.blendAdd();
		g.nicest();
		g.pointSize(6);
		g.draw(mesh);
	}
};

int main(){
	MyApp().start();
}
