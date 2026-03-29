#ifndef INC_AL_TIME_HPP
#define INC_AL_TIME_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	C++ helper wrappers for al_time

	Author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <cmath>
#include <string>
#include "allocore/system/al_Time.h"

namespace al{

/// @addtogroup allocore
/// @{

/// Sleep for an interval of seconds
inline void wait(al_sec dt){ al_sleep(dt); }

/// Get current system time in seconds
inline al_sec timeNow(){ return al_system_time(); }
inline al_sec walltime(){ return timeNow(); }


/// Convert nanoseconds to timecode string

/// @param[in] t		nanosecond time to convert
/// @param[in] format	String describing output format.
/// The following special characters output the following values:
///		D	date, as yyyynndd
///		y	year
///		n	month
///		d	day
///		H	hour
///		M	minute
///		S	second
///		m	millisecond
///		u	microsecond	
///
///	All other characters are preserved in the output (as delimiters).
std::string toTimecode(al_nsec t, const std::string& format="D:H:M:S:m:u");

/// Get timecode of current system time
std::string timecodeNow(const std::string& format="D:H:M:S:m:u");


/// A Gregorian date and time
struct Timestamp{
	unsigned year;			///< Year in Gregorian calendar
	unsigned char mon;		///< Month in [1,12]
	unsigned char day;		///< Day of month in [1,31]
	unsigned char hour;		///< Hour of day in [0,23]
	unsigned char min;		///< Minute in [0,59]
	unsigned char sec;		///< Second in [0,59]
	unsigned short msec;	///< Millsecond in [0,999]
	unsigned short usec;	///< Microsecond in [0,999]

	/// No initialization

	/// Calling valid() on this object will return false.
	///
	Timestamp();

	/// Initialize from nanosecond time
	Timestamp(al_nsec t);

	/// Get timestamp using current time
	static Timestamp now();

	/// Set from nanoseconds
	Timestamp& operator=(al_nsec t);
	
	/// Return whether timestamp is valid
	bool valid() const;
};


/// Timer with stopwatch-like functionality for benchmarking, etc.
class Timer {
public:
	Timer(bool setStartTime=true){
		if(setStartTime) start();
	}

	/// Returns nsec elapsed from start() call
	al_nsec elapsed() const { return (mStop<mStart ? getTime() : mStop) - mStart; }					

	/// Returns seconds elapsed from start() call
	al_sec elapsedSec() const { return al_time_ns2s * elapsed(); }

	/// Set start time to current time
	Timer& start(){ mStart = getTime(); return *this; }

	/// Set stop time to current time
	Timer& stop(){ mStop = getTime(); return *this; }

	/// Time a function

	/// @param[in] blockSize	number of function calls within one timing block
	/// @param[in] trials		number of blocks to trial to get lowest time
	/// @param[in] func			function to time, takes call count as input
	///							(to vary function arguments)
	///
	/// \returns the lowest time in nanoseconds to execute one block
	template <class Func>
	al_nsec timeFunc(int blockSize, int trials, const Func& func){
		auto bestTime = al_nsec(1)<<(sizeof(al_nsec)*8-2); // a big value
		for(int j=0; j<trials; ++j){
			start();
			for(int i=0; i<blockSize; ++i){
				volatile auto res = func(i);
			}
			stop();
			auto time = elapsed();
			if(time < bestTime) bestTime = time;
		}
		return bestTime;
	}

	/// Print current elapsed time
	void print() const;

private:
	al_nsec mStart=0, mStop=0;	// start and stop times
	static al_nsec getTime(){ return al_steady_time_nsec(); }
};


/// Helper macro to print timing of an expression

/// @param[in] blockSize	size of test block
/// @param[in] trials		number of blocks to measure for lowest time
/// @param[in] unit			'n' for nanoseconds,
///							'u' for microseconds,
///							'm' for milliseconds
/// @param[in] ...			expression to time
#define AL_PRINT_EXPR_TIME(blockSize, trials, unit, ...){\
	unsigned long unitDiv = 1;\
	const char * unitStr = "ns";\
	switch(unit){\
	case 'u': unitDiv=   1000; unitStr="us"; break;\
	case 'm': unitDiv=1000000; unitStr="ms"; break;\
	default:;\
	}\
	std::cout << #__VA_ARGS__ ": " << al::Timer().timeFunc(blockSize,trials, [&](int i){ return __VA_ARGS__; })/unitDiv << " " << unitStr << "\n";\
}


/// Interval timer

/// This generates a trigger whenever a specified interval passes. It may be
/// periodic or one-shot.
class ITimer{
public:
	ITimer(float interval=1, bool oneShot=false, bool active=true)
	:	mInterval(interval), mActive(active), mPeriodic(!oneShot)
	{}

	
	/// Get whether timer is active
	bool active() const { return mActive; }

	/// Get trigger interval
	float interval() const { return mInterval; }

	/// Get current accumulator time
	float time() const { return mTime; }

	/// Returns whether timer triggered on last update
	bool triggered() const { return mTriggered; }


	/// Set whether timer is active
	ITimer& active(bool v){ mActive=v; return *this; }

	/// Toggle whether timer is active
	ITimer& toggle(){ mActive^=true; return *this; }

	/// Set trigger interval
	ITimer& interval(float v){ mInterval=v; return *this; }

	/// Set current accumulator time
	ITimer& time(float v){ mTime=v; return *this; }

	/// Set one-shot or periodic mode
	ITimer& oneShot(bool v){ mPeriodic=!v; return *this; }

	/// Reset timer (with optional starting value)
	ITimer& reset(float start=0.){ return time(start); }

	/// Increment time by dt and return whether interval passed on this update
	bool operator()(float dt){
		mTriggered = false;
		if(mActive){
			mTime += dt;
			if(mTime >= mInterval){
				if(mPeriodic) mTime = std::fmod(mTime, mInterval);
				mTriggered = true;
			}
		}
		return mTriggered;
	}

private:
	float mTime = 0;
	float mInterval;
	bool mTriggered = false;
	bool mActive = true;
	bool mPeriodic;
};

/// @} // end allocore group

} // al::
#endif
