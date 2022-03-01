/*
AlloCore Example: Random Distributions

Description:
Plots of random distributions.

Author:
Lance Putnam, 2022
*/

#include <cstdio>
#include <cmath>
#include "allocore/math/al_Random.hpp"
#include "allocore/system/al_Printing.hpp"
using namespace al;

int main(){

	#define PLOT_DIST(func){\
		int B = 16;\
		int N = 8192;\
		float bins[B];\
		for(auto& v : bins) v = 0;\
		rnd::Random<> rng;\
		for(int i=0; i<N; ++i){\
			auto r = rng.func;\
			if(0 <= r && r < 1){\
				int k = r*B;\
				bins[k]++;\
			}\
		}\
		float mx = -1e38;\
		for(auto v : bins) mx = mx>v ? mx : v;\
		for(auto& v : bins) v /= mx;\
		printf(#func ":\n");\
		for(auto v : bins){\
			printPlot(v); printf("\n");\
		}\
	}

	PLOT_DIST(uniform())
	PLOT_DIST(linear0())
	PLOT_DIST(linear1())
	PLOT_DIST(triangle()*0.5+0.5)
	PLOT_DIST(normal()*0.25+0.5)
	PLOT_DIST(prob(   )?0:0.99)
	PLOT_DIST(prob(0.2)?0:0.99)
}
