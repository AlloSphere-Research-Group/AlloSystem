#include <cmath>
#include "utAllocore.h"
#include "allocore/math/al_Spherical.hpp"
using namespace al;

int utMathSpherical(){

	double eps = 1e-8;
	constexpr double pi = 3.1415926535897932;
	constexpr double twoPi = 2.*pi;

	auto eq = [](Vec3f a, Vec3f b, float eps=0.000001){
		return ::eq(&a[0], &b[0], a.size(), eps);
	};

	{
		assert(eq(sphereToCart(Vec3f( 1,0,0)), Vec3f(0,0, 1)));
		assert(eq(sphereToCart(Vec3f( 2,0,0)), Vec3f(0,0, 2)));
		assert(eq(sphereToCart(Vec3f(-2,0,0)), Vec3f(0,0,-2)));
		assert(eq(sphereToCart(Vec3f(1,0,pi)), Vec3f(0,0,-1)));
		assert(eq(sphereToCart(Vec3f(1,0,pi/2)), Vec3f(1,0,0)));
		assert(eq(sphereToCart(Vec3f(1,0,-pi/2)), Vec3f(-1,0,0)));
		assert(eq(sphereToCart(Vec3f(1,pi/2,pi/2)), Vec3f(0,1,0)));
		assert(eq(sphereToCart(Vec3f(1,pi/2,-pi/2)), Vec3f(0,-1,0)));

		int N = 32;
		for(int j=0; j<N; ++j){
		for(int i=0; i<N; ++i){
			// Try to avoid points on xyz axes as they have ambiguous angles
			Vec3f a(1, (float(i+1)/(N+1) - 0.5f) * twoPi, float(j+1)/(N+1) * pi);
			auto p = sphereToCart(a);
			auto b = cartToSphere(p);
			//a.print(); p.print(); b.println();
			assert(eq(a, b));
		}}
	}

	{
		auto spharmGround = [](int l, int m, Complexd th, Complexd ph) -> Complexd{
			switch(l){
				case 0: return 0.5*sqrt(1./pi);
				case 1:
					switch(m){
						case  0: return  0.5*sqrt(3/ pi) * ph.r;
						case  1: return -0.5*sqrt(3/twoPi) * ph.i * th;
						case -1: return  0.5*sqrt(3/twoPi) * ph.i * th.conj();
						default:;
					}
				case 2:
					switch(m){
						case  0: return  0.25*sqrt( 5/ pi) * (3*ph.r*ph.r-1);
						case  1: return -0.50*sqrt(15/twoPi) * ph.r*ph.i * th;
						case -1: return  0.50*sqrt(15/twoPi) * ph.r*ph.i * th.conj();
						case  2: return  0.25*sqrt(15/twoPi) * pow2(th*ph.i);
						case -2: return  0.25*sqrt(15/twoPi) * pow2(th.conj()*ph.i);
						default:;
					}
				case 3:
					switch(m){
						case  0: return  0.250*sqrt(  7/ pi) * (5*pow3(ph.r) - 3*ph.r);
						case  1: return -0.125*sqrt( 21/ pi) * ph.i*(5*pow2(ph.r)-1) * th;
						case -1: return  0.125*sqrt( 21/ pi) * ph.i*(5*pow2(ph.r)-1) * th.conj();
						case  2: return  0.250*sqrt(105/twoPi) * pow2(ph.i)*ph.r * pow2(th);
						case -2: return  0.250*sqrt(105/twoPi) * pow2(ph.i)*ph.r * pow2(th.conj());
						case  3: return -0.125*sqrt( 35/ pi) * pow3(th*ph.i);
						case -3: return  0.125*sqrt( 35/ pi) * pow3(th.conj()*ph.i);
						default:;
					}
				default:;
			}
			return 0;
		};

		for(int l=0; l<4; ++l){
		for(int m=-l; m<=l; ++m){
		for(double th=0; th< pi; th+= pi/8){
		for(double ph=0; ph<twoPi; ph+=twoPi/8){
			Complexd cth = Polard(th);
			Complexd cph = Polard(ph);
			Complexd ms = al::spharm(l,m,cth,cph);
			Complexd tr = spharmGround(l,m,cth,cph);

			if(!al::within(ms.r-tr.r,-eps,eps) || !al::within(ms.i-tr.i,-eps,eps)){
				printf("\nY(%d,%d,%g,%g):\n\ttr=(% g, % g)\n\tms=(% g, % g)\n", l,m,th,ph, tr.r,tr.i, ms.r,ms.i);
				assert(false);
			}
		}}}}
	}
	return 0;
}


