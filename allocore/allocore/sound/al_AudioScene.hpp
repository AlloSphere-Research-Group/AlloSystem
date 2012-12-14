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
*/

#include <math.h>
#include <string.h>
#include <vector>
#include <list>
#include "allocore/types/al_Buffer.hpp"
#include "allocore/math/al_Interpolation.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_Speaker.hpp"
#include "allocore/sound/al_Reverb.hpp"

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


/*
	So, for large numbers of sources it quickly gets too expensive.
	Having one delayline per soundsource (for doppler) is itself quite taxing.
	e.g., at 44100kHz sampling rate, a speed of sound of 343m/s and an audible distance of 50m implies a delay of at least 6428 samples (need to add blocksize to that too). 
	The actual buffersize sets the effective doppler far-clip; beyond this it always uses max-delay size (no doppler)
	The head-size sets the effective doppler near-clip.
	
*/


/// Audio scene listener object

/// 
///
class Listener {
public:

	/// Set current pose
	void pose(const Pose& p) { mPose.set(p); }
	Pose& pose(){ return mPose; }

	/// Get current pose
	const Pose& pose() const { return mPose; }


	int numSpeakers() const { return mDecoder.numSpeakers(); }

	Listener& numSpeakers(int num){
		mDecoder.numSpeakers(num); return *this;
	}

	/// azimuth is anti-clockwise; both azimuth and elevation are in degrees
	Listener& speakerPos(int speakerNum, int deviceChannel, double az, double el=0, double amp=1.){
		mDecoder.setSpeaker(speakerNum, deviceChannel, az, el, amp);
		return *this;
	}

	float * ambiChans(unsigned channel=0) { return &mAmbiDomainChannels[channel * mNumFrames]; }
	

	
protected:
	friend class AudioScene;
	
	Listener(int dim, int order, int numspeakers, int flavor, int numFrames_) 
	:	mDecoder(dim, order, numspeakers, flavor) 
	{
		numFrames(numFrames_);
	}
	
	void numFrames(unsigned v){
		mNumFrames = v;
		if(mQuatHistory.size() != v) mQuatHistory.resize(v);
		if(mAmbiDomainChannels.size() != (mDecoder.channels() * v)){
			mAmbiDomainChannels.resize(mDecoder.channels() * v);
		}
	}

	// move to specialized ambi panner...
	AmbiDecode mDecoder;	
	std::vector<float> mAmbiDomainChannels;

	std::vector<Quatd> mQuatHistory;		// buffer of interpolated orientations
	ShiftBuffer<4, Vec3d> mPosHistory;		// position in previous blocks
	Quatd mQuatPrev;						// orientation in previous block
	Pose mPose;								// current position
	unsigned mNumFrames;

	void zeroAmbi(){
		memset(ambiChans(), 0, mAmbiDomainChannels.size()*sizeof(ambiChans()[0]));
	}
};


/*
	Attenuation policy is different per source
		(because a bee has a different attenuation characteristic than an aeroplane)
		
	Nearclip is the point at which amplitude reaches 1 (and remains at 1 within nearclip)
	(Nearclip+ClipRange) is the point at which amplitude reaches its mimumum (zero by default)
	AmpFar is the amplitude at farclip (the minimum amplitude)
	
	Internal buffer needs to be long enough for the most distant sound:
		samples = sampleRate * (nearClip+clipRange)/speedOfSound
	Probably want to add to this the current buffersize plus 1.
	
	
*/
class SoundSource {
public:
	SoundSource(double rollOff=1, double near=1, double range=32, double ampFar=0.0, int bufSize=5000)
	:	mSound(bufSize), mRollOff(rollOff), mNearClip(near), mClipRange(range), mAmpFar(ampFar)
	{
		// initialize the position history to be VERY FAR AWAY so that we don't deafen ourselves... 
		mPosHistory(Vec3d(1000, 1000, 1000));
		mPosHistory(Vec3d(1000, 1000, 1000));
		mPosHistory(Vec3d(1000, 1000, 1000));
		mPosHistory(Vec3d(1000, 1000, 1000));
	}
	
