#include <sstream> // ostringstream
#include <iomanip> // std::setw
#include <stdio.h> // printf
#include "allocore/math/al_Constants.hpp"
#include "allocore/system/al_Time.hpp"

/* Windows */
#if defined(AL_WINDOWS)
#include <windows.h>
/*
Info on Windows timing:
http://windowstimestamp.com/description
*/
/*
// singleton object to force init/quit of timing
static struct TimeSingleton{
	TimeSingleton(){ timeBeginPeriod(1); }
	~TimeSingleton(){ timeEndPeriod(1); }
} timeSingleton;

// interface to Windows API
static DWORD time_ms(){ return timeGetTime(); }
static void sleep_ms(unsigned long long ms){ Sleep(DWORD(ms)); }

// allocore definitions
al_sec al_time(){				return time_ms() * 1e-3; }
al_nsec al_time_nsec(){			return al_nsec(time_ms()) * al_nsec(1e6); }
void al_sleep(al_sec v){		sleep_ms(v * 1e3); }
void al_sleep_nsec(al_nsec v){	sleep_ms(v / 1e6); }
//*/

//*
static void sleep_ms(unsigned long long ms){ Sleep(DWORD(ms)); }

// Method to supposedly get microsecond sleep
// From: http://blogs.msdn.com/b/cellfish/archive/2008/09/17/sleep-less-than-one-millisecond.aspx
/*
#include <Winsock2.h> // SOCKET
static int sleep_us(long usec){
	static bool first = true;
	if(first){
		first = false;
		WORD wVersionRequested = MAKEWORD(1,0);
		WSADATA wsaData;
		WSAStartup(wVersionRequested, &wsaData);
	}
	struct timeval tv;
	fd_set dummy;
	SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	FD_ZERO(&dummy);
	FD_SET(s, &dummy);
	tv.tv_sec = usec/1000000L;
	tv.tv_usec = usec%1000000L;
	return select(0, 0, 0, &dummy, &tv);
}*/

// system time as 100-nanosecond interval
al_nsec system_time_100ns(){
	SYSTEMTIME st;
	GetSystemTime(&st);
	//printf("%d/%d/%d %d:%d:%d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
	FILETIME time;
	SystemTimeToFileTime(&st, &time);
	//GetSystemTimeAsFileTime(&time);
	ULARGE_INTEGER timeULI = {{time.dwLowDateTime, time.dwHighDateTime}};
	al_nsec res = timeULI.QuadPart; // in 100-nanosecond intervals
	res -= al_nsec(116444736000000000); // convert epoch from 1601 to 1970
	return res;
}

al_nsec steady_time_us(){
	// Windows 10 and above
	//PULONGLONG time;
	//QueryInterruptTimePrecise(&time);

	LARGE_INTEGER freq; // ticks/second
	LARGE_INTEGER time; // tick count
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&time);

	// max 32-bit int:                2,147,483,648  = 2.1e09
	// max 64-bit int:    9,223,372,036,854,775,808  = 9.2e18
	// max double as int:     9,007,199,254,740,992  = 9.0e15

	// freq.QuadPart: 3,328,129
	//printf("freq.QuadPart: %I64d ticks/sec\n", freq.QuadPart);

	// As long as time.QuadPart < 9.0e15, this will work:
	return al_nsec(time.QuadPart / double(freq.QuadPart) * 1.0e6);

	/* Note this approach can overflow as time.QuadPart may be > 1e13
	// convert ticks to microseconds
	time.QuadPart *= 1000000; // 1e6
	time.QuadPart /= freq.QuadPart;
	return al_nsec(time.QuadPart);
	*/

	/* Correct integer-only approach
	LONG64 hi;
	LONG64 lo = Multiply128(time.QuadPart, 1000000, &hi);
	//*/

}

al_sec  al_system_time(){		return system_time_100ns() * 1e-7; }
al_nsec al_system_time_nsec(){	return system_time_100ns() * al_nsec(100); }
al_sec  al_steady_time(){		return steady_time_us() * 1e-6; }
al_nsec al_steady_time_nsec(){	return steady_time_us() * al_nsec(1e3); }
void al_sleep(al_sec v){		sleep_ms(v * 1e3); }
void al_sleep_nsec(al_nsec v){	sleep_ms(v / 1e6); }
//void al_sleep(al_sec v){		sleep_us(v * 1e6); }
//void al_sleep_nsec(al_nsec v){	sleep_us(v / 1e3); }
//*/

