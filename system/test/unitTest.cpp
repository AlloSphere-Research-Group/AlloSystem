#include <assert.h>
#include <stdio.h>
#include "al_thread.h"
#include "al_time.h"

using namespace allo;

THREAD_FUNCTION(threadFunc){
	*(int *)user = 1; return NULL;
}

template <class T>
bool aboutEqual(T v, T to, T r){ return v<(to+r) && v>(to-r); }

int main(int argc, char* argv[]){

	// Thread
	{	int x=0;
		Thread t(threadFunc, &x);
		t.wait();
		assert(x == 1);
	}
	
	
	// Timing
	{
		al_timing_init();
	
		al_nsec slop = 1e6;
		al_nsec sleepns = 1e8;

		assert(al_nsec2sec(1e9) == 1);
		assert(al_sec2nsec(1) == 1e9);
		
		al_nsec t,dt;
		
		t = al_time();
		al_sleep(sleepns);
		dt = al_time() - t;
		assert(aboutEqual(dt, sleepns, slop));

		t = al_time();
		al_sleep_sec(al_nsec2sec(sleepns));
		dt = al_time() - t;
		assert(aboutEqual(dt, sleepns, slop));

		t = al_time();
		al_sleep_until(t+sleepns);
		dt = al_time() - t;
		assert(aboutEqual(dt, sleepns, slop));

		Timer tm;
		
		tm.start();
		al_sleep(sleepns);
		tm.stop();
		dt = tm.elapsed();
		assert(aboutEqual(dt, sleepns, slop));
		assert(al_nsec2sec(tm.elapsed()) == tm.elapsedSec());

		al_timing_quit();
	}

	return 0;
}
