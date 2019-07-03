#ifndef INCLUDE_AL_INTERVAL_HPP
#define INCLUDE_AL_INTERVAL_HPP

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
	A closed interval

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


namespace al {

/// A closed interval [min, max]

/// An interval is a connected region of the real line. Geometrically, it
/// describes a 0-sphere. Order is strongly enforced so that the endpoints will
/// always satisfy min <= max.
///
/// @ingroup allocore
template <class T>
class Interval{
public:

	Interval()
	:	mMin(0), mMax(1){}

	/// @param[in] min	minimum endpoint
	/// @param[in] max	maximum endpoint
	Interval(const T& min, const T& max)
	{ endpoints(min,max); }

	T center() const { return (max()+min())/T(2); }	///< Returns center point

	/// Returns true if value is in interval
	bool contains(const T& v) const { return v>=min() && v<=max(); }

	bool degenerate() const { return min()==max(); }///< Returns true if diameter is zero
	T diameter() const { return max()-min(); }		///< Returns absolute difference of endpoints
	T size() const { return diameter(); }			///< Returns absolute difference of endpoints
	const T& max() const { return mMax; }			///< Get maximum endpoint
	const T& min() const { return mMin; }			///< Get minimum endpoint
	bool proper() const { return min()!=max(); }	///< Returns true if diameter is non-zero
	T radius() const { return diameter()/T(2); }	///< Returns one-half the diameter

	/// Linearly map point in interval to point in the unit interval [0,1]
	T toUnit(const T& v) const { return (v-min())/diameter(); }

	/// Linearly map point in the unit interval [0,1] to point in interval
	T fromUnit(const T& u) const { return u*diameter() + min(); }

	template <class U>
	bool operator == (const Interval<U>& v){ return min()==v.min() && max()==v.max(); }

	template <class U>
	bool operator != (const Interval<U>& v){ return !(*this == v); }

	template <class U>
	Interval& operator +=(const Interval<U>& v){ endpoints(min()+v.min(), max()+v.max()); return *this; }

	template <class U>
	Interval& operator -=(const Interval<U>& v){ endpoints(min()-v.max(), max()-v.min()); return *this; }

	template <class U>
	Interval& operator *=(const Interval<U>& v){
		T a=min()*v.min(), b=min()*v.max(), c=max()*v.min(), d=max()*v.max();
		mMin = min(min(a,b),min(c,d));
		mMax = max(max(a,b),max(c,d));
		return *this;
	}

	template <class U>
	Interval& operator /=(const Interval<U>& v){
		T a=min()/v.min(), b=min()/v.max(), c=max()/v.min(), d=max()/v.max();
		mMin = min(min(a,b),min(c,d));
		mMax = max(max(a,b),max(c,d));
		return *this;
	}

	/// Set center point preserving diameter
	Interval& center(const T& v){ return centerDiameter(v, diameter()); }

	/// Set diameter (width) preserving center
	Interval& diameter(const T& v){ return centerDiameter(center(), v); }

	/// Set center and diameter
	Interval& centerDiameter(const T& c, const T& d){
		mMin = c - d*T(0.5);
		mMax = mMin + d;
		return *this;
	}

	/// Set the endpoints
	Interval& endpoints(const T& min, const T& max){
		mMax=max; mMin=min;
		if(mMin > mMax){ T t=mMin; mMin=mMax; mMax=t; }
		return *this;
	}

	/// Translate interval by fixed amount
	Interval& translate(const T& v){ mMin+=v; mMax+=v; return *this; }

	/// Set maximum endpoint
	Interval& max(const T& v){ return endpoints(min(), v); }

	/// Set minimum endpoint
	Interval& min(const T& v){ return endpoints(v, max()); }

private:
	T mMin, mMax;

	const T& min(const T& a, const T& b){ return a<b?a:b; }
	const T& max(const T& a, const T& b){ return a>b?a:b; }
};

} // ::al::

#endif

