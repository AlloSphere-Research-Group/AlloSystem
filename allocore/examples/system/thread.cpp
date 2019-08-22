/*
AlloCore Example: Thread

Description:
Demonstration of basic usage of the Thread class.

Author:
Lance Putnam, 10/2012
*/

#include <stdio.h>
#include "allocore/system/al_Thread.hpp"
using namespace al;


int main(){

	// The main purpose of threads is to run complex operations in parallel
	// rather than sequentially.
	{
		Thread thread1, thread2;

		// Start both threads passing in functions that print a message.
		// The thread functions may begin now and the program continues without
		// waiting for the functions to return.
		thread1.start([](){ printf("Hello from thread 1!\n"); });
		thread2.start([](){ printf("Hello from thread 2!\n"); });

		// Print message from main thread; this waits until the function returns
		printf("Hello from main thread!\n");

		// Call 'join' to wait until the thread functions finish
		thread1.join();
		thread2.join();
	}

	// We may also pass a functor (function with persistant data) to a thread,
	// but must be a bit careful...
	{
		struct Functor{
			int i;
			void operator()(){ ++i; }
		};

		Functor functor{1};

		{	// The wrong way (passes copy of functor data)
			Thread thread(functor);
			thread.join();
			printf("functor.i is %d\n", functor.i); // a copy was incremented
		}

		{	// The right way (passes reference to functor data)
			Thread thread(std::ref(functor));
			thread.join();
			printf("functor.i is %d\n", functor.i); // actual object was incremented
		}
	}

}