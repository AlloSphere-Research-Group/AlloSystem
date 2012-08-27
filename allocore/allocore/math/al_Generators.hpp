#ifndef INCLUDE_AL_MATH_GENERATORS_HPP
#define INCLUDE_AL_MATH_GENERATORS_HPP

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
	Generator function objects

	A generator is a lightweight object that generates a sequence of elements.
	Generators have a standard interface specified by the Val class. 
	A const qualified generator only means that its generating function 
	parameters are held constant; its current value can change.
	
	When a generator is iterated (through its nullary function operator) it
	will compute its next value, store the result in its value member variable,
	and return its previously stored value. The reason the previous value is 
	returned is so that its value can be set directly and on its next iteration
	it will return that value.

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/

namespace al {

/// Classes for generating sequences of numbers
namespace gen{

// This is needed since templates are not always smart about inheriting super members.
#define INHERIT\
	using Val<T>::val; using Val<T>::operator=;\
	T   operator[](int i) const { return (*this)(); }


/// Single value generator
template <class T>
struct Val{
	mutable T val;										///< Value
	// Since this is a generator, we will allow its value to be modified if 
	// it's a const.

	Val(): val(T(0)){}									///< Constructor
	Val(const T& v): val(v){}							///< Constructor
	Val& operator= (const T& v){val=v; return *this;}	///< Set value
	T operator()() const { return val; }				///< Generate next value
};

/// Recursive add generator
template <class T>
struct RAdd: public Val<T>{ INHERIT;
	T add;												///< Addition amount

	RAdd(const T& add=T(1), const T& ival=T(0))
	: Val<T>(ival), add(add){}							///< Constructor
	T operator()() const {T r=val; val+=add; return r;}	///< Generate next value
};

/// Recursive multiply generator
template <class T>
struct RMul: public Val<T>{ INHERIT;
	T mul;												///< Multiplication amount

	RMul(const T& mul=T(1), const T& ival=T(1))
	:	Val<T>(ival), mul(mul){}						///< Constructor
	T operator()() const {T r=val; val*=mul; return r;}	///< Generate next value
};

/// Recursive multiply-add generator
template <class T>
struct RMulAdd: public Val<T>{ INHERIT;
	T mul;												///< Multiplication amount
	T add;												///< Addition amount

	RMulAdd(const T& mul=T(1), const T& add=T(0), const T& ival=T(0))
	:	Val<T>(ival), mul(mul), add(add){}				///< Constructor
	T operator()() const {T r=val; val=r*mul+add; return r; }	///< Generate next value
};


/// Sinusoidal generator based on recursive formula x0 = c x1 - x2
template <class T>
struct RSin : public Val<T>{ INHERIT;

	/// Constructor
	RSin(const T& frq=T(0), const T& phs=T(0), const T& amp=T(1))
	:	val2(0), mul(0){ set(frq,phs,amp); }

	/// Generate next value.
	T operator()() const {
		T v0 = mul * val - val2;
		val2 = val;
		return val = v0;
	}
	
	T freq() const { return acos(mul*0.5) * M_1_2PI; }
	
	/// Set parameters from unit freq, phase, and amplitude.
	RSin& set(const T& frq, const T& phs, const T& amp=T(1)){
		T f=frq*M_2PI, p=phs*M_2PI;
		mul  = (T)2 * (T)cos(f);
		val2 = (T)sin(p - f * T(2))*amp;
		val  = (T)sin(p - f       )*amp;
		return *this;
	}

	mutable T val2;
	T mul;			///< Multiplication factor
};


#undef INHERIT

} // ::al::gen::
} // ::al::

#endif
