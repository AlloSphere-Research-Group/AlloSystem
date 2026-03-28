#ifndef INC_AL_DIST_ATTEN_HPP
#define INC_AL_DIST_ATTEN_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Distance attentuation functions for calculating sound pressure levels,
	fog density, etc. within a space.

	Author(s):
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
