#include <map>
#include <string>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "allocore/system/al_Printing.hpp"

namespace al{

void printPlot(float value, uint32_t width, bool spaces, const char * point){
	bool clipped = false;	
	if(value < -1.f){ value=-1.f; clipped=true; }
	else if(value > 1.f){ value=1.f; clipped=true; }
	
	const char * pt = clipped ? "+" : point;
	
	value = (value + 1.f) * 0.5f * (float)(width);
	value += value>=0 ? 0.5 : -0.5;
	uint32_t pos = uint32_t(value);
	uint32_t mid = width >> 1;
	uint32_t i=0;

	if(pos < mid){	// [-1, 0)
		for(; i<pos; ++i) printf(" ");
		printf("%s", pt); ++i;
		for(; i<mid; ++i) printf("-");
		printf("|");
	}
	else{			// (0, 1]
		for(; i<mid; ++i) printf(" ");
		if(pos == mid){ printf("%s", pt); goto end; }
		printf("|"); ++i;
		for(; i<pos; ++i) printf("-");
		printf("%s", pt);
	}
	
	end: 
	if(spaces) for(; i<width; ++i) printf(" ");
}


void err(const char * msg, const char * src, bool exits){
	fprintf(stderr, "%s%serror: %s\n", src, src[0]?" ":"", msg);
	if(exits) exit(EXIT_FAILURE);
}

void warn(const char * msg, const char * src){
	fprintf(stderr, "%s%swarning: %s\n", src, src[0]?" ":"", msg);
}

void warnOnce(const char * msg){
	static std::map<const char *, int> M;
	if(0==M.count(msg)){
		M[msg]=1;
		fprintf(stderr, "warning: %s\n", msg);
	}
}


void _warn(const char * fileName, int lineNumber, const char * fmt, ...){
	fprintf(stderr, "%s:%d: ", fileName, lineNumber);
	va_list arg;
	va_start(arg, fmt);
	vfprintf(stderr, fmt, arg);
	va_end(arg);
}


void _warnOnce(const char * fileName, int lineNumber, const char * fmt, ...){
	static std::string msg;
	static std::map<std::string, int> M;
	static char buf[256];

	va_list arg;
	va_start(arg, fmt);
	vsprintf(buf, fmt, arg);
	va_end(arg);

	msg = buf;

	if(0==M.count(msg)){
		M[msg]=1;
		fprintf(stderr, "%s:%d: %s", fileName, lineNumber, buf);
	}
}

} // ::al::
