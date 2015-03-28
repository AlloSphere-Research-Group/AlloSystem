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
    Ryan McGee, 2012-2015, ryanmichaelmcgee@gmail.com
*/

#include <math.h>
#include <string.h>
#include <vector>
#include <list>
#include "allocore/types/al_Buffer.hpp"
#include "allocore/math/al_Interpolation.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/spatial/al_Pose.hpp"
#include "allocore/io/al_AudioIO.hpp"
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


/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
class Listener;
class SoundSource;
    
/// Abstract class for all spatializers: Ambisonics, DBAP, VBAP, etc.
class Spatializer {
public:
    Spatializer(SpeakerLayout& sl) : enabled(true){
        unsigned numSpeakers = sl.mSpeakers.size();
		for(unsigned i=0;i<numSpeakers;++i){
			mSpeakers.push_back(sl.mSpeakers[i]);
		}
    };
    virtual ~Spatializer(){};
    virtual void numFrames(int v){ };
    
    /// perform any necessary updates when the listener or speaker layout changes, ex. new speaker triplets for VBAP
    virtual void compile(Listener& l){};
    
    /// called once per listener, before sources are rendered. ex. zero ambisonics coefficients
    virtual void prepare(){};
    
    #if !ALLOCORE_NO_PORTAUDIO //if using AudioIOData and Allocore's PortAudio wrapper for an audio callback (standard AlloCore app)
    /// render each source per sample
    virtual void perform(
                         AudioIOData& io,
                         SoundSource& src,
                         Vec3d& relpos,
                         const int& numFrames,
                         int& frameIndex,
                         float& sample) = 0;
    
    //render each source per buffer
    virtual void perform(
                         AudioIOData& io,
                         SoundSource& src,
                         Vec3d& relpos,
                         const int& numFrames,
                         float *samples)= 0;
    
    /// called once per listener, after sources are rendered. ex. ambisonics decode
    virtual void finalize(AudioIOData &io){};
    
    #else //if using another API's callback (JUCE, for example)
    /// render each source per sample
    virtual void perform(
                         float** outputBuffers,
                         SoundSource& src,
                         Vec3d& relpos,
                         const int& numFrames,
                         int& frameIndex,
                         float& sample) = 0;
    
    //render each source per buffer
    virtual void perform(
                         float** outputBuffers,
                         SoundSource& src,
                         Vec3d& relpos,
                         const int& numFrames,
                         float *samples)= 0;
    
    /// called once per listener, after sources are rendered. ex. ambisonics decode
    virtual void finalize(float **outs, const int numFrames){};
    #endif
    
    int numSpeakers() const { return mSpeakers.size(); }
    
    void setEnabled(bool _enable) {enabled = _enable;}
    
protected:
    Speakers mSpeakers;
    bool enabled;
    
};
/////////////////////////////////////////////////////
/////////////////////////////////////////////////////

class Listener {
public:

	/// Set current pose
	void pose(const Pose& p) { mPose.set(p); }
	Pose& pose(){ return mPose; }

	/// Get current pose
	const Pose& pose() const { return mPose; }
    
    /// Perform any necessary updates when the listener or speaker layout changes, ex. new speaker triplets for VBAP
    void compile(){
		isCompiled = true;
		mSpatializer->compile(*(this));
	}
    
    std::vector<Quatd> mQuatHistory;		// buffer of interpolated orientations
	ShiftBuffer<4, Vec3d> mPosHistory;		// position in previous blocks
	Quatd mQuatPrev;						// orientation in previous block
	Pose mPose;								// current position
    
    Spatializer *mSpatializer;
    
protected:
	friend class AudioScene;
    friend class AmbisonicsSpatializer;
    
    Listener(int numFrames_, Spatializer *spatializer)
	:	mSpatializer(spatializer), isCompiled(false){
		numFrames(numFrames_);
	}
	
	void numFrames(unsigned v){
		if(mQuatHistory.size() != v) mQuatHistory.resize(v);
        mSpatializer->numFrames(v);
	}

    bool isCompiled;
};


/*
	Attenuation policy is different per source (because a bee has a different attenuation characteristic than an aeroplane)
		
	Nearclip is the point at which amplitude reaches 1 (and remains at 1 within nearclip)
	(Nearclip+ClipRange) is the point at which amplitude reaches its mimumum (zero by default)
	AmpFar is the amplitude at farclip (the minimum amplitude)
	
	Internal buffer needs to be long enough for the most distant sound:
		samples = sampleRate * (nearClip+clipRange)/speedOfSound
	Probably want to add to this the current buffersize plus 1.
*/
class SoundSource {
public:
    
    enum EAttenuationType {
        eAttenOneOverD,
        eAttenOneOverD2
    };
    
