#ifndef INCLUDE_AL_AMBISONICS_HPP
#define INCLUDE_AL_AMBISONICS_HPP

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
	Higher order Ambisonics encoding/decoding

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com

	Based on prior work also contributed to by:
	Jorge Castellanos
	Florian Hollerweger
*/

#include <stdio.h>
#include "allocore/sound/al_AudioScene.hpp"

//#define MAX_ORDER 3


/*
	TODO: near-field compensation
	acknowledges that the speakers are at a finite distance from the listener.  This a simple Minimum Phase HP filter (on directional signals, not the W component) to deal with "proximity effect" because the speakers are 'near' the listener.

	f = c / (2 * PI * d) = 54.6 / d    Hz	c : speed of sound  343 m/s
											d : speaker distance      m

	Should be IIR, because it mostly affects LF.

	If we pass speaker distance, then we might also want to attenuate/delay speakers for irregular layouts.

	@see "Near Field filters for Higher Order Ambisonics" Fons ADRIAENSEN
	http://kokkinizita.linuxaudio.org/papers/index.html
*/


/*!
	A note on coordinate conventions

	The cartesian coordinate system used for Ambisonics is:
		+x is forward
		+y is left
		+z is up

	The polar coordinate system is as follows:
		Azimuth is the angle between the xz-plane and the source. From the listener's perspective, a positive azimuth is leftward (towards +y) and negative is rightwards (towards -y).
		Elevation is the angle between the xy-plane and the source. From the listener's perspective, a positive elevation is upward (towards +z) and negative is downward (towards -z).

	The cartesian coordinate system used in Allocore's OpenGL is:
		+x is right
		+y is up
		+z is backward

	The correct OpenGL to Ambisonics conversion is thus:
		 ambi_x = -gl_z;
		 ambi_y = -gl_x;
		 ambi_z =  gl_y;
*/


namespace al{

/// Ambisonic base class
///
/// @ingroup allocore
class AmbiBase{
public:

	/// @param[in] dim		number of spatial dimensions (2 or 3)
	/// @param[in] order	highest spherical harmonic order
	AmbiBase(int dim, int order);

	virtual ~AmbiBase();

	/// Get number dimensions
	int dim() const { return mDim; }

	/// Get order
	int order() const { return mOrder; }

	/// Get Ambisonic channel weights
	const float * weights() const { return mWeights; }

	/// Returns total number of Ambisonic domain (B-format) channels
	int channels() const { return mChannels; }

	/// Set the order
	void order(int order);

	/// Called whenever the number of Ambisonic channels changes
	virtual void onChannelsChange(){}


	static int channelsToUniformOrder(int channels);

	/// Compute spherical harmonic weights based on azimuth and elevation
	/// azimuth is anti-clockwise; both azimuth and elevation are in degrees
	static void encodeWeightsFuMa(float * weights, int dim, int order, float azimuth, float elevation);

	/// Compute spherical harmonic weights based on unit direction vector (in the listener's coordinate frame)
	static void encodeWeightsFuMa(float * ws, int dim, int order, float x, float y, float z);

	/// Brute force 3rd order.  Weights must be of size 16.
	static void encodeWeightsFuMa16(float * weights, float azimuth, float elevation);
	/// (x,y,z unit vector in the listener's coordinate frame)
	static void encodeWeightsFuMa16(float * ws, float x, float y, float z);

	static int orderToChannels(int dim, int order);
	static int orderToChannelsH(int orderH);
	static int orderToChannelsV(int orderV);

	static int channelsToOrder(int channels);
	static int channelsToDimensions(int channels);

protected:
	int mDim;			// dimensions - 2d or 3d
	int mOrder;			// order - 0th, 1st, 2nd, or 3rd
	int mChannels;		// cached for efficiency
	float * mWeights;	// weights for each ambi channel

	template<typename T>
	static void resize(T *& a, int n);
};



/// Higher Order Ambisonic Decoding class
///
/// @ingroup allocore
class AmbiDecode : public AmbiBase{
public:

	/// @param[in] dim			number of spatial dimensions (2 or 3)
	/// @param[in] order		highest spherical harmonic order
	/// @param[in] numSpeakers	number of speakers
	/// @param[in] flavor		decoding algorithm
	AmbiDecode(int dim, int order, int numSpeakers, int flavor=1);

	virtual ~AmbiDecode();


	/// @param[out] dec				output time domain buffers (non-interleaved)
	/// @param[in ] enc				input Ambisonic domain buffers (non-interleaved)
	/// @param[in ] numDecFrames	number of frames in time domain buffers
	virtual void decode(float * dec, const float * enc, int numDecFrames) const;

	float decodeWeight(int speaker, int channel) const {
		return mWeights[channel] * mDecodeMatrix[speaker * channels() + channel];
	}

