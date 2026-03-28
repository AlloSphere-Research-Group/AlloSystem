#ifndef INC_AL_PRINTING_H
#define INC_AL_PRINTING_H

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Various routines for printing text to files, standard output, etc.

	Author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/

#include <cstdio>
#include "allocore/system/al_Config.h"

namespace al{

/// @addtogroup allocore
/// @{

/// Returns an ASCII character with intensity most closely matching a value in [0,1)
char intensityToASCII(float v);

/// Print an array of numbers

/// @param[in] arr		input array
/// @param[in] size		size of input array
/// @param[in] append	an extra string to append at end
template <typename T>
void print(const T * arr, int size, const char * append="");

/// Print an array of numbers with new line

/// @param[in] arr		input array
/// @param[in] size		size of input array
template <typename T>
void println(const T * arr, int size){ print(arr, size, "\n"); }

/// Print value

/// @param[in] v		value to print
/// @param[in] append	an extra string to append at end
template <typename T>
void print(const T& v, const char * append=""){ print(&v, 1, append); }

/// Print value with new line
template <typename T>
void println(const T& v){ print(v, "\n"); }

/// Prints 2D array of intensity values

/// @param[in] arr		flat 2D array of values in [0, 1) where x moves fastest
/// @param[in] nx		number of elements along x
/// @param[in] ny		number of elements along y
/// @param[in] fp		file to write output to
template<class T> void print2D(const T* arr, int nx, int ny, FILE * fp=stdout);

/// Print signed unit value on a horizontal plot.

/// @param[in]	value	Normalized value to plot
/// @param[in]	width	Character width of plot excluding center point
/// @param[in]	spaces	Print extra filling spaces to the right
/// @param[in]	point	The print character for points
void printPlot(float value, int width=50, bool spaces=true, const char * point="o");

/// Prints error message to stderr and optionally calls exit()
void err(const char * msg, const char * src="", bool exits=true);

/// Prints warning message to stderr
#define AL_WARN(fmt, ...) ::al::_warn(__FILE__, __LINE__, fmt "\n", ##__VA_ARGS__)

/// Prints warning message to stderr once during program lifecycle
#define AL_WARN_ONCE(fmt, ...) ::al::_warnOnce(__FILE__, __LINE__, fmt "\n", ##__VA_ARGS__)

void _warn(const char * fileName, int lineNumber, const char * fmt, ...);
void _warnOnce(const char * fileName, int lineNumber, const char * fmt, ...);

/// Enable/disable echoing stdin
void stdinEcho(bool enable);

/// @} // end allocore group

// Implementation --------------------------------------------------------------

inline char intensityToASCII(float v){
	static const char map[] =
	" .,;-~_+<>i!lI?/|)(1}{][rcvunxzjftLCJUYXZO0Qoahkbdpqwm*WMB8&%$#@";
	//"$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
//	 123456789.123456789.123456789.123456789.123456789.123456789.1234
	static const int N = sizeof(map)-1;
	v = v<0 ? 0 : (v>0.9999999f ? 0.9999999f : v);
	return map[int(N*v)];
}


#define DEF_PRINT(T, code)\
template<>\
inline void print<T>(const T * arr, int size, const char * append){\
	for(int i=0; i<size; ++i){ printf(code " ", arr[i]); } if(append[0]) printf("%s", append);\
}

DEF_PRINT(float, "%g")
DEF_PRINT(double, "%g")
DEF_PRINT(char, "%d")
DEF_PRINT(unsigned char, "%u")
DEF_PRINT(short, "%d")
DEF_PRINT(unsigned short, "%u")
DEF_PRINT(int, "%d")
DEF_PRINT(unsigned int, "%u")
DEF_PRINT(long, "%ld")
DEF_PRINT(unsigned long, "%lu")
DEF_PRINT(long long, "%" AL_PRINTF_LL "d")
DEF_PRINT(unsigned long long, "%" AL_PRINTF_LL "u")

#undef DEF_PRINT

template<class T> void print2D(const T* pix, int nx, int ny, FILE * fp){
	for(int j=0; j<nx; ++j){
	for(int i=0; i<ny; ++i){
		float v = pix[j*nx + i];
		fprintf(fp, "%c ", intensityToASCII(v));
	} printf("\n"); }
}

} // al::
#endif
