/*
Allocore Example: Gravity Well

Description:
The demonstrates how to make many lightweight bodies interact with the
gravitational pull of a single heavy body.

Press the number keys to reset the particles with different initial conditions.

Author:
Lance Putnam, Nov. 2015
*/

#include <cmath> // max
#include "allocore/io/al_App.hpp"
using namespace al;

// A particle with acceleration
class Particle{
public:
	Vec3f pos;
	Vec3f vel;
	Vec3f acc;

	void update(double dt){
		// Semi-implicit Euler method:
		vel += acc * dt;
		pos += vel * dt;
	}
};


class MyApp : public App{
public:

	static const int M = 20;
	static const int N = M*M;
	Particle particles[N];
	Particle well;
	Mesh body1, body2;

	MyApp(){
		reset();
		addIcosahedron(body1, 0.03);
		body1.color(HSV(0.67, 0.2, 0.5));
		body1.generateNormals();
		addTorus(body2, 0.03, 0.1);
		body2.color(HSV(0.2));
		body2.generateNormals();

		nav().pullBack(3.5);
		nav().faceToward(Vec3f(0,0.7,-1));
		initWindow();
	}

	void reset(int preset='1'){
		switch(preset){
		case '1': // dust cloud
			for(auto& p : particles){
				p.pos = rnd::ball<Vec3f>()*0.2 + Vec3f(-0.7,0,0);
				p.vel = Vec3f(0,-0.3,0);
			}
			break;
		case '2': // hourglass
			for(auto& p : particles){
				p.pos = rnd::ball<Vec3f>().mag(1);
				p.vel = Vec3f(p.pos).rotate(M_PI/2)*Vec3f(1,1,-1)*0.2;
			}
			break;
		case '3': // line orbit 1
			for(int i=0; i<N; ++i){
				particles[i].pos = Vec3f(float(i)/N*0.5-1, 0, 0);
				particles[i].vel = Vec3f(0,-0.3,0);
			}
			break;
		case '4': // line orbit 2
			for(int i=0; i<N; ++i){
				float frac = float(i)/N;
				particles[i].pos = Vec3f(-0.8, frac, 0);
				particles[i].vel = Vec3f(-0.1,-0.2,0.2);
			}
			break;
		case '5': // grid formation (side)
			for(int i=0; i<N; ++i){
				particles[i].pos = Vec3f(-1, float(i%M)/(M-1)*2-1, float(i/M)/(M-1)*2-1);
				particles[i].vel = Vec3f(0,0,0);
			}
			break;
		case '6': // grid formation (front)
			for(int i=0; i<N; ++i){
				particles[i].pos = Vec3f(float(i%M)/(M-1)-0.5, float(i/M)/(M-1)-0.5, 1);
				particles[i].vel = Vec3f(0.1,0,0);
			}
			break;
		}
	}

	void onAnimate(double dt) override {

		// Compute forces
		for(auto& p : particles){
			// Newton's law of gravity
			auto r21 = well.pos - p.pos;	// distance vector between well and particle
			auto dist = r21.mag();			// distance between well and particle
			dist = std::max(dist, 0.1f);	// prevent high velocities
			auto F = r21/(dist*dist*dist);	// force vector acting on particle

			// Newton's second law of motion, F = ma -> a = F/m
			p.acc = F * (1./10); // mass of particle is 10
		}

		// Update particles
		for(auto& p : particles) p.update(dt);
	}

	void onDraw(Graphics& g) override {
		g.cullFace(true);
		//g.polygonMode(Graphics::LINE);

		g.light(0).dir(1,1,1);

		g.light(1).pos(0,0,0)
			.attenuation(1,0,4)
			.diffuse(body2.colors()[0]);

		// Draw the well
		g.draw(body2);

		// Draw the particles
		for(auto& p : particles){
			g.matrixScope([&](){
				g.translate(p.pos);
				g.draw(body1);
			});
		}
	}

	void onKeyDown(const Keyboard& k){
		reset(k.key());
	}
};


int main(){
	MyApp().start();
}