	SoundSource(double rollOff=1, double near=1, double range=100, double ampFar=0, int bufSize=50000)
	:	mSound(bufSize), mRollOff(rollOff), mNearClip(near), mClipRange(range), mAmpFar(ampFar), useAtten(true), useDoppler(true), perSampleProcessing(false),
        attenuationType(eAttenOneOverD) {
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
    
    /// Set attenuation type (1/D or 1/D2)
    void setAttenuationType(EAttenuationType atten){ attenuationType = atten;}

	/// Returns attentuation factor based on distance to listener
	double attenuation(double distance) const {
	
        //TODO make enum for curves, make virtual
        
        if(!useAtten) return 1.0;
        
		if (distance < mNearClip)
			return 1.0;
        else if (distance > (mNearClip+mClipRange))
			return mAmpFar;
        else {
            if(attenuationType == eAttenOneOverD)
                return mAmpFar + 1.0/distance;
            else if(attenuationType == eAttenOneOverD2)
                return mAmpFar + 1.0/(distance*distance);
            
//			// normalized distance (0..1)
//			double dN = (distance-mNearClip) / (mClipRange);
//		
//			// different possible policies for amplitude attenuation
//			// amplitude curve (max/cosm):
//			//curve = (1.-dN)*(1.-dN);	
//			// alternative curve (hydrogen bond):
//			//curve = ((d + C) / (d*d + d + C))^2;	// e.g. C=2
//			// alternative curve (skewed sigmoid):
//            // alternative methods using rollOff
//            
//			double curve = 1-tanh(M_PI * dN*dN);
//			return mAmpFar + curve*(1.-mAmpFar);
		}
        return 1.0;
	}

    /// Enable/disable distance-based gain attenuation
    void enableAttenuation(bool enable) { useAtten = enable; }
    
    /// Enable/disable Doppler shift
    void enableDoppler(bool enable) { useDoppler = enable; }
    
	/// Get size of delay in samples
	int delaySize() const { return mSound.size(); }

	/// Convert delay, in seconds, to an index
	double delayToIndex(double delay, double sampleRate) const {
        if(!useDoppler) return 0;
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
//        float a0 = mSound.read(index0-1);
//        float b1 = mSound.read(index0+2);
//        return ipl::cubic(frac, a0, a, b, b1);
	}

	/// Set far clipping distance
	void farClip(double v){ mClipRange=(v-mNearClip); }
    
    /// Set far clipping amplitude (minimum amplitude)
	void ampFar(double v){ mAmpFar=v; }

	/// Set near clipping distance	
	void nearClip(double v){ mNearClip=v; }

	/// Set roll off amount
	void rollOff(double v){ mRollOff=v; }	

	/// Write sample to internal delay-line
	void writeSample(const double& v){ mSound.write(v); }
    
    /// Optional onProcessSample so sample rate processing of sound sources can be used in AudioScene
    virtual void onProcessSample(int frame){}//AudioIOData& io){}
    void usePerSampleProcessing(bool _usePerSampleProcessing) { perSampleProcessing = _usePerSampleProcessing;}
    bool isUsingPerSampleProcessing() { return perSampleProcessing; }
    
    bool isUsingDoppler(){return useDoppler;}

    RingBuffer<float> mSound;			// spherical wave around position
	Pose mPose;							// current position/orientation
	ShiftBuffer<4, Vec3d> mPosHistory;	// previous positions
    
protected:
	friend class AudioScene;
    friend class Dbap;
	
	double mRollOff;
	double mNearClip, mClipRange, mAmpFar;
    
    bool useAtten, useDoppler, perSampleProcessing;
    
    EAttenuationType attenuationType;
};	


class AudioScene {
public:

    typedef std::vector<Listener *> Listeners;
	typedef std::list<SoundSource *> Sources;

    AudioScene(int numFrames)
	:   mNumFrames(numFrames), mSpeedOfSound(344), perSampleProcessing(false)
	{}

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
    
    /// Create a new listener for this scene using the given spatializer
    Listener * createListener(Spatializer* spatializer) {
		Listener * l = new Listener(mNumFrames, spatializer);
        //l->pose(Pose(Vec3f(0, 0, 0.1))); //TODO why the click at 0, 0, 0?
        l->compile();
		mListeners.push_back(l);
		return l;
	}
	
    void removeListeners(){
        mListeners.clear();
    }
    
	void addSource(SoundSource& src) {
		mSources.push_back(&src);
	}
	
	void removeSource(SoundSource& src) {
		mSources.remove(&src);
	}
    
