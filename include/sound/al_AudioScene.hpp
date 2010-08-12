#ifndef INCLUDE_AL_AUDIOSCENE_HPP
#define INCLUDE_AL_AUDIOSCENE_HPP

#include <vector>
#include <list>
#include "types/al_Buffer.hpp"
#include "math/al_Vec.hpp"
#include "spatial/al_CoordinateFrame.hpp"
#include "sound/al_Ambisonics.hpp"
#include "sound/al_Reverb.hpp"
#include "math/al_Interpolation.hpp"

namespace al{



class Listener : public NavRef {
public:

	Listener& numSpeakers(int num){ mDecoder.numSpeakers(num); return *this; }
	Listener& speakerPos(int speakerNum, int deviceChannel, double az, double el=0){ mDecoder.setSpeaker(speakerNum, deviceChannel, az, el); return *this; }

	int numSpeakers() const { return mDecoder.numSpeakers(); }
	float * ambiChans() { return &mAmbiDomainChannels[0]; }
	
protected:
	friend class AudioScene;
	
	Listener(int dim, int order, int numspeakers, int flavor, int numFrames) 
	:	mDecoder(dim, order, numspeakers, flavor) 
	{
		mQuatHistory.resize(numFrames);
		mAmbiDomainChannels.resize(mDecoder.channels() * numFrames);
	}
	
	AmbiDecode mDecoder;
	
	std::vector<float> mAmbiDomainChannels;
	std::vector<Quatd> mQuatHistory;				// buffer of slerped orientations
	ShiftBuffer<4, Vec3d> mPosHistory;	// position in previous blocks
	Quatd mQuatPrev;					// orientation in previous block
	
};



class SoundSource : public NavRef {
public:
	SoundSource(int bufSize=1024): mSound(bufSize)
	{}

	void writeSample(const float& v){ mSound.write(v); }
	
	float readSample(double index) {
		float a = mSound.atRel(index);
		float b = mSound.atRel(index+1);
		float frac = index - (int)index;
		return ipl::linear(frac, a, b);
	}
	
	double delay2index(al_sec delay, double samplerate) {
		return delay * samplerate;
	}
	
	al_sec maxDelay(double samplerate) {
		return mSound.size() / samplerate;
	}
	double maxIndex() { return mSound.size(); }

protected:
	friend class AudioScene;
	
	Buffer<float> mSound;	// spherical wave around position
	ShiftBuffer<4, Vec3d> mPosHistory; // previous positions
	
};	



class AudioScene {
public:

	AudioScene(int dim, int order, int numFrames) 
	:	mEncoder(dim, order), mNumFrames(numFrames), mSpeedOfSound(343), mNearClip(0.1), mFarClip(100)
	{}
	
	int dim() { return mEncoder.dim(); }
	int order() { return mEncoder.order(); }
	
	// TODO: setNumFrames

	Listener& createListener(int numspeakers) {
		mListeners.push_back(new Listener(dim(), order(), numspeakers, 1, mNumFrames));
		return *mListeners.back();
	}
	
	void addSource(SoundSource & src) {
		mSources.push_back(&src);
	}
	
	void removeSource(SoundSource & src) {
		mSources.remove(&src);
	}

	/// encode sources (per listener)
	void encode(const int& numFrames, double samplerate) {
	
		const double invClipRange = mFarClip - mNearClip;
	
		// update source history data:
		for(std::list<SoundSource *>::iterator it = mSources.begin(); it != mSources.end(); it++) {
			SoundSource& src = *(*it);
			src.mPosHistory(src.vec());
		}
	
		for(int il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			
			// update listener history data:
			Quatd qnew = l.quat();
			Quatd::slerp_buffer(l.mQuatPrev, qnew, &l.mQuatHistory[0], numFrames);
			l.mQuatPrev = qnew;
			l.mPosHistory(l.vec());
			
			float * ambiChans = l.ambiChans();
			
			for(std::list<SoundSource *>::iterator it = mSources.begin(); it != mSources.end(); it++) {
				SoundSource& src = *(*it);
				
				for(unsigned int i=0; i<numFrames; ++i) {
					
					// interpolated source position relative to listener
					float alpha = i/(float)numFrames;
					Vec3d relpos = ipl::cubic(
						alpha, 
						src.mPosHistory[3]-l.mPosHistory[3], 
						src.mPosHistory[2]-l.mPosHistory[2], 
						src.mPosHistory[1]-l.mPosHistory[1], 
						src.mPosHistory[0]-l.mPosHistory[0]
					);
					
					double distance = relpos.mag();
					double index = samplerate * (distance / mSpeedOfSound);
					double x = distance - mFarClip;
					if (x >= 0. || index < src.maxIndex()) {
						
						double amp = 1;
						// TODO: amplitude rolloff
						if (distance > mNearClip) {
							
//							// 
//							amp = 
//							
//							const double c = 
//							const double f = mFarClip;
//							
//							double denom = cf + f - x;
//							
//							// smooth to zero at farcilp:
//							amp *= (1 - (cx) / (cf - f + x));
						}
					
						// TODO: transform relpos by listener's perspective
						// to derive x,y,z in listener's coordinate frame (or az/el/dist)
						Quatd q = l.mQuatHistory[i];
						//...
						
					
					} // otherwise, it's too far away for the doppler.... (culled)
				}
				
				//mEncoder.encode<Vec3d>(ambiChans, mSource);void encode(float ** ambiChans, const XYZ * pos, const float * input, numFrames)
			}
			
		}
	}
	
