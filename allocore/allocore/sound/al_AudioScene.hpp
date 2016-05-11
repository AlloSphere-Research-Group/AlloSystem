#ifndef INCLUDE_AL_AUDIOSCENE_HPP
#define INCLUDE_AL_AUDIOSCENE_HPP

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
	Rendering spatial audio with multiple sources

	File author(s):
	Lance Putnam, 2010, putnam.lance@gmail.com
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Ryan McGee, 2012, ryanmichaelmcgee@gmail.com
*/

#include <math.h>
#include <string.h>
#include <vector>
#include <list>
#include "allocore/types/al_Buffer.hpp"
#include "allocore/math/al_Interpolation.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/spatial/al_DistAtten.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/io/al_AudioIO.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "allocore/sound/al_Reverb.hpp"
#include "allocore/sound/al_Biquad.hpp"

namespace al{

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

class Listener;
class SoundSource;


/// Abstract class for all spatializers: Ambisonics, DBAP, VBAP, etc.
///
/// @ingroup allocore
class Spatializer {
public:

	/// @param[in] sl	A speaker layout to use
	Spatializer(const SpeakerLayout& sl);

	virtual ~Spatializer(){};

	//
	virtual void numFrames(int v){};

	/// Perform any necessary updates when the listener or speaker layout changes, ex. new speaker triplets for VBAP
	virtual void compile(Listener& l){};

	/// Called once per listener, before sources are rendered. ex. zero ambisonics coefficients
	virtual void prepare(){};

	/// Render each source per sample
	virtual void perform(
			AudioIOData& io,
			SoundSource& src,
			Vec3d& relpos,
			const int& numFrames,
			int& frameIndex,
			float& sample
			) = 0;

	/// Render each source per buffer
	virtual void perform(
			AudioIOData& io,
			SoundSource& src,
			Vec3d& relpos,
			const int& numFrames,
			float *samples
			) = 0;

	/// Called once per listener, after sources are rendered. ex. ambisonics decode
	virtual void finalize(AudioIOData& io){};


	/// Print out information about spatializer
	virtual void print(){};

	/// Get number of speakers
	int numSpeakers() const { return mSpeakers.size(); }

	/// Enable Spatializaion(true by default)
	void setEnabled(bool _enable) {mEnabled = _enable;}

protected:
	Speakers mSpeakers;
	bool mEnabled;
};



/// Base class for an object (listener or source) in an audio scene

/// This contains a "pose" to represent the position and orientation of a
/// scene object and a buffer of previous positions that can be used to
/// generate smooth, "zipper"-free trajectories at audio rate.
///
/// @ingroup allocore
class AudioSceneObject{
public:

	/// Set current pose
	void pose(const Pose& p) { mPose.set(p); }
	Pose& pose(){ return mPose; }

	/// Set position
	void pos(double x, double y, double z=0.){ mPose.pos(x,y,z); }


	/// Get current pose
	const Pose& pose() const { return mPose; }

	/// Get current position
	const Vec3d& pos() const { return mPose.pos(); }

	/// Get history of positions
	const ShiftBuffer<4, Vec3d>& posHistory() const { return mPosHistory; }

	void updateHistory();

protected:
	Pose mPose;							// current position/orientation
	ShiftBuffer<4, Vec3d> mPosHistory;	// previous positions
};



/// A listener in an audio space. Listener objects are not created by the user
/// but instantiated using the createListener() function from the AudioScene
/// class.
/// A Listener is tied to a spatialization technique when it is created.
///
/// @ingroup allocore
class Listener : public AudioSceneObject {
public:

	/// Get history of orientations
	const std::vector<Quatd>& quatHistory() const { return mQuatHistory; }

	void compile();

	void updateHistory(int numFrames);

protected:
	friend class AudioScene;

	Listener(int numFrames, Spatializer *spatializer);

	void numFrames(unsigned v);

	Spatializer * mSpatializer;
	std::vector<Quatd> mQuatHistory;// buffer of interpolated orientations
	Quatd mQuatPrev;				// orientation in previous block
	bool mIsCompiled;
};



/// A sound source

/// Doppler Types
enum DopplerType{
	DOPPLER_NONE=0,			/**< Do not use Doppler Shift */
	DOPPLER_SYMMETRICAL,	/**< Symmetrical Frequency Shift (not physically accurate) */
	DOPPLER_PHYSICAL			/**< Physically Accurate Doppler Shift. Requires per sample processingfor AudioScene and SoundSource. */
};

/// The attenuation policy may be different per source, i.e., because a bee has
/// a different attenuation characteristic than an airplane.
///
/// @ingroup allocore
class SoundSource : public AudioSceneObject, public DistAtten<double> {
public:

	/// @param[in] nearClip		Distance below which amplitude is clamped to 1
	/// @param[in] farClip		Distance above which amplitude reaches its mimumum
	/// @param[in] law			Attenuation law
	/// @param[in] farBias		Amplitude at far clip (the minimum amplitude)
	/// @param[in] delaySize	Size of internal delay line. This should be
	///							large enough for the most distant sound:
	///							samples = sampleRate * (near + range)/speedOfSound
	SoundSource(
	        double nearClip=0.1, double farClip=20, AttenuationLaw law = ATTEN_INVERSE, DopplerType dopplerType = DOPPLER_SYMMETRICAL,
	        double farBias=0, int delaySize=100000
	        );

