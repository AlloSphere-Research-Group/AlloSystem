#ifndef INCLUDE_AL_SOUND_SPEAKER_HPP
#define INCLUDE_AL_SOUND_SPEAKER_HPP

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
	Abstraction of a loudspeaker used for sound spatialization algorithms

	File author(s):
	Lance Putnam, 2006, putnam.lance@gmail.com
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