    /// Per sample processing is off by default.
    /// Per sample processing is useful for smoother doppler and gain interpolation for high-speed sources,
    /// but uses much more CPU.
    void usePerSampleProcessing(bool shouldUsePerSampleProcessing){
        perSampleProcessing = shouldUsePerSampleProcessing;
    }
    
#if !ALLOCORE_NO_PORTAUDIO
    void render(AudioIOData& io) {
        const int numFrames = io.framesPerBuffer();
        double sampleRate = io.framesPerSecond();
#else
    void render(float **outputBuffers, const int numFrames, double sampleRate) {
#endif
        
        // scalar factor to convert distances into delayline indices
        // varies per source,
        // since each source has its own buffersize and far clip
        // (not physically accurate of course)
        double distanceToSample = sampleRate / mSpeedOfSound;
        
		// iterate through all listeners adding contribution from all sources
		for(unsigned il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			
			Spatializer* spatializer = l.mSpatializer;
			spatializer->prepare();

			// update listener history data:
			Quatd qnew = l.pose().quat();
			Quatd::slerpBuffer(l.mQuatPrev, qnew, &l.mQuatHistory[0], numFrames);
			l.mQuatPrev = qnew;
			l.mPosHistory(l.pose().pos());

			// iterate through all sound sources
			for(Sources::iterator it = mSources.begin(); it != mSources.end(); ++it){
				SoundSource& src = *(*it);
				
                if(perSampleProcessing) //Original, inefficient, per sample processing
                {
                    // iterate time samples
                    for(int i=0; i<numFrames; ++i){
                        
                        // per sample source processing and update source history (only for the first listener)
                        if(il == 0){
                            src.mPosHistory(src.pose().pos());
                            if(src.isUsingPerSampleProcessing())
                                src.onProcessSample(i);
                        }
                    
                        // compute interpolated source position relative to listener
                        // TODO: this tends to warble when moving fast
                        double alpha = double(i)/numFrames;

                        // moving average:
                        // cheaper & slightly less warbly than cubic,
                        // less glitchy than linear
                        Vec3d relpos = (
                            (src.mPosHistory[3]-l.mPosHistory[3])*(1.-alpha) + 
                            (src.mPosHistory[2]-l.mPosHistory[2]) + 
                            (src.mPosHistory[1]-l.mPosHistory[1]) + 
                            (src.mPosHistory[0]-l.mPosHistory[0])*(alpha)
                        )/3.0;

                        //double distance = relpos.mag();
                        
                        double distance = src.mPosHistory[0].mag();
                        double prevDistance = src.mPosHistory[1].mag();
                        
                        //physically accurate doppler
                        double sourceVel = (distance - prevDistance)*sampleRate; //positive when moving away, negative moving toward
                        if(sourceVel > mSpeedOfSound) sourceVel = mSpeedOfSound;
                        else if(sourceVel < (1-mSpeedOfSound)) sourceVel = 1 - mSpeedOfSound;
                        
                        if(src.useDoppler)
                            distanceToSample = sampleRate / (mSpeedOfSound + sourceVel);
                        else
                            distanceToSample = 0;
                        
                        double idx = distance * distanceToSample;
                        
                        // are we within range?
                        if(idx <= src.maxIndex()-numFrames){
                        //if (distance < src.farClip()) {

                            if(!src.isUsingPerSampleProcessing())
                                idx += (numFrames-i);
                            
                            double gain = src.attenuation(distance);
                            
                            float s = src.readSample(idx) * gain;
                            
                            #if !ALLOCORE_NO_PORTAUDIO
                            spatializer->perform(io, src,relpos, numFrames, i, s);
                            #else
                            spatializer->perform(outputBuffers, src,relpos, numFrames, i, s);
                            #endif
                            
                        } // end if in range

                    } //end for each frame
                } //end per sample processing
                //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                else //more efficient, per buffer processing
                {
                    //update source history data (only for the first listener)
                    if(il == 0)
                    {
                        src.mPosHistory(src.pose().pos());
                        // TODO ? src.onProcessBuffer(numFrames);
                    }
                    
                    Vec3d relpos = src.pose().pos() - l.pose().pos();
                    double distance = relpos.mag();
                    double gain = src.attenuation(distance);
                    
                    float samples[numFrames];
                    for(int i = 0; i < numFrames; i++)
                    {                        
                        double readIndex = distance * distanceToSample;
                        readIndex += (numFrames-i);
                        samples[i] = gain*src.readSample(readIndex);
                    }
                    #if !ALLOCORE_NO_PORTAUDIO
                    spatializer->perform(io, src,relpos, numFrames, samples);
                    #else
                    spatializer->perform(outputBuffers, src, relpos, numFrames, samples);
                    #endif
                }
                
                
			} //end for each source
            
            #if !ALLOCORE_NO_PORTAUDIO
            spatializer->finalize(io);
            #else
            spatializer->finalize(outputBuffers, numFrames);
            #endif
            
		} // end for each listener
		
	} //end render


protected:

	Listeners mListeners;
	Sources mSources;
    
	int mNumFrames;			// audio frames per block
	double mSpeedOfSound;	// distance per second
    
    bool perSampleProcessing;
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