	// calculate the buffersize needed for given samplerate, speed of sound & distance traveled (e.g. nearClip+clipRange).
	// probably want to add io.samplesPerBuffer() to this for safety.
	static int bufferSize(double samplerate, double speedOfSound, double distance) {
		return (int)ceil(samplerate * distance / speedOfSound);
	}

	/// Set current pose
	void pose(const Pose& p) { mPose.set(p); }
	Pose& pose(){ return mPose; }

	/// Get current pose
	const Pose& pose() const { return mPose; }


	/// Get far clipping distance
	double farClip() const { return mNearClip+mClipRange; }
	double ampFar() const { return mAmpFar; }

	/// Get near clipping distance
	double nearClip() const { return mNearClip; }

	/// Get roll off factor
	double rollOff() const { return mRollOff; }

	/// Returns attentuation factor based on distance to listener
	double attenuation(double distance) const {
	
		if (distance<mNearClip) {
			return 1;
		} else if (distance>(mNearClip+mClipRange)) {
			return mAmpFar;
		} else {
			// normalized distance (0..1)
			double dN = (distance-mNearClip) / (mClipRange);
		
			// different possible policies for amplitude attenuation
			// amplitude curve (max/cosm):
			//curve = (1.-dN)*(1.-dN);	
			// alternative curve (hydrogen bond):
			//curve = ((d + C) / (d*d + d + C))^2;	// e.g. C=2
			// alternative curve (skewed sigmoid):
			double curve = 1-tanh(M_PI * dN*dN);
			
			return mAmpFar + curve*(1.-mAmpFar);
		}

//		double d = distance;
//		if(d < nearClip()){ d=nearClip(); }
//		else if(d > farClip()){ d=farClip(); }
//
//		// inverse
//		return nearClip() / (nearClip() + rollOff() * (d - nearClip()));
//		
////		// exponential
////		return pow(d / nearClip(), -rollOff());

	}

	/// Get size of delay in samples
	int delaySize() const { return mSound.size(); }

	/// Convert delay, in seconds, to an index
	double delayToIndex(double delay, double sampleRate) const {
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
		return ipl::linear(frac, a, b);
	}

	/// Set far clipping distance
	void farClip(double v){ mClipRange=(v-mNearClip); }
	void ampFar(double v){ mAmpFar=v; }

	/// Set near clipping distance	
	void nearClip(double v){ mNearClip=v; }

	/// Set roll off amount
	void rollOff(double v){ mRollOff=v; }	

	/// Write sample to internal delay-line
	void writeSample(const double& v){ mSound.write(v); }

protected:
	friend class AudioScene;
	
	RingBuffer<float> mSound;			// spherical wave around position
	Pose mPose;							// current position/orientation
	ShiftBuffer<4, Vec3d> mPosHistory;	// previous positions
	
	double mRollOff;
	double mNearClip, mClipRange, mAmpFar;
};	



class AudioScene {
public:

	typedef std::vector<Listener *> Listeners;
	typedef std::list<SoundSource *> Sources;


	AudioScene(int dim, int order, int numFrames) 
	:	mEncoder(dim, order), mNumFrames(numFrames), mSpeedOfSound(343)
	{}


	int dim() const { return mEncoder.dim(); }
	int order() const { return mEncoder.order(); }

	Listeners& listeners(){ return mListeners; }
	const Listeners& listeners() const { return mListeners; }

	Sources& source(){ return mSources; }
	const Sources& source() const { return mSources; }

	
	void numFrames(int v){
		if(mNumFrames != v){
			Listeners::iterator it = mListeners.begin();
			while(it != mListeners.end()){
				(*it)->numFrames(v);
				++it;
			}
			mNumFrames = v;
		}
	}

