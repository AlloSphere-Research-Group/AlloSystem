#include <algorithm>
#include "allocore/system/al_PeriodicThread.hpp"

namespace al{

PeriodicThread::PeriodicThread(double periodSec)
{
	period(periodSec);
}

PeriodicThread::PeriodicThread(const PeriodicThread& other)
:	Thread(other), mPeriod(other.mPeriod), mTimeCurr(other.mTimeCurr),
	mTimePrev(other.mTimePrev), mWait(other.mWait), mUserFunc(other.mUserFunc),
	mRun(other.mRun)
{}



void * PeriodicThread::sPeriodicFunc(void * userData){
	static_cast<PeriodicThread *>(userData)->go();
	return NULL;
}

PeriodicThread& PeriodicThread::period(double sec){ mPeriod=sec * 1e9; return *this; }

double PeriodicThread::period() const { return mPeriod * 1e-9; }

void PeriodicThread::start(ThreadFunction& func){
	mUserFunc = &func;
	mRun = true;
	Thread::start(sPeriodicFunc, this);
}

void PeriodicThread::stop(){
	mRun = false;
}


void swap(PeriodicThread& a, PeriodicThread& b){
	using std::swap;
	swap(static_cast<Thread&>(a), static_cast<Thread&>(b));
	#define SWAP_(x) swap(a.x, b.x);
	SWAP_(mPeriod);
	SWAP_(mTimeCurr);
	SWAP_(mTimePrev);
	SWAP_(mWait);
	SWAP_(mUserFunc);
	SWAP_(mRun);
	#undef SWAP_
}

PeriodicThread& PeriodicThread::operator= (PeriodicThread other){
	swap(*this, other);
	return *this;
}

void PeriodicThread::go(){
	mTimeCurr = al_time_nsec();
	mWait = 0;
	while(mRun){
		(*mUserFunc)();

		mTimePrev = mTimeCurr + mWait;
		mTimeCurr = al_time_nsec();
		al_nsec dt = mTimeCurr - mTimePrev;

		// The wait amount is the ideal period minus the actual amount
		// of time spent processing between iterations
		if(dt<mPeriod){
			mWait = mPeriod - dt;
			al_sleep_nsec(mWait);
		}

		// This means we are behind, so don't wait
		else{
			//mTimeBehind += dt - mPeriod;
			mWait = 0;
		}
		//printf("period=%g, dt=%g, wait=%g\n", mPeriod, dt, mWait);
	}
}

} // al::
