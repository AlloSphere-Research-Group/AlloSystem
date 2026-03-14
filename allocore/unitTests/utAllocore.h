#ifndef INCLUDE_UT_ALLOCORE_H
#define INCLUDE_UT_ALLOCORE_H

// Force assertions
#ifdef NDEBUG
#undef NDEBUG
#endif

#include <cassert>
#include <cmath>

#ifdef AL_UT_NOPRINT	/* disable printing to stdout */
	#define UT_PRINTF	//
#else
	#define UT_PRINTF	printf
#endif

/// Returns whether two values are almost equal (within some epsilon)
template <class T>
inline bool eq(T x, T y, T eps=0.000001){
	return abs(x-y) < eps;
}

template <class T>
inline bool eq(const T* x, const T* y, int n, T eps=0.0000001){
	for(int i=0; i<n; ++i){
		if(!eq(x[i], y[i], eps)) return false;
	}
	return true;
}

inline bool eq(int x, int y){ return x==y; }

int utAudioScene();
int utIOAudioIO();
int utIOSocket();
int utIOWindowGL();
int utMath();
int utMathSpherical();
int utGraphicsDraw();
int utGraphicsMesh();
int utProtocolOSC();
int utSpatial();
int utSystem();
int utTypes();
int utTypesConversion();
int utThread();
int utFile();
int utAsset();
int utAmbisonics();

#endif
