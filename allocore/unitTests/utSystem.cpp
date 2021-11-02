#include "utAllocore.h"
#include "allocore/system/al_Time.hpp"

template <class T>
bool aboutEqual(T v, T to, T r){ return v<(to+r) && v>(to-r); }

int utSystem(){
	using namespace al;

	// Timing
//	UT_PRINTF("system: timing\n");
	{
		al_nsec slop = 2e7;
		al_nsec sleepns = 1e8;

		assert(al_time_ns2s * 1e9 == 1);
		assert(al_time_s2ns * 1 == 1e9);

		{
			auto t = al_steady_time_nsec();
			al_sleep_nsec(sleepns);
			auto dt = al_steady_time_nsec() - t;
			assert(aboutEqual(dt, sleepns, slop));
		}

		{
			auto t = al_steady_time_nsec();
			al_sleep(al_time_ns2s * sleepns);
			auto dt = al_steady_time_nsec() - t;
			assert(aboutEqual(dt, sleepns, slop));
		}

		{
			// Note that system time is not as precise as steady time, so our
			// error threshold is higher.
			al_sec sleeps = 0.5;
			auto t = al_system_time();
			al_sleep_until(t + sleeps);
			auto dt = al_system_time() - t;
			assert(aboutEqual(dt, sleeps, 15./1000.));
		}

		{
			Timer tm;
			tm.start();
			al_sleep_nsec(sleepns);
			tm.stop();
			auto dt = tm.elapsed();
			assert(aboutEqual(dt, sleepns, slop));
			assert(al_time_ns2s * tm.elapsed() == tm.elapsedSec());
		}
	}

	return 0;
}