	/// Returns decode flavor
	int flavor() const { return mFlavor; }

	/// Returns number of speakers
	int numSpeakers() const { return mNumSpeakers; }

	void print(FILE * fp = stdout, const char * append = "\n") const;


	/// Set decoding algorithm
	void flavor(int type);

	/// Set number of speakers. Positions are zeroed upon resize.
	void numSpeakers(int num);

	void setSpeakerRadians(int index, int deviceChannel, float azimuth, float elevation, float amp=1.f);

	void setSpeaker(int index, int deviceChannel, float azimuth, float elevation=0, float amp=1.f);
	//void zero();					///< Zeroes out internal ambisonic frame.

	void setSpeakers(Speakers *spkrs);

//	float * azimuths();				///< Returns pointer to speaker azimuths.
//	float * elevations();			///< Returns pointer to speaker elevations.
//	float * frame() const;			///< Returns pointer to ambisonic channel frame used by decode(int)

	/// Get speaker
	Speaker& speaker(int num) { return (*mSpeakers)[num]; }

	virtual void onChannelsChange();

protected:
	int mNumSpeakers;
	int mFlavor;				// decode flavor
	float * mDecodeMatrix;		// deccoding matrix for each ambi channel & speaker
								// cols are channels and rows are speakers
	float mWOrder[5];			// weights for each order
    Speakers* mSpeakers;
    //float * mPositions;		// speakers' azimuths + elevations
	//float * mFrame;			// an ambisonic channel frame used for decode(int)

	void updateChanWeights();
	void resizeArrays(int numChannels, int numSpeakers);

	float decode(float * encFrame, int encNumChannels, int speakerNum);	// is this useful?

	static float flavorWeights[4][5][5];
};



/// Higher Order Ambisonic encoding class
///
/// @ingroup allocore
class AmbiEncode : public AmbiBase{
public:

	/// @param[in] dim			number of spatial dimensions (2 or 3)
	/// @param[in] order		highest spherical harmonic order
	AmbiEncode(int dim, int order) : AmbiBase(dim, order) {}

//	/// Encode input sample and set decoder frame.
//	void encode   (const AmbiDecode &dec, float input);
//
//	/// Encode input sample and add to decoder frame.
//	void encodeAdd(const AmbiDecode &dec, float input);

	/// Encode a single time sample

	/// @param[out] ambiChans	Ambisonic domain channels (non-interleaved)
	/// @param[in] numFrames	number of frames in time buffer
	/// @param[in] timeIndex	index at which to encode time sample
	/// @param[in] timeSample	value of time sample
	void encode(float * ambiChans, int numFrames, int timeIndex, float timeSample) const;

	/// Encode buffer with constant position throughout buffer

	/// @param ambiChans	Ambisonic domain channels (non-interleaved)
	/// @param input		time-domain sample buffer to encode
	/// @param numFrames	number of frames to encode
	void encode(float * ambiChans, const float * input, int numFrames);

	/// Encode a buffer of samples

	/// @param[in] ambiChans	Ambisonic domain channels (non-interleaved)
	/// @param[in] dir			unit vector in the listener's coordinate frame)
	/// @param[in] input		time-domain sample buffer to encode
	/// @param[in] numFrames	number of frames to encode
	template <class XYZ>
	void encode(float * ambiChans, const XYZ * dir, const float * input, int numFrames);

	/// Set spherical direction of source to be encoded
	void direction(float az, float el);

	/// Set Cartesian direction of source to be encoded
	/// (x,y,z unit vector in the listener's coordinate frame)
	void direction(float x, float y, float z);
};


/// Ambisonic coder
///
/// @ingroup allocore
class AmbisonicsSpatializer : public Spatializer {
public:

	AmbisonicsSpatializer(SpeakerLayout &sl, int dim, int order, int flavor=1);

	void zeroAmbi();

	float * ambiChans(unsigned channel=0);

	virtual void compile(Listener& l) override;

	virtual void numFrames(int v) override;

	void numSpeakers(int num);

	void setSpeakerLayout(const SpeakerLayout& sl);

	virtual void prepare();

	virtual void renderBuffer(AudioIOData& io,
	                  const Pose& listeningPose,
	                  const float *samples,
	                  const int& numFrames
	                  ) override;

	virtual void renderSample(AudioIOData& io, const Pose& listeningPose,
	                          const float& sample,
	                          const int& frameIndex) override;

