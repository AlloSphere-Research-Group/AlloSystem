/*
Allocore Example: Parallel thread computation

Description:
This example shows how to spawn multiple threads to perform a calculation-
computing the mean of an array. The task is broken into individual work units
that compute the sum of a unique sub-interval of the entire array. When the
individual workers are done, their results are combined into the final result.

Author:
Lance Putnam, 1/2012
*/

#include <stdio.h>
#include "allocore/system/al_Thread.hpp"
using namespace al;

// The function each worker thread will execute
struct Func{

	void operator()(){
		printf("Summing indices %u to %u ...\n", ival[0], ival[1]-1);
		for(unsigned i=ival[0]; i<ival[1]; ++i) sum += data[i];
	}

	unsigned ival[2];		// processing interval
	double sum = 0.;		// final result
	const double * data;	// read-only data
};


int main(){
	int Nthreads = 7;			// number of worker threads
	const unsigned N = 100000;	// size of our array
	double data[N];

	// Fill array with polynomial
	for(unsigned i=0; i<N; ++i){
		double f = double(i)/N;
		data[i] = 1 - f*f;
	}

	Threads<Func> threads(Nthreads);

	// Setup worker thread summation intervals
	for(int i=0; i<Nthreads; ++i){
		threads.getInterval(threads.function(i).ival, i, N);
		threads.function(i).data = data;
	}

	// Compute in parallel
	// By default, the threads join the main thread when finished
	threads.start();

	// Combine worker results
	double sumPll = 0;
	for(int i=0; i<Nthreads; ++i){
		sumPll += threads.function(i).sum;
	}

	// Compute serially for verification
	double sumSer = 0;
	for(unsigned i=0; i<N; ++i){
		sumSer += data[i];
	}

	printf("Correct : %g\n", sumSer/N);
	printf("Computed: %g\n", sumPll/N);
}
