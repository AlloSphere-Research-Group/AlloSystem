/*
Allocore Example: Periodic thread

Description:
This shows how to create a thread that performs an action every t seconds.

Author:
Lance Putnam, Nov. 2014
*/

#include <stdio.h>
#include "allocore/system/al_PeriodicThread.hpp"
using namespace al;

int main(){

	// Our thread function object
	struct Func : ThreadFunction{
	
		// Our thread function will just print the current time
		void operator()(){
			// Current minute in milliseconds
			unsigned ms = unsigned(al::timeNow() * 1000) % 60000;
			printf("%5u\n", ms);
		}
	};

	// This is a special thread that periodically calls its thread function.
	// This is unlike an ordinary thread which only calls its thread function
	// once.
	PeriodicThread t;

	// Set the calling period, in seconds
	t.period(1);

	// Bind a function to the thread and start the thread
	Func f;
	t.start(f);

	getchar();
}
