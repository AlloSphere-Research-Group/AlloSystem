/*
Allocore Example: Flocking

Description:
This is an example implementation of a flocking algorithm. The original flocking
algorithm [1] consists of three main interactions between flockmates ("boids"):

	1) Collision avoidance (of nearby flockmates)
	2) Velocity matching (of nearby flockmates)
	3) Flock centering (of nearby flockmates)

Here, we implement 1) and 2) only. Another change from the reference source is
the use of Gaussian functions rather than inverse-squared functions for
calculating the "nearness" of flockmates. This is done primarily to avoid
infinities, but also to give smoother motions. Lastly, we give each boid a
random walk motion which helps both dissolve and redirect the flocks.

[1] Reynolds, C. W. (1987). Flocks, herds, and schools: A distributed behavioral
    model. Computer Graphics, 21(4):25–34.

Author:
Lance Putnam, Oct. 2014
*/

#include "allocore/io/al_App.hpp"
using namespace al;

// A "boid" (play on bird) is one member of a flock.
class Boid{
public:

	// Each boid has a position and velocity.
	Vec2f pos, vel;

	// Update position based on velocity and delta time
	void update(float dt){
		pos += vel*dt;
	}
};


class MyApp : public App{
public:

	static const int Nb = 32; // Number of boids
	Boid boids[Nb];
	Mesh heads, tails;
	Mesh box;

	MyApp(){
		box.primitive(Graphics::LINE_LOOP);
		box.color(RGB(1));
		box.vertex(-1,-1);
		box.vertex( 1,-1);
		box.vertex( 1, 1);
		box.vertex(-1, 1);
		nav().pullBack(4);
		initWindow();

		resetBoids();
	}

	// Randomize boid positions/velocities uniformly inside unit disc
	void resetBoids(){
		for(auto& b : boids){
			b.pos = rnd::ball<Vec2f>();
			b.vel = rnd::ball<Vec2f>();
		}
	}

	void onAnimate(double dt){

		// Compute boid-boid interactions
		for(int i=0; i<Nb-1; ++i){
			for(int j=i+1; j<Nb; ++j){
				//printf("checking boids %d and %d\n", i,j);

				auto ds = boids[i].pos - boids[j].pos;
				auto dist = ds.mag();

				// Collision avoidance
				float pushRadius = 0.05;
				float pushStrength = 1;
				float push = exp(-al::pow2(dist/pushRadius)) * pushStrength;

				auto pushVector = ds.normalized() * push;
				boids[i].pos += pushVector;
				boids[j].pos -= pushVector;

				// Velocity matching
				float matchRadius = 0.125;
				float nearness = exp(-al::pow2(dist/matchRadius));
				Vec2f veli = boids[i].vel;
				Vec2f velj = boids[j].vel;

				// Take a weighted average of velocities according to nearness
				boids[i].vel = veli*(1 - 0.5*nearness) + velj*(0.5*nearness);
				boids[j].vel = velj*(1 - 0.5*nearness) + veli*(0.5*nearness);

				// TODO: Flock centering
			}
		}

		// Update boid independent behaviors
		for(auto& b : boids){
			// Random "hunting" motion
			float huntUrge = 0.2;
			auto hunt = rnd::ball<Vec2f>();
				// Use cubed distribution to make small jumps more frequent
			hunt *= hunt.magSqr();
			b.vel += hunt*huntUrge;

			// Bound boid into a box
			if(b.pos.x > 1 || b.pos.x < -1){
				b.pos.x = b.pos.x > 0 ? 1 : -1;
				b.vel.x =-b.vel.x;
			}
			if(b.pos.y > 1 || b.pos.y < -1){
				b.pos.y = b.pos.y > 0 ? 1 : -1;
				b.vel.y =-b.vel.y;
			}
		}


		// Generate meshes
		heads.reset();
		heads.primitive(Graphics::POINTS);

		tails.reset();
		tails.primitive(Graphics::LINES);

		for(int i=0; i<Nb; ++i){
			boids[i].update(dt);

			heads.vertex(boids[i].pos);
			heads.color(HSV(float(i)/Nb*0.3+0.3, 0.7));

			tails.vertex(boids[i].pos);
			tails.vertex(boids[i].pos - boids[i].vel.normalized(0.07));

			tails.color(heads.colors()[i]);
			tails.color(RGB(0.5));
		}
	}

	void onDraw(Graphics& g){
		g.nicest();
		g.stroke(8);
		g.draw(heads);

		g.stroke(1);
		g.draw(tails);
		g.draw(box);
	}

	void onKeyDown(const Keyboard& k){
		switch(k.key()){
		case 'r': resetBoids(); break;
		}
	}
};


int main(){
	MyApp().start();
}
