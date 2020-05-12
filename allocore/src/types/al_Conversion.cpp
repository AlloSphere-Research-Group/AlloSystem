#include <cctype>		// tolower, toupper
#include <cstdio>		// vsnprintf
#include <cstdarg>		// vsnprintf
#include <cstring>		// strlen
#include "allocore/types/al_Conversion.hpp"

namespace al{

/// Returns string with all characters converted to lowercase
std::string toLower(const std::string& s){
	auto r = s;
	for(auto& c : r) c = std::tolower(c);
	return r;
}

/// Returns string with all characters converted to uppercase
std::string toUpper(const std::string& s){
	auto r = s;
	for(auto& c : r) c = std::toupper(c);
	return r;
}

char base10To36(int v){
	static const char * c = "0123456789abcdefghijklmnopqrstuvwxyz";
	if(v>=0 && v<=35) return c[v];
	return '0';
}

int base36To10(char v){
	v = std::tolower(v);
	if(v>='0' && v<='9') return v - '0';
	if(v>='a' && v<='z') return v - 'a' + 10;
	return 0;	// non-alphanumeric
}

uint32_t bitsToUInt(const char * string){
	uint32_t v=0; int n = std::strlen(string);
	for(int i=0; i<n; ++i) if(string[i] == '1') v |= 1<<(n-1-i);
	return v;
}

std::string toString(const char * fmt, ...){
	char buf[64];
	va_list args;
	va_start(args, fmt);
	std::vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);
	return std::string(buf);
}

/* // Would prefer this, but snprintf has flakey support across platforms
template <class... Args>
std::string toString(const char * fmt, Args... args){
	char buf[64];
	AL_SNPRINTF(buf, sizeof(buf), fmt, args...);
	return std::string(buf);
}*/

} // al::