	Listener * createListener(int numspeakers) {
		Listener * l = new Listener(dim(), order(), numspeakers, 1, mNumFrames);
		mListeners.push_back(l);
		return l;
	}
	
	void addSource(SoundSource& src) {
		mSources.push_back(&src);
	}
	
	void removeSource(SoundSource& src) {
		mSources.remove(&src);
	}

	/// encode sources (per listener)
	void encode(const int& numFrames, double sampleRate) {
		//printf("__________________\n");
	
		double distanceToSample = sampleRate / mSpeedOfSound;
		//distanceToSample = 16;
	
		
		//const double invClipRange = mFarClip - mNearClip;
		
		// update source history data:
		for(Sources::iterator it = mSources.begin(); it != mSources.end(); it++) {
			SoundSource& src = *(*it);
			src.mPosHistory(src.pose().pos());
		}
	
		//printf("%d, %d\n", (int)mListeners.size(), (int)mSources.size());
	
		// iterate through all listeners adding contribution from all sources
		for(unsigned il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			
			// update listener history data:
			Quatd qnew = l.pose().quat();
			Quatd::slerpBuffer(l.mQuatPrev, qnew, &l.mQuatHistory[0], numFrames);
			l.mQuatPrev = qnew;
			l.mPosHistory(l.pose().vec());
			
			l.zeroAmbi(); // zero out existing ambi samples

			// iterate through all sound sources
			for(Sources::iterator it = mSources.begin(); it != mSources.end(); ++it){
				SoundSource& src = *(*it);
				
				// scalar factor to convert distances into delayline indices
				// varies per source, 
				// since each source has its own buffersize and far clip
				// (not physically accurate of course)
				distanceToSample = (src.maxIndex()-numFrames)/src.farClip();
				
				// iterate time samples
				for(int i=0; i<numFrames; ++i){
					
					// compute interpolated source position relative to listener
					// TODO: this tends to warble when moving fast
					double alpha = double(i)/numFrames;
					
					// note: cubic(src-listener) 
					// is equivalent to cubic(src) - cubic(listener)
//					Vec3d relpos = ipl::cubic(
//						alpha, 
//						src.mPosHistory[3]-l.mPosHistory[3], 
//						src.mPosHistory[2]-l.mPosHistory[2], 
//						src.mPosHistory[1]-l.mPosHistory[1], 
//						src.mPosHistory[0]-l.mPosHistory[0]
//					);
					
					// sounds rough!
//					Vec3d relpos = ipl::linear(
//						alpha, 
//						src.mPosHistory[1]-l.mPosHistory[1], 
//						src.mPosHistory[0]-l.mPosHistory[0]
//					);

//					// moving average:
//					// cheaper & slightly less warbly than cubic, 
//					// less glitchy than linear
					Vec3d relpos = (
						(src.mPosHistory[3]-l.mPosHistory[3])*(1.-alpha) + 
						(src.mPosHistory[2]-l.mPosHistory[2]) + 
						(src.mPosHistory[1]-l.mPosHistory[1]) + 
						(src.mPosHistory[0]-l.mPosHistory[0])*(alpha)
					)/3.0;


					//Vec3d relpos = src.mPosHistory[0]-l.mPosHistory[0];

					//printf("%g %g %g\n", l.pos()[0], l.pos()[1], l.pos()[2]);
					//printf("%g %g %g\n", src.pos()[0], src.pos()[1], src.pos()[2]);

					double distance = relpos.mag();
					
					double idx = distance * distanceToSample;
					//if (i==0) printf("%g\n", distance);
					
					

					int idx0 = idx;
					
					// are we within range?
					if(idx0 <= src.maxIndex()-numFrames){
					//if (distance < src.farClip()) {

						idx += (numFrames-i);
						
						
						double gain = src.attenuation(distance);
						
						//if (i==0) printf("in range, gain %f\n", gain);
						
						float s = src.readSample(idx) * gain;
						//printf("%g\n", s);
						//printf("%g\n", idx);
						
						// compute azimuth & elevation of relative position in
						//  current listener's coordinate frame:
						Vec3d urel(relpos);
						urel.normalize();	// unit vector in axis listener->source
						// project into listener's coordinate frame:
//						Vec3d axis;			
//						l.mQuatHistory[i].toVectorX(axis);
//						double rr = urel.dot(axis);	
//						l.mQuatHistory[i].toVectorY(axis);
//						double ru = urel.dot(axis);
//						l.mQuatHistory[i].toVectorZ(axis);
//						double rf = urel.dot(axis);
						
						// cheaper:
						Vec3d direction = l.mQuatHistory[i].rotateTransposed(urel);

						//mEncoder.direction(azimuth, elevation);
						//mEncoder.direction(-rf, -rr, ru);
						mEncoder.direction(-direction[2], -direction[0], direction[1]);
						mEncoder.encode(l.ambiChans(), numFrames, i, s);
					}

//					double x = distance - mFarClip;

//					if (x >= 0. || index < src.maxIndex()) {
//						
//						//double amp = 1;
//						// TODO: amplitude rolloff
//						if (distance > mNearClip) {
//							
////							// 
////							amp = 
////							
////							const double c = 
////							const double f = mFarClip;
////							
////							double denom = cf + f - x;
////							
////							// smooth to zero at farcilp:
////							amp *= (1 - (cx) / (cf - f + x));
//						}
//					
//						// TODO: transform relpos by listener's perspective
//						// to derive x,y,z in listener's coordinate frame (or az/el/dist)
//						Quatd q = l.mQuatHistory[i];
//						//...
//						
//					
//					} // otherwise, it's too far away for the doppler.... (culled)
				}
				
				//void encode(double ** ambiChans, const XYZ * pos, const double * input, numFrames)
				//mEncoder.encode<Vec3d>(ambiChans, mSource);
			}
		}
		//printf("%f\n", mListeners[0]->ambiChans()[0]);
	}
	
