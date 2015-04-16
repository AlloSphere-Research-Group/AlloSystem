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


class MyApp : public App {
public:

	Emitter<8000> em1;
	Mesh mesh;

	MyApp(){
		nav().pos(0,0,16);
		initWindow();
	}

	void onAnimate(double dt){
		em1.update<40>();

		mesh.reset();
		mesh.primitive(Graphics::POINTS);

		for(int i=0; i<em1.size(); ++i){
			Particle& p = em1.particles[i];
			float age = float(p.age) / em1.size();

			mesh.vertex(p.pos);
			mesh.color(Color(HSV(0.6, rnd::uniform(), 1-age), 0.4));
		}
	}

	void onDraw(Graphics& g){
		g.blendAdd();
		g.nicest();
		g.pointSize(6);
		g.draw(mesh);
	}
};

int main(){
	MyApp().start();
}
