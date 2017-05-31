#include <algorithm>
#include "allocore/system/al_PeriodicThread.hpp"

namespace al{

PeriodicThread::PeriodicThread(double periodSec, bool oneShot, bool triggerOnStart)
    : mAutocorrect(0.1),
      mOneShot(oneShot),
      mTriggerOnStart(triggerOnStart),
      mUserData(nullptr)
{
	period(periodSec);
	if (mOneShot) {
		mTriggerOnStart = false; // It doesn't make sense to have trigger on start enabled with one shot....
	}
}

PeriodicThread::PeriodicThread(const PeriodicThread& o)
:	Thread(o), mPeriod(o.mPeriod), mTimeCurr(o.mTimeCurr),
	mTimePrev(o.mTimePrev), mWait(o.mWait), mTimeBehind(o.mTimeBehind),
	mAutocorrect(o.mAutocorrect),
    mOneShot(o.mOneShot),
    mTriggerOnStart(o.mTriggerOnStart),
	mUserFunc(o.mUserFunc),
    mUserData(nullptr),
	mRun(o.mRun)
{}



void * PeriodicThread::sPeriodicFunc(void * threadData){
	static_cast<PeriodicThread *>(threadData)->go();
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

void PeriodicThread::start(ThreadFunction& func, void *userData){
	mUserFunc = &func;
	mUserData = userData;
	mRun = true;
	Thread::start(sPeriodicFunc, this);
}

void PeriodicThread::start(std::function<void(void *data)> function, void *userData){
	if(mRun) {
		stop();
	}
	mFunction = function;
	mUserData = userData;
	mRun = true;
	Thread::start(sPeriodicFunc, this);
}

void PeriodicThread::stop(){
	mRun = false;
	mUserFunc = nullptr;
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
	bool firstPass = true;
	while(mRun){
		if (mTriggerOnStart || !firstPass) {
			if (mUserFunc) {
				(*mUserFunc)();
			} else if(mFunction) {
				mFunction(mUserData);
			}
		}

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
		if (mOneShot && !firstPass) {
			mRun = false;
		}
		firstPass = false;
		//printf("period=%g, dt=%g, wait=%g\n", mPeriod, dt, mWait);
	}
}

} // al::
