#include "utAllocore.h"

using namespace al;

THREAD_FUNCTION(threadFunc){
	*(int *)user = 1; return NULL;
}

template <class T>
bool aboutEqual(T v, T to, T r){ return v<(to+r) && v>(to-r); }

int utSystem(){

	// Thread
	{	int x=0;
		Thread t(threadFunc, &x);
		t.wait();
		assert(x == 1);
	}
	
	
	// Timing
	{
		al_nsec slop = 1e7;
		al_nsec sleepns = 1e8;
		al_sec sleeps = 0.5;
		al_sec slops = 0.005;

		assert(al_time_ns2s * 1e9 == 1);
		assert(al_time_s2ns * 1 == 1e9);
		
		al_nsec t, dt;
		al_sec now, dts;
		
		t = al_time_nsec();
		al_sleep_nsec(sleepns);
		dt = al_time_nsec() - t;
		assert(aboutEqual(dt, sleepns, slop));

		t = al_time_nsec();
		al_sleep(al_time_ns2s * sleepns);
		dt = al_time_nsec() - t;
		assert(aboutEqual(dt, sleepns, slop));

		now = al_time();
		al_sleep_until(now + sleeps);
		dts = al_time() - now;
		assert(aboutEqual(dts, sleeps, slops));

		Timer tm;
		
		tm.start();
		al_sleep_nsec(sleepns);
		tm.stop();
		dt = tm.elapsed();
		assert(aboutEqual(dt, sleepns, slop));
		assert(al_time_ns2s * tm.elapsed() == tm.elapsedSec());
	}

	return 0;
}
