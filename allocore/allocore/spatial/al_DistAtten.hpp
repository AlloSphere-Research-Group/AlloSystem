#ifndef INCLUDE_AL_DIST_ATTEN_HPP
#define INCLUDE_AL_DIST_ATTEN_HPP

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
	Distance attentuation functions useful for calculating sound pressure
	levels, fog density, etc. within a space.

	File author(s):
	Lance Putnam, 2011, putnam.lance@gmail.com
*/

#include <math.h>

namespace al{


/// Maps a distance into an attenuation factor
template <class T = float>
class DistAtten{
public:

	enum Func{
		LINEAR=0,
		INVERSE,
		INVERSE_POWER
	};

	DistAtten(const T& nearClip = T(0.1), const T& farClip = T(20), const T& rollOff = T(1))
	:	mNear(nearClip), mFar(farClip), mRollOff(rollOff)
	{	setScale(); }

	T near() const { return mNear; }
	T far() const { return mFar; }
	T rollOff() const { return mRollOff; }
	T scale() const { return mScale; }

	DistAtten& near(const T& v){ mNear=v; return setScale(); }
	DistAtten& far(const T& v){ mFar=v; return setScale(); }
	DistAtten& rollOff(const T& v){ mRollOff=v; return *this; }

	/// Compile-time generic function mapping
	template<Func func>
	T map(const T& dist){ return (*this)(func, dist); }

	/// Run-time variable function mapping
	T map(Func func, const T& dist){
		switch(func){
		case LINEAR:		return linear(dist);
		case INVERSE:		return inverse(dist);
		case INVERSE_POWER:	return inversePower(dist);
		default:			return none(dist);
		}
	}
	

	/// Bias an attentuation factor at zero; [0, 1] -> [bias, 1]
	T bias(const T& amp, const T& bias) const { return bias + amp - amp*bias; }


	/// Map distance into attentuation factor using an inverse power law
	
	/// The rolloff controls the degree of the inverse power. When rolloff=1,
	/// it is identical to an inverse law. When the rolloff=2, it is an
	/// inverse square law, etc.
	T inversePower(const T& dist) const {
		return pow(dist / near(), -rollOff());
	}

	/// Map distance into attentuation factor using inverse law
	
	/// This is the correct law for computing the sound pressure level from a
	/// point source in 3D (sound intensity is inverse square).
	T inverse(const T& dist) const {
		if(dist <= near()) return T(1);
		return near() / (near() + rollOff() * (dist - near()));
	}
	
	/// Map distance into attentuation factor using linear map, [0, dist] -> [0, 1]
	T linear(const T& dist) const {
		if(dist <= near())		return T(1);
		else if(dist >= far())	return T(0);
		else					return (far() - dist) * scale();

		// OpenAL version: rolloff slides the far clip
		//else					return T(1) - (dist - near()) * scale() * rolloff();
	}

	T none(const T& dist) const { return 1; }

	T sigmoid(const T& dist) const {
		return 1. - tanh(M_PI * rollOff() * dist*dist);
	}


protected:
	T mNear, mFar;		// clipping planes
	T mScale;			// always 1/(far - near)
	T mRollOff;
	DistAtten& setScale(){ mScale = 1./(far() - near()); return *this; }
};


/*
Distance laws:

Phenomenon					Law
sound pressure	(N/m^2)		inverse
sound intensity (W/m^2)		inverse squared
*/

/*
TODO:
From SoundSource::attentuation():

	// different possible policies for amplitude attenuation
	// amplitude curve (max/cosm):
	//curve = (1.-dN)*(1.-dN);	
	// alternative curve (hydrogen bond):
	//curve = ((d + C) / (d*d + d + C))^2;	// e.g. C=2
	// alternative curve (skewed sigmoid):
	double curve = 1-tanh(M_PI * dN*dN);

*/

} // al::

#endif
