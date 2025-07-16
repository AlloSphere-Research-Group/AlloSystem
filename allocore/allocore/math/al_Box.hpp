#ifndef INCLUDE_AL_BOX_HPP
#define INCLUDE_AL_BOX_HPP

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.


	File description:
	Geometric box (hyperrectangle)

	File author(s):
	Lance Putnam, 2025, putnam.lance@gmail.com
*/

#include "allocore/math/al_Vec.hpp"

namespace al {

namespace{
template <class T> T Box__bigVal();
template <> float Box__bigVal<float>(){ return 3.4e38; }
template <> double Box__bigVal<double>(){ return 1.7e308; }
}

/// @addtogroup allocore
/// @{

/// Geometric box shape

/// In geometry, a box or hyperrectangle is an n-dimensional generalization of
/// a rectangle. In one, two and three dimensions it is an interval, rectangle
/// and cuboid, respectively. Here, the box is axis-aligned.
template <int N=3, class T=float>
class Box{
public:

	using vec = Vec<N,T>;
	using value_type = T;


	template <class Array>
	static constexpr unsigned countof(){ return sizeof(Array)/sizeof(typename Array::value_type); }

	template <class Tvec>
	static constexpr int static_assert_vec(){
		static_assert(countof<Tvec>() >= countof<vec>(), "Vector size too small");
		return 0;
	}


	Box(){ reset(); }
	Box(const Box& b){ *this = b; }


	/// Return a copy
	Box dup() const { return *this; }

	Box& operator= (const Box& b){
		return set(b.mMin, b.mMax);
	}

	/// Set from min and max corners
	Box& set(vec min, vec max){
		mMin = min;
		mMax = max;
		return *this;
	}

	/// Set max with min at origin
	Box& set(vec ext){ return set(vec(0), ext); }

	#define AL_BOX_DEF_OP(op)\
		Box& operator op##= (vec v){ mMin op##= v; mMax op##= v; return *this; }\
		Box operator op (vec v) const { return dup() op##= v; }

	AL_BOX_DEF_OP(+) AL_BOX_DEF_OP(-) AL_BOX_DEF_OP(*) AL_BOX_DEF_OP(/)

	#undef AL_BOX_DEF_OP

	/// Get min corner
	const vec& min() const { return mMin; }
	/// Get max corner
	const vec& max() const { return mMax; }
	/// Get extent (width, height, depth)
	vec extent() const { return mMax - mMin; }
	/// Get center of box
	vec center() const { return (mMax+mMin)*T(0.5); }
	/// Get volume of box
	T volume() const { return extent().product(); }

	/// Fit bounds to a set of points
	template <class GetPointAtIndex>
	Box& fit(const GetPointAtIndex& getPointAtIndex, int numPoints, int offset=0){
		reset();
		for(int i=offset; i<offset+numPoints; ++i){
			adjust(getPointAtIndex(i));
		}
		return *this;
	}

	/// Reset the bounds (to be fitted again)
	Box& reset(){ mMin=Box__bigVal<T>(); mMax=-mMin; return *this; }

	/// Adjust box to include point
	template <class Tvec>
	Box& adjust(const Tvec& point){
		static_assert_vec<Tvec>();
		for(unsigned i=0; i<size(); ++i){
			if(point[i] < mMin[i]) mMin[i] = point[i];
			if(point[i] > mMax[i]) mMax[i] = point[i];
		}
		return *this;
	}

	/// Adjust box to include sphere
	template <class Tvec>
	Box& adjustSphere(const Tvec& pos, float rad){
		static_assert_vec<Tvec>();
		for(unsigned i=0; i<size(); ++i){
			auto lo = pos[i] - rad;
			auto hi = pos[i] + rad;
			if(lo < mMin[i]) mMin[i] = lo;
			if(hi > mMax[i]) mMax[i] = hi;
		}
		return *this;
	}

	/// Test whether a point is in/on box
	template <class Tvec>
	bool contains(const Tvec& point) const {
		static_assert_vec<Tvec>();
		for(unsigned i=0; i<size(); ++i){
			if(point[i] < mMin[i] || point[i] > mMax[i])
				return false;
		}
		return true;
	}

	/// Test whether we intersect with another box
	bool intersects(const Box& other) const {
		for(unsigned i=0; i<size(); ++i){
			if(other.mMax[i] < mMin[i] || other.mMin[i] > mMax[i])
				return false;
		}
		return true;
	}

	/// Test whether we intersect with a sphere
	template <class Tvec>
	bool intersectsSphere(const Tvec& center, T radius) const {
		static_assert_vec<Tvec>();
		for(unsigned i=0; i<size(); ++i){
			if((center[i]+radius) < mMin[i] || (center[i]-radius) > mMax[i])
				return false;
		}
		return true;
	}

	/// Add padding amount to box (changes all AA radii by this amount)
	Box& pad(T v){
		mMin -= v;
		mMax += v;
		return *this;
	}

private:
	vec mMin, mMax;
	static constexpr unsigned size(){ return countof<vec>(); }
};

/// @} // end allocore group

} // al::

#endif
