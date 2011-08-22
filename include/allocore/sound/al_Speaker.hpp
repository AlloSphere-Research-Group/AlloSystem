#ifndef INCLUDE_AL_SOUND_SPEAKER_HPP
#define INCLUDE_AL_SOUND_SPEAKER_HPP

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

#include <cmath>
#include <vector>

namespace al{

/// Spatial definition of a speaker in a listening space
struct Speaker {

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
	Speaker(int deviceChan=0, float az=0.f, float el=0.f, float radius=1.f, float gain=1.f)
	:	deviceChannel(deviceChan), gain(gain), azimuth(az), elevation(el), radius(radius)
	{}

	template <class T>
	Speaker& posCart(T * xyz){
		using namespace std;
		float elr = toRad(elevation);
		float azr = toRad(azimuth);
		float cosel = cos(elr);
		xyz[0] = sin(azr) * cosel * radius;
		xyz[1] = cos(azr) * cosel * radius;
		xyz[2] =         sin(elr) * radius;
		return *this;
	}
	
	static double toRad(double d){ return d*2.*M_PI/180.; }
};



/// Base class for a configuration of multiple speakers
class SpeakerLayout{
public:
	typedef std::vector<Speaker> Speakers;

	SpeakerLayout(){}

	int numSpeakers() const { return speakers().size(); }

	const Speakers& speakers() const { return mSpeakers; }
	Speakers& speakers(){ return mSpeakers; }

//	Speaker addSpeaker(float azimuth, float elevation, float distance, int deviceChannel){
//		Speaker s;
//		s.azimuth = azimuth;
//		s.elevation = elevation;
//		s.distance = distance;
//		s.deviceChannel = deviceChannel;
//		addSpeaker(s);
//		return s;
//	}

	SpeakerLayout& addSpeaker(const Speaker& spkr){
		mSpeakers.push_back(spkr);
		return *this;
	}

	

protected:
	friend class Listener;
	Speakers mSpeakers;
};



/// Generic layout of N speakers spaced equidistantly in a ring
template <int N>
struct SpeakerRingLayout : public SpeakerLayout{

	/// @param[in] deviceChannelStart	starting index of device channel
	/// @param[in] phase				starting phase of first speaker, in degrees
	/// @param[in] radius				radius of all speakers
	/// @param[in] gain					gain of all speakers
	SpeakerRingLayout(int deviceChannelStart=0, float phase=0.f, float radius=1.f, float gain=1.f)
	{
		mSpeakers.reserve(N);
		for(int i=0; i<N; ++i)
			addSpeaker(Speaker(i+deviceChannelStart, 360./N*i + phase, 0, radius, gain));
	}
};

/// Headset speaker layout
struct HeadsetSpeakerLayout : public SpeakerRingLayout<2>{
	HeadsetSpeakerLayout(int deviceChannelStart=0, float radius=1.f, float gain=1.f)
	:	SpeakerRingLayout<2>(deviceChannelStart, 90, radius, gain)
	{}
};


/// Octophonic ring speaker layout
typedef SpeakerRingLayout<8> OctalSpeakerLayout;


} // al::
#endif
