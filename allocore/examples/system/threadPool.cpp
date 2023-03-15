/*
AlloCore Example: Thread Pool

Description:
Demonstration of the ThreadPool class. A thread pool is a group of threads that is created once and used many times to avoid the overhead of thread creation. With a thread pool, parallel processing tasks can be executed in a near optimal fashion.

Author:
Lance Putnam, March 2023
*/

#include <cstdio>
#include "allocore/system/al_ThreadPool.hpp"
using namespace al;


int main(){

	{
		// The default constructor creates a pool matching hardware concurrency
		//ThreadPool p;

		// You can also specify the size of the pool
		//ThreadPool p(16);

		// This is a thread-local pool; use it whenever possible to avoid creating new pools
		auto& p = ThreadPool::get();

		printf("Thread pool size: %d\n", p.size());

		// Push tasks; execution begins immediately
		p.push([](){ printf("Hello\n"); });
		p.push([](){ printf("World\n"); });

		// Block until all tasks are finished
		p.wait();

		// We can push as many tasks as we like; they are executed when threads become available
		for(int i=0; i<100; ++i)
			p.push([i](){ printf("%d ", i); });

		// All threads are joined when the ThreadPool is destroyed
	}

	{ // Here we do some basic parallel processing
		int N = 100000;
		auto * array = new double[N];

		// This is a parallel for loop
		// pushRange runs one contiguous chunk of the for loop on each thread.
		// We specify how many iterations and a function to be called for each iteration.
		ThreadPool::get().pushRange(N,
			[array](int i){
				array[i] = double(i)*i;
			}
		).wait();

		// Inspect some of the results
		printf("\n");
		for(int i=0; i<N; i+=(N/16))
			printf("[%5d] %f\n", i, array[i]);

		delete[] array;
	}

	{ // Compute the sum of integers from 1 to N, in parallel
		auto& p = ThreadPool::get();

		// We create buckets to store the result from each thread
		auto * buckets = new double[p.size()];
		for(int i=0; i<p.size(); ++i) buckets[i] = 0;

		int N = 100000;

		p.pushRange(N,
			[buckets](int i, int threadID){
				buckets[threadID] += i+1;
			}
		).wait();

		double sum = 0.;
		for(int i=0; i<p.size(); ++i) sum += buckets[i];

		printf("Sum of 1 to %d: %f\n", N, sum);

		delete[] buckets;
	}
}