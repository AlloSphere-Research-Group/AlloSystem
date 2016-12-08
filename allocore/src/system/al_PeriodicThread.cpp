#include <algorithm>
#include "allocore/system/al_PeriodicThread.hpp"

namespace al{

PeriodicThread::PeriodicThread(double periodSec)
:	mAutocorrect(0.1)
{
	period(periodSec);
}

PeriodicThread::PeriodicThread(const PeriodicThread& o)
:	Thread(o), mPeriod(o.mPeriod), mTimeCurr(o.mTimeCurr),
	mTimePrev(o.mTimePrev), mWait(o.mWait), mTimeBehind(o.mTimeBehind),
	mAutocorrect(o.mAutocorrect),
	mUserFunc(o.mUserFunc),
	mRun(o.mRun)
{}



void * PeriodicThread::sPeriodicFunc(void * userData){
	static_cast<PeriodicThread *>(userData)->go();
	return NULL;
}

PeriodicThread& PeriodicThread::autocorrect(float factor){
	mAutocorrect=factor;
	return *this;
}

PeriodicThread& PeriodicThread::period(double sec){
	mPeriod=sec * 1e9;
	return *this;
}

double PeriodicThread::period() const {
	return mPeriod * 1e-9;
}

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
	// Note: times are al_nsec (long long int)
	mTimeCurr = al_steady_time_nsec();
	mWait = 0;
	mTimeBehind = 0;
	while(mRun){
		(*mUserFunc)();

		mTimePrev = mTimeCurr + mWait;
		mTimeCurr = al_steady_time_nsec();
		al_nsec dt = mTimeCurr - mTimePrev;
		// dt -> t_curr - (t_prev + wait)

		// The wait amount is the ideal period minus the actual amount
		// of time spent processing between iterations
		if(dt<mPeriod){
			mWait = mPeriod - dt;
			al_sleep_nsec(mWait);
		}

		// This means we are behind, so don't wait
		else{
			mWait = 0;
			mTimeBehind += dt - mPeriod;
		}

		if(mTimeBehind > 0){
			al_nsec comp = mPeriod*mAutocorrect;
			if(mTimeBehind < comp){
				comp = mTimeBehind;
				mTimeBehind = 0;
			}
			else{
				mTimeBehind -= comp;
			}

			mWait -= comp;
		}

		//printf("period=%g, dt=%g, wait=%g\n", mPeriod, dt, mWait);
	}
}

} // al::
