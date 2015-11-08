/*
Allocore Example: Gravity Well

Description:
The demonstrates how to make many lightweight bodies interact with the
gravitational pull of a single heavy body.

Press the number keys to reset the particles with different initial conditions.

Author:
Lance Putnam, Nov. 2015
*/

#include "allocore/io/al_App.hpp"
using namespace al;

// A particle with acceleration
class Particle{
public:
	Vec3f pos;
	Vec3f vel;
	Vec3f acc;

	void update(double dt){
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
	Light light1, light2;

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
			for(int i=0; i<N; ++i){
				particles[i].pos = rnd::ball<Vec3f>()*0.2 + Vec3f(-0.7,0,0);
				particles[i].vel = Vec3f(0,-0.3,0);
			}
			break;
		case '2': // hourglass
			for(int i=0; i<N; ++i){
				particles[i].pos = rnd::ball<Vec3f>().mag(1);
				particles[i].vel = clone(particles[i].pos).rotate(M_PI/2)*Vec3f(1,1,-1)*0.2;
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

	void onAnimate(double dt){

		// Compute forces
		for(int i=0; i<N; ++i){
			// Newton's law of gravity
			Vec3f r21 = well.pos - particles[i].pos;
			float dist = r21.mag();
			dist = al::max(dist, 0.1f); // prevent high velocities
			Vec3f F = r21/(dist*dist*dist);

			// Newton's second law of motion, F = ma
			particles[i].acc = F * 0.1;
		}

		// Update particles
		for(int i=0; i<N; ++i){
			particles[i].update(dt);
		}
	}

	void onDraw(Graphics& g){
		g.cullFace(true);
		//g.polygonMode(Graphics::LINE);

		light1.dir(1,1,1);
		light1();

		light2.pos(0,0,0);
		light2.attenuation(1,0,4);
		light2.diffuse(body2.colors()[0]);
		light2();

		// Draw the well
		g.draw(body2);

		// Draw the particles
		for(int i=0; i<N; ++i){
			g.pushMatrix();
			g.translate(particles[i].pos);
			g.draw(body1);
			g.popMatrix();
		}
	}

	void onKeyDown(const Keyboard& k){
		reset(k.key());
	}
};


int main(){
	MyApp().start();
}