	// between encode & decode, can apply optional processing to ambi domain signals (e.g. reverb)


	/// Decode sources (per listener) to output channels
	
	/// @param[out] outs		1D array of output (non-interleaved)
	/// @param[in ] numFrames	number of frames per channel buffer
	void render(float * outs, const int& numFrames) const {
		for(unsigned il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			l.mDecoder.decode(outs, l.ambiChans(), numFrames);
		}
	}
	
	/// Copy the encoded channels directly to the output channels:
	
	/// @param[out] outs		1D array of output (non-interleaved)
	/// @param[in ] numOuts		number of channels in the outs array
	/// @param[in ] numFrames	number of frames per channel buffer
	void copyAmbiChannels(float * outs, const int& numOuts, const int& numFrames) const {
		unsigned limit = al::min(numOuts, mEncoder.channels());
		unsigned frames = al::min(numFrames, mNumFrames);
		
		// for each listener
		for(unsigned il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			// for each channel:
			for (unsigned c=0; c<limit; c++) {
				// copy the ambi channel into the output channel:
				float * out = outs + c;
				float * in = l.ambiChans(c);
				for (unsigned i=0; i<frames; i++) {
					*out++ += *in++;
				}
			}
		}
	}

protected:

	Listeners mListeners;
	Sources mSources;
	AmbiEncode mEncoder;
	int mNumFrames;			// audio frames per block
	double mSpeedOfSound;	// distance per second
	
	// how slowly amplitude decays away from mNearClip. 
	// 1-> 50% at farclip, 10 -> 10% at farclip, 100 -> 1% at farclip, 1000 -> 0.1%  
	// (if mKneeSmoothness is near zero; as d increases, amplitude at farclip increases slightly
	//double mRollOff;	
	
