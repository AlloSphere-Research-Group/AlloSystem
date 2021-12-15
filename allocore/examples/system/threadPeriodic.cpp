/*
Allocore Example: Periodic thread

Description:
This shows how to create a thread that performs an action every t seconds.

Author:
Lance Putnam, Nov. 2014
*/

#include <stdio.h>
#include "allocore/system/al_PeriodicThread.hpp"
#include "allocore/system/al_Time.hpp"
using namespace al;

int main(){
	// This is a special thread that periodically calls its thread function.
	// This is unlike an ordinary thread which only calls its thread function
	// once.
	PeriodicThread t;

	// Set the calling period, in seconds
	t.period(1);

	// Bind a function to the thread and start the thread
	t.start([](){
		// Current minute in milliseconds
		unsigned ms = unsigned(al::timeNow() * 1000) % 60000;
		printf("%5u\n", ms);
	});

	getchar();
}
