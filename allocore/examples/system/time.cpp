/*
AlloCore Example: Time

Description:
Demonstration of various time related functions

Author:
Lance Putnam, March 2023
*/

#include <iostream>
#include "allocore/system/al_Time.hpp"
using namespace al;

#define PRINT_EXPR(...) std::cout << #__VA_ARGS__ ": " << (__VA_ARGS__) << std::endl;

int main(){
	// System time is the same as calendar time; low precision
	PRINT_EXPR(al_system_time())
	PRINT_EXPR(al_system_time_nsec())
	// Same as system time
	PRINT_EXPR(timeNow())
		
	// Steady time starts at program launch; high precision, monotonic
	PRINT_EXPR(al_steady_time())
	PRINT_EXPR(al_steady_time_nsec())
	
	// Get timecode of current system time as a string
	PRINT_EXPR(timecodeNow())
	// Just the date
	PRINT_EXPR(timecodeNow("D"))
	// Prettier date
	PRINT_EXPR(timecodeNow("y-n-d"))
	// Just a basic clock time
	PRINT_EXPR(timecodeNow("H:M:S"))
	
	// Timestamp can be used to get a numerical breakdown of a time
	{	// Typically, we want the current calendar time
		auto ts = Timestamp::now();
		//Timestamp ts(al_system_time_nsec()); // equivalent to above
		std::cout << "Timestamp from current system time:\n";
		PRINT_EXPR((int)ts.year)
		PRINT_EXPR((int)ts.mon)
		PRINT_EXPR((int)ts.day)
		PRINT_EXPR((int)ts.hour)
		PRINT_EXPR((int)ts.min)
		PRINT_EXPR((int)ts.sec)
	}
	{	// A Timestamp can be set from any nanosecond time
		Timestamp ts(al_steady_time_nsec());
		std::cout << "Timestamp from current steady time:\n";
		PRINT_EXPR((int)ts.year)
		PRINT_EXPR((int)ts.mon)
		PRINT_EXPR((int)ts.day)
		PRINT_EXPR((int)ts.hour)
		PRINT_EXPR((int)ts.min)
		PRINT_EXPR((int)ts.sec)
		PRINT_EXPR((int)ts.msec)
		PRINT_EXPR((int)ts.usec)
	}
}