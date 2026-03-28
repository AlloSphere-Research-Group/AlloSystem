#ifndef INC_AL_PERIODIC_THREAD_HPP
#define INC_AL_PERIODIC_THREAD_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Thread that calls a function periodically

	Author(s):
	Lance Putnam, 2013, putnam.lance@gmail.com
*/

#include <functional> // std::function
#include "allocore/system/al_Thread.hpp"
#include "allocore/system/al_Config.h" // al_nsec

namespace al{

/// @addtogroup allocore
/// @{

/// Thread that calls a function periodically

/// The sleep time is dynamically adjusted based on the time taken by the
/// user-supplied thread function. This prevents drift that would occur in a
/// more simplistic implementation using a fixed sleep interval.
///
/// Upon destruction, the thread loop will be stopped and, by default, joined.
class PeriodicThread : public Thread{
public:

	/// @param[in] periodSec	calling period in seconds
	PeriodicThread(double periodSec=1);

	/// Copy constructor
	PeriodicThread(const PeriodicThread& other);

	~PeriodicThread();


	/// Set autocorrection factor

	/// This parameter is used to compensate for occasional iterations that take
	/// longer than the expected iteration period. Smaller values mean the
	/// timing corrections will be spread over a larger number of iterations.
	/// If all iterations take longer than the period, then no autocorrection
	/// measures will be able to make up for the lost time.
	///
	/// @param[in] factor	Maximum fraction of one period, in [0,1], to try to
	///						make up each iteration if behind on timing.
	PeriodicThread& autocorrect(float factor);

	/// Set period, in seconds
	PeriodicThread& period(double sec);

	/// Get period, in seconds
	double period() const;

	/// Start calling the supplied function periodically
	void start(Thread::Function func);

	/// Stop the thread
	void stop();


	// Stuff for assignment
	friend void swap(PeriodicThread& a, PeriodicThread& b);
	PeriodicThread& operator= (PeriodicThread other);

private:
	void go();

	al_nsec mPeriod;
	al_nsec mTimeCurr, mTimePrev;	// time measurements between frames
	al_nsec mWait;					// actual time to sleep between frames
	al_nsec mTimeBehind;
	float mAutocorrect;
	Thread::Function mUserFunc;
	bool mRun;
};

/// @} // end allocore group

} // al::
#endif