	/// between encode & decode, can apply optional processing to ambi domain signals (e.g. reverb)
	

	/// decode sources (per listener) to output channels
	void render(float * outs, const int& numFrames) {
		for(int il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			l.mDecoder.decode(outs, l.ambiChans(), numFrames);
		}
	}

protected:
	std::vector<Listener *> mListeners;
	std::list<SoundSource *> mSources;
	AmbiEncode mEncoder;
	int mNumFrames;			// audio frames per block
	double mSpeedOfSound;	// distance per second
	double mNearClip, mFarClip;
	
	// how slowly amplitude decays away from mNearClip. 
	// 1-> 50% at farclip, 10 -> 10% at farclip, 100 -> 1% at farclip, 1000 -> 0.1%  
	// (if mKneeSmoothness is near zero; as d increases, amplitude at farclip increases slightly
	double mRollOff;	
	
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


//
///// Referential interface to a multi-channel audio i/o frame
//class AudioFrame {
//public:
//
//	AudioFrame(int chansOut=0, int chansIn=0)
//	:	mIndex(0)
//	{	channels(chansOut, chansIn); }
//
//	/// Get sample from input channel
//	float in(int channel){ return mIn[channel][mIndex]; }
//
//	/// Add sample to output channel
//	void out(float v, int channel){ mOut[channel][mIndex] += v; }
//	void out(float v, int ch1, int ch2){ out(v,ch1); out(v,ch2); }
//	void out(float v, int ch1, int ch2, int ch3){ out(v,ch1,ch2); out(v,ch3); }
//
//	/// Get frame index in block
//	int index() const { return mIndex; }
//
//private:
////	friend class AudioContext;
////	friend class AudioContextImpl;
//	int mIndex;
//	std::vector<const float *> mIn;
//	std::vector<float *> mOut;
//	
//	AudioFrame& index(int v){ mIndex=v; return *this; }
//	void channelsIn (int v){ if(v >  mIn.size())  mIn.resize(v); }
//	void channelsOut(int v){ if(v > mOut.size()) mOut.resize(v); }
//	void channels(int vo, int vi){ channelsIn(vi); channelsOut(vo); }
//};




// From hydrogen bond...
/*
struct AmbiSource{

	AmbiSource(uint32_t order, uint32_t dim)
	:	doppler(0.1), dist(2, 0.001), angH(2), angV(2), enc(order, dim)
	{}

	void operator()(float * outA, float * inT, int numT, float dopScale=1, float distCoef=2, float ampOffset=0){
	
		// do distance coding
		for(int i=0; i<numT; ++i){
			float dis = dist();				// distance from source
			float amp = scl::clip(distToAmp(dis, distCoef) - ampOffset, 1.f);	// computing amplitude of sound based on its distance (1/d^2)
			doppler.delay(dis*dopScale);
			inT[i] = doppler(inT[i] * amp);
		}
		
		enc.position(angH.stored() * M_PI, angV.stored() * M_PI);
		
		// loop ambi channels
		for(unsigned int c=0; c<enc.numChannels(); c++) {
			float * out = outA + numT*c;
			float w = enc.weights()[c];
			
			// compute ambi-domain signals
			for(int i=0; i<numT; ++i) *out++ += inT[i] * w;		
		}
	}
	
	// map distance from observer to a sound source to an amplitude value
	static float distToAmp(float d, float coef=2){
		
		d = scl::abs(d);	// positive vals only
		
		// we want amp=1 when distance=0 and a smooth transition across 0, so 
		// we approximate 1/x with a polynomial
		d = (d + coef) / (d*d + d + coef);	// 2nd order apx
		return d*d;
		//return exp(-d);
	}

	synz::Delay doppler;
	OnePole<> dist, angH, angV;		// Low-pass filters on source position
	AmbiEncode<float> enc;
};


	// block rate
	// (float * outA, float * inT, int numT, float dopScale=1, float distCoef=2, float ampOffset=0)
	ambiH (io.aux(AMBI_NUM_SOURCES), io.aux(0), io.numFrames(), 1./32);
	ambiO (io.aux(AMBI_NUM_SOURCES), io.aux(1), io.numFrames(), 1./32, 2, 0.1);
	ambiZn(io.aux(AMBI_NUM_SOURCES), io.aux(2), io.numFrames(), 1./32, 2, 0.1);

	//---- Ambisonic decoding:
	static float chanMap[16] = {
		 2,  3,  4,  5,  6,  7,  8,  9,		// MOTU #1
		16, 17, 18, 19, 20, 21, 22, 23		// MOTU #2
	};
	
	//void decode(T ** dec, const T ** enc, uint32_t numDecFrames);
	
	// loop speakers
	for(int s=0; s < 16; ++s){
		float * out = io.out(chanMap[s]);
		
		// loop ambi channels
		for(unsigned int c=0; c<ambiDec.numChannels(); c++){
			float * in = io.aux(c + AMBI_NUM_SOURCES);
			float w = ambiDec.decodeWeight(s, c);
			
			for(unsigned int i=0; i<io.numFrames(); ++i) out[i] += *in++ * w * 8.f;
		}		
	}
*/




} // al::
#endif
