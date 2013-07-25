/*
AlloCore Example: Thread

Description:
Demontration of basic usage of the Thread class.

Author:
Lance Putnam, 10/2012
*/

#include <stdio.h>
#include "allocore/system/al_Thread.hpp"
using namespace al;


// We subclass ThreadFunction to define both the data and execution function of 
// the thread.
struct MyThreadFunction : public ThreadFunction{
	int i; // We'll use this integer to identify the thread

	MyThreadFunction(int i_=0): i(i_){}

	// This is the function that will get executed once the thread starts
	void operator()(){
		printf("Thread function %d\n", i);
	}
};


int main(){

	// First, we will show very simple use of a single thread.
	Thread thread;
	MyThreadFunction function1(1);
	
	// Start execution of the thread.
	// The program will continue immediately after starting the thread.
	thread.start(function1);
	
	// Join the thread with the main thread.
	// When we call join on a thread, the main program will wait until that 
	// thread is finished.
	thread.join();
	
	// We can reuse threads, possibly with other functions, as long as they are
	// not running.
	MyThreadFunction function2(2);
	thread.start(function2);
	thread.join();


	// Up to this point, we haven't really done any parallel processing which
	// is the whole point of using threads.

	// Declare multiple threads and functions
	const int N = 8;
	MyThreadFunction functions[N];
	Thread threads[N];

	// Initialize the data in our thread function objects
	for(int i=0; i<N; ++i){
		functions[i].i = i+10;
	}
	
	// Start all the threads. 
	// The threads will execute in parallel in an indeterminate order. Run the 
	// example multiple times to observe how the print statements get 
	// interleaved in a seemingly random way.
	printf("Starting multiple threads from the main thread.\n");
	for(int i=0; i<N; ++i){
		threads[i].start(functions[i]);
	}
	printf("All threads started.\n");

	// Join all the threads with the main thread.
	for(int i=0; i<N; ++i){
		printf("Joining %d\n", functions[i].i);
		threads[i].join();
	}

	printf("All threads joined.\n");
}