	virtual ~SoundSource(){}


	/// Returns whether distance-based attenuation is enabled
	bool useAttenuation() const { return mUseAtten; }

	/// Returns Doppler Type
	DopplerType dopplerType() const { return mDopplerType; }

	/// Returns attentuation factor based on distance to listener
	double attenuation(double distance) const {
		return mUseAtten ? DistAtten<double>::attenuation(distance) : 1.0;
	}

	/// Get size of delay in samples
	int delaySize() const { return mSound.size(); }

	/// Convert delay, in seconds, to an index
	double delayToIndex(double delay, double sampleRate) const {
		if(mDopplerType == DOPPLER_NONE) return 0;
		return delay*sampleRate;
	}

	/// Returns maximum number of seconds of delay
	double maxDelay(double sampleRate) const {
		return delaySize()/sampleRate;
	}

	/// Returns maximum index that can be used for reading samples
	int maxIndex() const { return delaySize()-2; }

	/// Read sample from delay-line using linear interpolation
	/// The index specifies how many samples ago by which to read back from
	/// the buffer. The index must be less than or equal to bufferSize()-2.
	float readSample(double index) const {
		int index0 = index;
		float a = mSound.read(index0);
		float b = mSound.read(index0+1);
		float frac = index - index0;
		//return ipl::linear(frac, a, b);

		float a0 = mSound.read(index0-1);
		float b1 = mSound.read(index0+2);
		return ipl::cubic(frac, a0, a, b, b1);

	}

	/// Enable/disable distance-based gain attenuation
	void useAttenuation(bool enable){ mUseAtten = enable; }

	/// Set Doppler Type
	void dopplerType(DopplerType type){ mDopplerType = type; }

	/// Write sample to internal delay-line
	void writeSample(float v){ mSound.write(v); }

	/// optional onProcessSample for sample rate processing of sound sources
	virtual void onProcessSample(int frame){}

	/// Returns whether the source is using per sample processing (AudioScene::render will call this source's onProcessSample)
	bool usePerSampleProcessing() const { return mUsePerSampleProcessing; }

	/// Enable per sample processing
	void usePerSampleProcessing(bool shouldUsePerSampleProcessing){
		mUsePerSampleProcessing = shouldUsePerSampleProcessing;
	}

	void cachedIndex(unsigned int v){ mCachedIndex = v; }

	unsigned int cachedIndex(){ return mCachedIndex; }

	// calculate the buffersize needed for given samplerate, speed of sound & distance traveled (e.g. nearClip+clipRange).
	// probably want to add io.samplesPerBuffer() to this for safety.
	static int bufferSize(double samplerate, double speedOfSound, double distance);

	BiQuadNX presenceFilter; //used for presence filtering and spatial modulation BW control


protected:
	RingBuffer<float> mSound;		// spherical wave around position
	bool mUseAtten;
	DopplerType mDopplerType;
	bool mUsePerSampleProcessing;
    unsigned int mCachedIndex; // for VBAP with multiple sources
};


/// An audio scene consisting of Listeners and Sources.
///
/// @ingroup allocore
class AudioScene {
public:

	/// A set of listeners
	typedef std::vector<Listener *> Listeners;

	/// A set of sources
	typedef std::list<SoundSource *> Sources;


	/// @param[in] numFrames	block size of audio buffers
	AudioScene(int numFrames);

	~AudioScene();


	/// Get listeners
	Listeners& listeners(){ return mListeners; }
	const Listeners& listeners() const { return mListeners; }

	/// Get sources
	Sources& sources(){ return mSources; }
	const Sources& sources() const { return mSources; }

	void numFrames(int v);

	/// Create a new listener for this scene using the given spatializer

	/// The returned Listener is allocated internally and will be deleted
	/// in the AudioScene destructor.
	Listener * createListener(Spatializer * spatializer);

	/// Add a sound source to scene
	void addSource(SoundSource& src);

	/// Remove a sound source from scene
	void removeSource(SoundSource& src);

	/// Perform rendering
	void render(AudioIOData& io);

	/// Set per sample processing (false by default)
	/// Per sample processing is useful for smoother doppler and gain
	/// interpolation for high-speed sources, but uses much more CPU.
	/// When set to true, smoothing of the position of sources
	/// is performed within the audio buffer using a 4 point moving average
	/// When false, the position of a source will be static within the audio
	/// processing block.
	/// Turn off to reduce CPU for large number of sources and/or Doppler shift not required.
	void usePerSampleProcessing(bool shouldUsePerSampleProcessing){
		mPerSampleProcessing = shouldUsePerSampleProcessing;
	}

protected:
	Listeners mListeners;
	Sources mSources;
	int mNumFrames;				// audio frames per block
	std::vector<float> mBuffer;	// temporary frame buffer
	double mSpeedOfSound;		// distance per second
	bool mPerSampleProcessing;
};

} // al::
#endif

