#ifndef INC_AL_DBAP_HPP
#define INC_AL_DBAP_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Distance-based amplitude panner (DBAP)

	Author(s):
	Ryan McGee, 2012, ryanmichaelmcgee@gmail.com
*/

#include "allocore/sound/al_AudioScene.hpp"

namespace al{

#define DBAP_MAX_NUM_SPEAKERS 192

/// Distance-based amplitude panner
///
/// @ingroup allocore
class Dbap : public Spatializer{
public:

	/// @param[in] sl		A speaker layout
	/// @param[in] focus	Amplitude focus to nearby speakers
	Dbap(const SpeakerLayout &sl, float focus = 1.f);

	void compile(Listener& listener) override;
	void renderBuffer(AudioIOData& io, const Pose& reldir, const float *samples, int numFrames) override;
	void renderSample(AudioIOData& io, const Pose& reldir, float sample, int frameIndex) override;

	/// Set exponent determining the amplitude focus to nearby speakers.

	///focus is (0, inf) with usable range typically [0.2, 5]. Default is 1.
	///A denser speaker layout my benefit from a high focus > 1, and a sparse layout may benefit from focus < 1
	void setFocus(float focus) { mFocus = focus; }

	void print() override;

private:
	Listener * mListener;
	Vec3f mSpeakerVecs[DBAP_MAX_NUM_SPEAKERS];
	int mDeviceChannels[DBAP_MAX_NUM_SPEAKERS];
	int mNumSpeakers;
	float mFocus;
};

} // al::
#endif
