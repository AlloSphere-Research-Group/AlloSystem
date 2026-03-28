#ifndef INC_AL_SPEAKER_HPP
#define INC_AL_SPEAKER_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Abstraction of a loudspeaker used for sound spatialization algorithms

	Author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
*/


#include <vector>
#include "allocore/math/al_Vec.hpp"

namespace al{

/// \addtogroup allocore
/// @{

/// Spatial definition of a speaker in a listening space
class Speaker {
public:

	unsigned int deviceChannel;	///< Index in the output device channels array
	float gain;					///< Gain of speaker
	float azimuth;				///< Angle from forward to left vector
	float elevation;			///< Angle from forward-right plane to up vector
	float radius;				///< Distance from center of listening space

	/// @param[in] deviceChan		audio device output channel
	/// @param[in] az				azimuth of speaker
	/// @param[in] el				elevation of speaker
	/// @param[in] radius			radius of speaker
	/// @param[in] gain				gain of speaker
	Speaker(int deviceChan=0, float az=0.f, float el=0.f, float radius=1.f, float gain=1.f);

	/// Set position from Cartesian coordinate
	Speaker& pos(Vec3f p);

	/// Get position as Cartesian coordinate
	Vec3f pos() const;
};


/// A set of speakers
using Speakers = std::vector<Speaker>;


/// Base class for a configuration of multiple speakers
class SpeakerLayout{
public:

	/// Get number of speakers
	int numSpeakers() const;

	/// Get speaker array
	Speakers& speakers(){ return mSpeakers; }
	const Speakers& speakers() const { return mSpeakers; }

	/// Add speaker
	SpeakerLayout& addSpeaker(const Speaker& s);

protected:
	Speakers mSpeakers;
};


/// Generic layout of N speakers spaced equidistantly in a ring
template <int N>
class SpeakerRingLayout : public SpeakerLayout{
public:
	/// @param[in] deviceChannelStart	starting index of device channel
	/// @param[in] phase				starting phase of first speaker, in degrees
	/// @param[in] radius				radius of all speakers
	/// @param[in] gain					gain of all speakers
	SpeakerRingLayout(int deviceChannelStart=0, float phase=0.f, float radius=1.f, float gain=1.f){
		mSpeakers.reserve(N);
		for(int i=0; i<N; ++i)
            addSpeaker({i+deviceChannelStart, 360.f/N*i + phase, 0, radius, gain});
	}
};


/// Headset speaker layout
class HeadsetSpeakerLayout : public SpeakerRingLayout<2>{
public:
	HeadsetSpeakerLayout(int deviceChannelStart=0, float radius=1.f, float gain=1.f);
};

/// Stereo speaker layout
class StereoSpeakerLayout : public SpeakerLayout{
public:
	StereoSpeakerLayout(int deviceChannelStart=0, float angle=30.f, float distance=1.f, float gain=1.f);
private:
	Speaker mLeft;
	Speaker mRight;
};


/// Octophonic ring speaker layout
using OctalSpeakerLayout = SpeakerRingLayout<8>;


/// Generic layout of 8 speakers arranged in a cube with listener in the middle
class CubeLayout : public SpeakerLayout {
public:
	CubeLayout(int deviceChannelStart=0);
};

/// @} // end allocore group

} // al::
#endif