/* Posix (Mac, Linux) */
#else
#include <time.h> // nanosleep, clock_gettime
#include <sys/time.h> // gettimeofday
#include <unistd.h> // _POSIX_TIMERS

al_sec  al_system_time(){
	timeval t;
	gettimeofday(&t, NULL);
	return al_sec(t.tv_sec) + al_sec(t.tv_usec) * 1e-6;
}

al_nsec al_system_time_nsec(){
	timeval t;
	gettimeofday(&t, NULL);
	return al_nsec(t.tv_sec) * al_nsec(1e9) + al_nsec(t.tv_usec) * al_nsec(1e3);
}

#ifdef AL_OSX
#include <mach/mach_time.h>

// Code from:
// http://stackoverflow.com/questions/23378063/how-can-i-use-mach-absolute-time-without-overflowing

al_sec al_steady_time(){
	return al_steady_time_nsec() * 1e-9;
}

al_nsec al_steady_time_nsec(){
	uint64_t now = mach_absolute_time();
	static struct Data {
		Data(uint64_t bias_) : bias(bias_) {
			mach_timebase_info(&tb);
			if (tb.denom > 1024) {
				double frac = (double)tb.numer/tb.denom;
				tb.denom = 1024;
				tb.numer = tb.denom * frac + 0.5;
			}
		}
		mach_timebase_info_data_t tb;
		uint64_t bias;
	} data(now);
	return (now - data.bias) * data.tb.numer / data.tb.denom;
}

// Posix timers available?
#elif _POSIX_TIMERS > 0 && defined(_POSIX_MONOTONIC_CLOCK)
al_sec al_steady_time(){
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return al_sec(t.tv_sec) + al_sec(t.tv_nsec) * 1e-9;
}

al_nsec al_steady_time_nsec(){
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return al_nsec(t.tv_sec) * al_nsec(1e9) + al_nsec(t.tv_nsec);
}

// Otherwise fallback to system time
#else
al_sec  al_steady_time(){
	return al_system_time();
}

al_nsec al_steady_time_nsec(){
	return al_system_time_nsec();
}
#endif


void al_sleep(al_sec v) {
	time_t sec = time_t(v);
	al_nsec nsec = al_time_s2ns * (v - al_sec(sec));
	timespec tspec = { sec, long(nsec) };
	while (nanosleep(&tspec, &tspec) == -1)
		continue;
}

void al_sleep_nsec(al_nsec v) {
	al_sleep(al_sec(v) * al_time_ns2s);
}

#endif
/* end platform specific */


void al_sleep_until(al_sec target) {
	al_sec dt = target - al_time();
	if (dt > 0) al_sleep(dt);
}

al_sec  al_time(){				return al_system_time(); }
al_nsec al_time_nsec(){			return al_system_time_nsec(); }



