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

std::string toBase64(const void * data, int numBytes){
	static const char * lut =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

	const auto * bytes = (const unsigned char *)data;
	std::string s;

	for(unsigned i=0; i<numBytes*8; i+=6){ // iterate over sextets
		auto i8 = i/8;
		auto byte1 = bytes[i8];
		auto byte2 = i8 <= (numBytes-2) ? bytes[i8+1] : 0;
		unsigned lutIdx;

		switch((i/6)%4){
		case 0: lutIdx = byte1>>2; break;
		case 1: lutIdx = ((byte1<<4)&63) | (byte2>>4); break;
		case 2: lutIdx = ((byte1<<2)&63) | (byte2>>6); break;
		case 3: lutIdx = byte1&63; break;
		}

		s += lut[lutIdx];
	}

	return s;
}

void fromBase64(void * dst, const std::string& src, unsigned numDstBytes){

    unsigned numSrcBits = src.size()*6;
    unsigned numSrcBytes = (numSrcBits/8) + ((numSrcBits%8)!=0);

	if(numSrcBytes > numDstBytes) numSrcBytes = numDstBytes;
	auto * data = (unsigned char *)dst;

	auto toByte = [](char c){
		if('A'<=c && c<='Z') return c-'A';
		if('a'<=c && c<='z') return c-'a'+26;
		if('0'<=c && c<='9') return c-'0'+52;
		if('-'==c) return 62;
		else return 63;
	};

	// sextet    0     1     2     3     4     5
	//        |12345|12345|12345|12345|12345|12345
	//        |1234567|1234567|1234567|1234567
	// octect     0       1       2       3

	for(unsigned i=0; i<numSrcBytes; ++i){
		auto i6 = i*8/6; // sextet index
		uint16_t b2 = uint16_t(toByte(src[i6]))<<6 | toByte(src[i6+1]);
		data[i] = b2 >> ((2-i%3)*2);
	}
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
