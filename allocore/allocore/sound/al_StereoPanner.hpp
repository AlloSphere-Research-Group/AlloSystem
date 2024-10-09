#ifndef INCLUDE_AL_STEREO_PANNER_HPP
#define INCLUDE_AL_STEREO_PANNER_HPP

#include <vector>
#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Constants.hpp"

namespace al{

class AudioIOData;
class Pose;

///
/// \brief The StereoPanner class
///
/// @ingroup allocore
class StereoPanner : public Spatializer{
public:
	StereoPanner(const SpeakerLayout& sl);

	void compile(Listener& listener) override;

	/// Per Sample Processing
	void renderSample(AudioIOData& io, const Pose& listeningPose, float sample, int frameIndex) override;

	/// Per Buffer Processing
    void renderBuffer(AudioIOData& io, const Pose& listeningPose, const float *samples, int numFrames) override;

private:
	Listener* mListener;
	int mNumSpeakers;
	std::vector<float> mBuffer;
};

} // al::

#endif