namespace al {

struct Date{
	int year, month, day;
};

// z is number of days since 1970-01-01
// From http://howardhinnant.github.io/date_algorithms.html
Date daysToDate(int z){
	Date date;
	auto &y = date.year, &m = date.month, &d = date.day;
	z += 719468;
	const int era = (z >= 0 ? z : z - 146096) / 146097;
	const unsigned doe = static_cast<unsigned>(z - era * 146097);          // [0, 146096]
	const unsigned yoe = (doe - doe/1460 + doe/36524 - doe/146096) / 365;  // [0, 399]
	y = static_cast<int>(yoe) + era * 400;
	const unsigned doy = doe - (365*yoe + yoe/4 - yoe/100);                // [0, 365]
	const unsigned mp = (5*doy + 2)/153;                                   // [0, 11]
	d = doy - (153*mp+2)/5 + 1;                             // [1, 31]
	m = mp + (mp < 10 ? 3 : -9);                            // [1, 12]
	y = y + (m <= 2);
	return date;
}

Timestamp::Timestamp(): mon(255){}

Timestamp::Timestamp(al_nsec t){
	auto date = daysToDate(
		t/(al_nsec(1000000000) * 60 * 60 * 24) // basically for overflow
	);
	year = date.year;
	mon = date.month;
	day = date.day;
	hour = t/(al_nsec(1000000000) * 60 * 60) % 24;
	min  = t/(al_nsec(1000000000) * 60) % 60;
	sec  = t/(al_nsec(1000000000)) % 60;
	msec = t/(al_nsec(1000000)) % 1000;
	usec = t/(al_nsec(1000)) % 1000;
}

bool Timestamp::valid() const {
	if(1<=mon && mon<=12){
		static unsigned char dpmLUT[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
		auto dpm = dpmLUT[mon-1];
		if(2==mon){
			int leap = 0;
			if(year%4 == 0){
				leap = 1;
				if(year%100 == 0){
					leap = 0;
					if(year%400) leap = 1;
				}
			}
			dpm += leap;
		}
		return 1<=day && day<=dpm && hour<60 && min<60 && sec<60 && msec<1000 && usec<1000; 
	}
	return false;
}

/*static*/ Timestamp Timestamp::now(){
	return Timestamp(al_system_time_nsec());
}


std::string toTimecode(al_nsec t, const std::string& format){
	Timestamp ts(t);

	std::ostringstream s;
	s.fill('0');

	for(unsigned i=0; i<format.size(); ++i){
		const auto c = format[i];
		switch(c){
		case 'D':
			s << std::setw(4) << (unsigned)ts.year;
			s << std::setw(2) << (unsigned)ts.mon;
			s << std::setw(2) << (unsigned)ts.day;
			break;
		case 'y': s << std::setw(4) << (unsigned)ts.year; break;
		case 'n': s << std::setw(2) << (unsigned)ts.mon; break;
		case 'd': s << std::setw(2) << (unsigned)ts.day; break;
		case 'H': s << std::setw(2) << (unsigned)ts.hour; break;
		case 'M': s << std::setw(2) << (unsigned)ts.min; break;
		case 'S': s << std::setw(2) << (unsigned)ts.sec; break;
		case 'm': s << std::setw(3) << (unsigned)ts.msec; break;
		case 'u': s << std::setw(3) << (unsigned)ts.usec; break;
		default:  s << c; // delimiter
		}
	}

	return s.str();
}

std::string timecodeNow(const std::string& format){
	return toTimecode(al_system_time_nsec(), format);
}


void Timer::print() const {
	auto dtSec = elapsedSec();
	printf("%g sec (%g ms) elapsed\n", dtSec, dtSec*1000.);
}


void DelayLockedLoop :: setBandwidth(double bandwidth) {
	double F = 1./tperiod;		// step rate
	double omega = M_PI * 2.8 * bandwidth/F;
	mB = omega * sqrt(2.);	// 1st-order weight
	mC = omega * omega;		// 2nd-order weight
}

void DelayLockedLoop :: step(al_sec realtime) {
	if (mReset) {
		// The first iteration sets initial conditions.

		// init loop
		t2 = tperiod;
		t0 = realtime;
		t1 = t0 + t2;	// t1 is ideally the timestamp of the next block start

		// subsequent iterations use the other branch:
		mReset = false;
	} else {
		// read timer and calculate loop error
		// e.g. if t1 underestimated, terr will be
		al_sec terr = realtime - t1;
		// update loop
		t0 = t1;				// 0th-order (distance)
		t1 += mB * terr + t2;	// integration of 1st-order (velocity)
		t2 += mC * terr;		// integration of 2nd-order (acceleration)
	}

//		// now t0 is the current system time, and t1 is the estimated system time at the next step
//		//
//		al_sec tper_estimate = t1-t0;	// estimated real duration between this step & the next one
//		double factor = tperiod/tper_estimate;	// <1 if we are too slow, >1 if we are too fast
//		double real_rate = 1./tper_estimate;
//		al_sec tper_estimate2 = t2;	// estimated real duration between this step & the next one
//		double factor2 = 1./t2;	// <1 if we are too slow, >1 if we are too fast
//		printf("factor %f %f rate %f\n", factor, factor2, real_rate);
}

} // al::
