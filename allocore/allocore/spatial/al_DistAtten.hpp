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
	Distance attentuation functions for calculating sound pressure levels,
	fog density, etc. within a space.

	File author(s):
	Lance Putnam, 2015, putnam.lance@gmail.com
*/

namespace al{

/// Distance to attenuation laws
enum AttenuationLaw{
	ATTEN_NONE=0,			/**< No attenuation over distance */
	ATTEN_LINEAR,			/**< Linear attenuation over distance */
	ATTEN_INVERSE,			/**< Attenuation is one over distance */
	ATTEN_INVERSE_SQUARE	/**< Attenuation is one over distance squared */
};


/// Maps a distance into an attenuation factor
///
/// @ingroup allocore
template <class T = float>
class DistAtten{
public:

	/// @param[in] nearClip		Distance below which amplitude is clamped to 1
	/// @param[in] farClip		Distance at which amplitude reaches its minimum
	/// @param[in] law			Distance to attenuation factor law
	/// @param[in] farBias		Bias at far clip distance (linear model only)
	DistAtten(
		T nearClip = T(0.1), T farClip = T(20),
		AttenuationLaw law = ATTEN_INVERSE, T farBias = T(0)
	)
	:	mNear(nearClip), mFar(farClip), mFarBias(farBias), mLaw(law)
	{	setScale(); }


	/// Get near clip distance
	T nearClip() const { return mNear; }

	/// Get far clip distance
	T farClip() const { return mFar; }

	/// Get far bias
	T farBias() const { return mFarBias; }

	/// Get attenuation law
	AttenuationLaw law() const { return mLaw; }


	/// Set attenuation law
	DistAtten& law(AttenuationLaw v){ mLaw=v; return setScale(); }

	/// Set near clip distance
	DistAtten& nearClip(T v){ mNear=v; return setScale(); }

	/// Set far clip distance
	DistAtten& farClip(T v){ mFar=v; return setScale(); }

	/// Set bias at far clip distance (linear model only)
	DistAtten& farBias(T v){ mFarBias=v; return setScale(); }


	/// Get attenuation factor for a given distance
	T attenuation(T dist) const {

		// No attenuation if below near distance
		if(dist <= mNear) return T(1);

		switch(mLaw){
		case ATTEN_LINEAR:
			return dist < mFar ? T(1) - mScale*(dist - mNear) : mFarBias;

		case ATTEN_INVERSE:
			return mNear / (mNear + mScale*(dist - mNear));

		case ATTEN_INVERSE_SQUARE:{
			T nearSqr = mNear*mNear;
			return nearSqr / (nearSqr + mScale*(dist*dist - nearSqr));
		}

		default:
			return T(1);
		}
	}

protected:
	T mNear, mFar;		// clipping planes
	T mFarBias;			// bias on far clip (linear model only)
	T mScale;
	AttenuationLaw mLaw;

	DistAtten& setScale(){
		switch(mLaw){
		case ATTEN_LINEAR:
			mScale = (T(1) - mFarBias)/(mFar - mNear);
			break;

		// Note: For inverse laws, the attenuation factor at far clip is
		// hard-coded. For INVERSE_SQUARE, it is the square of the value for
		// INVERSE. This ensures a correct inverse power relationship.
		case ATTEN_INVERSE:
			mScale = (mNear/T(0.25) - mNear) / (mFar - mNear);
			break;
		case ATTEN_INVERSE_SQUARE:{
			T nearSqr = mNear*mNear;
			mScale = (nearSqr/T(0.25*0.25) - nearSqr) / (mFar*mFar - nearSqr);
			}
			break;
		default:
			mScale = 1;
		}
		return *this;
	}
};

} // al::

#endif
