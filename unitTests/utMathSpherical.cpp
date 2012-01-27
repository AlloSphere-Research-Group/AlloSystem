#include <math.h>
#include "utAllocore.h"

int utMathSpherical(){

	double eps = 1e-8;

	{
		struct F{
			static Complexd spharmGround(int l, int m, Complexd th, Complexd ph){
				switch(l){
					case 0: return 0.5*sqrt(1./M_PI);
					case 1:
						switch(m){
							case  0: return  0.5*sqrt(3/ M_PI) * ph.r;
							case  1: return -0.5*sqrt(3/M_2PI) * ph.i * th;
							case -1: return  0.5*sqrt(3/M_2PI) * ph.i * th.conj();
							default:;
						}
					case 2:
						switch(m){
							case  0: return  0.25*sqrt( 5/ M_PI) * (3*ph.r*ph.r-1);
							case  1: return -0.50*sqrt(15/M_2PI) * ph.r*ph.i * th;
							case -1: return  0.50*sqrt(15/M_2PI) * ph.r*ph.i * th.conj();
							case  2: return  0.25*sqrt(15/M_2PI) * pow2(th*ph.i);
							case -2: return  0.25*sqrt(15/M_2PI) * pow2(th.conj()*ph.i);
							default:;
						}
					case 3:
						switch(m){
							case  0: return  0.250*sqrt(  7/ M_PI) * (5*pow3(ph.r) - 3*ph.r);
							case  1: return -0.125*sqrt( 21/ M_PI) * ph.i*(5*pow2(ph.r)-1) * th;
							case -1: return  0.125*sqrt( 21/ M_PI) * ph.i*(5*pow2(ph.r)-1) * th.conj();
							case  2: return  0.250*sqrt(105/M_2PI) * pow2(ph.i)*ph.r * pow2(th);
							case -2: return  0.250*sqrt(105/M_2PI) * pow2(ph.i)*ph.r * pow2(th.conj());
							case  3: return -0.125*sqrt( 35/ M_PI) * pow3(th*ph.i);
							case -3: return  0.125*sqrt( 35/ M_PI) * pow3(th.conj()*ph.i);
							default:;
						}
					default:;
				}
				return 0;
			}
		};

		for(int l=0; l<4; ++l){
		for(int m=-l; m<=l; ++m){
		for(double th=0; th< M_PI; th+= M_PI/8){
		for(double ph=0; ph<M_2PI; ph+=M_2PI/8){
			Complexd cth = Polard(th);
			Complexd cph = Polard(ph);
			Complexd ms = al::spharm(l,m,cth,cph);
			Complexd tr = F::spharmGround(l,m,cth,cph);

			if(!al::within(ms.r-tr.r,-eps,eps) || !al::within(ms.i-tr.i,-eps,eps)){
				printf("\nY(%d,%d,%g,%g):\n\ttr=(% g, % g)\n\tms=(% g, % g)\n", l,m,th,ph, tr.r,tr.i, ms.r,ms.i);
				assert(false);
			}
		}}}}
	}
	return 0;
}


