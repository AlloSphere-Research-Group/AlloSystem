#ifndef INCLUDE_AL_DIST_ATTEN_HPP
#define INCLUDE_AL_DIST_ATTEN_HPP

/*
 *  AlloSphere Research Group / Media Arts & Technology, UCSB, 2009
 */

/*
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
*/

#include <math.h>

namespace al{


/// Maps a distance into an attenuation factor
template <class T>
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