	// how much the curve approximates 1/(1+x)
	// 0 -> (1/1+x), 1 -> (1+1)/(x^2+x+1); typical might be 0.2 - 100 
	// (increasing mRollOff allows us to increase mKneeSmoothness)
	double mKneeSmoothness;
	
	// how sharply amplitude curves to zero near mFarClip.
	// 0 -> no fadeout, 1 is already quite a strong fadeout, default 0.01
	double mFarFadeOut;		
	
	// approx to (1/(1+x)):
	// x = distance-mNearClip
	// f = mFarClip-mNearClip
	// n = x/f (normalized distance)
	// g = mRollOff
	// d = mKneeSmoothness
	// h = gn
	// y = (h+d)/(h^2+h+d)
	
	// rolloff scalar:
	// c = mFarFadeOut
	// y *= 1-(cn)/(c+1+n)
};



// From hydrogen bond...
/*
struct AmbiSource{

	AmbiSource(uint32_t order, uint32_t dim)
	:	doppler(0.1), dist(2, 0.001), angH(2), angV(2), enc(order, dim)
	{}

	void operator()(double * outA, double * inT, int numT, double dopScale=1, double distCoef=2, double ampOffset=0){
	
		// do distance coding
		for(int i=0; i<numT; ++i){
			double dis = dist();				// distance from source
			double amp = scl::clip(distToAmp(dis, distCoef) - ampOffset, 1.f);	// computing amplitude of sound based on its distance (1/d^2)
			doppler.delay(dis*dopScale);
			inT[i] = doppler(inT[i] * amp);
		}
		
		enc.position(angH.stored() * M_PI, angV.stored() * M_PI);
		
		// loop ambi channels
		for(unsigned int c=0; c<enc.numChannels(); c++) {
			double * out = outA + numT*c;
			double w = enc.weights()[c];
			
			// compute ambi-domain signals
			for(int i=0; i<numT; ++i) *out++ += inT[i] * w;		
		}
	}
	
	// map distance from observer to a sound source to an amplitude value
	static double distToAmp(double d, double coef=2){
		
		d = scl::abs(d);	// positive vals only
		
		// we want amp=1 when distance=0 and a smooth transition across 0, so 
		// we approximate 1/x with a polynomial
		d = (d + coef) / (d*d + d + coef);	// 2nd order apx
		return d*d;
		//return exp(-d);
	}

	synz::Delay doppler;
	OnePole<> dist, angH, angV;		// Low-pass filters on source position
	AmbiEncode<double> enc;
};


	// block rate
	// (double * outA, double * inT, int numT, double dopScale=1, double distCoef=2, double ampOffset=0)
	ambiH (io.aux(AMBI_NUM_SOURCES), io.aux(0), io.numFrames(), 1./32);
	ambiO (io.aux(AMBI_NUM_SOURCES), io.aux(1), io.numFrames(), 1./32, 2, 0.1);
	ambiZn(io.aux(AMBI_NUM_SOURCES), io.aux(2), io.numFrames(), 1./32, 2, 0.1);

	//---- Ambisonic decoding:
	static double chanMap[16] = {
		 2,  3,  4,  5,  6,  7,  8,  9,		// MOTU #1
		16, 17, 18, 19, 20, 21, 22, 23		// MOTU #2
	};
	
	//void decode(T ** dec, const T ** enc, uint32_t numDecFrames);
	
	// loop speakers
	for(int s=0; s < 16; ++s){
		double * out = io.out(chanMap[s]);
		
		// loop ambi channels
		for(unsigned int c=0; c<ambiDec.numChannels(); c++){
			double * in = io.aux(c + AMBI_NUM_SOURCES);
			double w = ambiDec.decodeWeight(s, c);
			
			for(unsigned int i=0; i<io.numFrames(); ++i) out[i] += *in++ * w * 8.f;
		}		
	}
*/




} // al::
#endif