	virtual void finalize(AudioIOData& io) override;

private:
	AmbiDecode mDecoder;
	AmbiEncode mEncoder;
	std::vector<float> mAmbiDomainChannels;
	Listener* mListener;
	int mNumFrames;
};




// Implementation ______________________________________________________________


// AmbiBase
inline int AmbiBase::orderToChannels(int dim, int order){
	int chans = orderToChannelsH(order);
	return dim == 2 ? chans : chans + orderToChannelsV(order);
}

inline int AmbiBase::orderToChannelsH(int orderH){ return (orderH << 1) + 1; }
inline int AmbiBase::orderToChannelsV(int orderV){ return orderV * orderV; }

inline int AmbiBase::channelsToOrder(int channels)
{
	int order = -1;
	switch(channels) {
	case 3:
	case 4:
		order = 1;
		break;
	case 9:
		order = 2;
		break;
	case 16:
		order = 3;
		break;
	default:
		order = -1;
	}
	return order;
}

inline int AmbiBase::channelsToDimensions(int channels)
{
	int dim = 3;
	switch(channels) {
	case 3:
		dim = 2;
		break;
	case 4:
	case 9:
	case 16:
		dim = 3;
		break;
	default:
		dim = -1;
	}
	return dim;
}

template<typename T>
void AmbiBase::resize(T *& a, int n){
	delete[] a;
	a = new T[n];
	memset(a, 0, n*sizeof(T));
}


// AmbiDecode

inline float AmbiDecode::decode(float * encFrame, int encNumChannels, int speakerNum){
	float smp = 0;
	float * dec = mDecodeMatrix + speakerNum * channels();
	float * wc = mWeights;
	for(int i=0; i<encNumChannels; ++i) smp += *dec++ * *wc++ * *encFrame++;
	return smp;
}

//inline float AmbiDecode::decode(int speakerNum){
//	return decode(mFrame, channels(), speakerNum);
//}

//inline float * AmbiDecode::azimuths(){ return mPositions; }
//inline float * AmbiDecode::elevations(){ return mPositions + mNumSpeakers; }
//inline float * AmbiDecode::frame() const { return mFrame; }



// AmbiEncode
//inline void AmbiEncode::encode(const AmbiDecode &dec, float input){
//	for(int c=0; c<dec.channels(); ++c) dec.frame()[c] = weights()[c] * input;
//}
//
//inline void AmbiEncode::encodeAdd(const AmbiDecode &dec, float input){
//	for(int c=0; c<dec.channels(); ++c) dec.frame()[c] += weights()[c] * input;
//}

inline void AmbiEncode::direction(float az, float el){
	AmbiBase::encodeWeightsFuMa(mWeights, mDim, mOrder, az, el);
}

inline void AmbiEncode::direction(float x, float y, float z){
	AmbiBase::encodeWeightsFuMa(mWeights, mDim, mOrder, x,y,z);
}

inline void AmbiEncode::encode(float * ambiChans, int numFrames, int timeIndex, float timeSample) const {

	// "Iterate" through spherical harmonics using Duff's device.
	// This requires only a simple jump per time sample.
	#define CS(chanindex) case chanindex: ambiChans[chanindex*numFrames+timeIndex] += weights()[chanindex] * timeSample;
	int ch = channels()-1;
	switch(ch){
		CS(15) CS(14) CS(13) CS(12) CS(11) CS(10) CS( 9) CS( 8)
		CS( 7) CS( 6) CS( 5) CS( 4) CS( 3) CS( 2) CS( 1) CS( 0)
		default:;
	}
	#undef CS
}

inline void AmbiEncode::encode(float * ambiChans, const float * input, int numFrames)
{
	float * pAmbi = ambiChans; // non-interleaved ambi buffers, we can use fast pointer arithmetic

	for(int c=0; c<channels(); ++c){
		const float * pInput = input;
		float weight = weights()[c];
		for(int i=0; i<numFrames; ++i){
			*pAmbi++ += weight * *pInput++;
		}
	}
}

template <class XYZ>
void AmbiEncode::encode(float * ambiChans, const XYZ * dir, const float * input, int numFrames){

	// TODO: how can we efficiently encode a moving source?

	// Changing the position recomputes ALL the spherical harmonic weights.
	// Ideally we only want to change the position for each time sample.
	// However, for our loops to be most efficient, we want the inner loop
	// to process over time since it will have around 64-512 iterations
	// while the spatial loop will have at most 16 iterations.

	/* outer-space, inner-time
	for(int c=0; c<channels(); ++c){
		float * ambi = ambiChans[c];
		//T
		for(int i=0; i<numFrames; ++i){
			position(pos[i][0], pos[i][1], pos[i][2]);
			ambi[i] += weights()[c] * input[i];
		}
	}*/

	// outer-time, inner-space
	for(int i=0; i<numFrames; ++i){
		direction(dir[i][0], dir[i][1], dir[i][2]);
		encode(ambiChans, numFrames, i, input[i]);
		/*for(int c=0; c<channels(); ++c){
			ambiChans[c][i] += weights()[c] * input[i];
		}*/
	}
}


inline float * AmbisonicsSpatializer::ambiChans(unsigned channel) {
	return &mAmbiDomainChannels[channel * mNumFrames];
}

} // al::
#endif
