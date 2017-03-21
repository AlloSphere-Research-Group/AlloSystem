/*
Allocore Example: Wave Equation

Description:
This implements a discretized version of the wave equation:

	u(r, t+1) = 2u(r,t) - u(r,t-1) + v^2 [u(r+1, t) - 2u(r,t) + u(r-1,t)]

where

	u	is the wave equation,
	r	is position,
	t	is time, and
	v	is the velocity or CFL (Courant-Friedrichs-Lewy) number.

The simulation adds random Gaussian-shaped drops to simulate water droplets
falling into a pool. A minor artifact is increased rippling along the wavefronts
in the x and y directions.

See also: http://locklessinc.com/articles/wave_eqn/

Author:
Lance Putnam, Oct. 2014
*/

#include "allocore/io/al_App.hpp"
using namespace al;


class MyApp : public App{
public:

	static const int Nx = 256, Ny = Nx;
	float wave[Nx*Ny*2];// Values of wave for current and previous time step
	int zcurr=0;		// The current "plane" coordinate representing time
	float decay=0.96;	// Decay factor of waves, in (0, 1]
	float velocity=0.5;	// Velocity of wave propagation, in (0, 0.5]

	Mesh mesh;
	Light light;
	Material mtrl;


	MyApp(){
		for(auto& v : wave) v = 0;

		// Add a tessellated plane
		addSurface(mesh, Nx,Ny);
		mesh.color(HSV(0.6, 0.2, 0.9));

		nav().pullBack(4);
		initWindow();
	}


	int indexAt(int x, int y, int z){
		//return (z*Nx + y)*Ny + x;
		return (y*Nx + x)*2 + z; // may give slightly faster accessing
	}

	void onAnimate(double dt){

		int zprev = 1-zcurr;

		// Add some random droplets
		for(int k=0; k<3; ++k){
			if(rnd::prob(0.01)){

				// Add a Gaussian-shaped droplet
				int ix = rnd::uniform(Nx-8)+4;
				int iy = rnd::uniform(Ny-8)+4;
				for(int j=-4; j<=4; ++j){
					for(int i=-4; i<=4; ++i){
						float x = float(i)/4;
						float y = float(j)/4;
						float v = 0.5*exp(-(x*x+y*y)/(0.5*0.5));
						wave[indexAt(ix+i, iy+j, zcurr)] += v;
						wave[indexAt(ix+i, iy+j, zprev)] += v;
					}
				}
			}
		}

		// Update wave equation
		for(int j=0; j<Ny; ++j){
		for(int i=0; i<Nx; ++i){

			// Neighbor indices; wrap toroidally
			int im1 = i!=0 ? i-1 : Nx-1;
			int ip1 = i!=Nx-1 ? i+1 : 0;
			int jm1 = j!=0 ? j-1 : Ny-1;
			int jp1 = j!=Nx-1 ? j+1 : 0;

			// Get neighborhood of samples
			auto vp = wave[indexAt(i,j,zprev)];		// previous value
			auto vc = wave[indexAt(i,j,zcurr)];		// current value
			auto vl = wave[indexAt(im1,j,zcurr)];	// neighbor left
			auto vr = wave[indexAt(ip1,j,zcurr)];	// neighbor right
			auto vd = wave[indexAt(i,jm1,zcurr)];	// neighbor up
			auto vu = wave[indexAt(i,jp1,zcurr)];	// neighbor down

			// Compute next value of wave equation at (i,j)
			auto val = 2*vc - vp + velocity*((vl - 2*vc + vr) + (vd - 2*vc + vu));

			// Store in previous value since we don't need it again
			wave[indexAt(i,j,zprev)] = val * decay;

			int idx = j*Nx + i;
			mesh.vertices()[idx].z = val;
		}}

		mesh.generateNormals();

		zcurr = zprev;
	}

	void onDraw(Graphics& g){
		mtrl.specular(RGB(1));
		mtrl.shininess(30);
		mtrl();
		light.dir(1,1,1);
		light();
		g.draw(mesh);
	}
};

int main(){
	MyApp().start();
}
