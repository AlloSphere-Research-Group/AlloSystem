#ifndef INCLUDE_AL_AUDIOSCENE_HPP
#define INCLUDE_AL_AUDIOSCENE_HPP

#include <vector>
#include <list>
#include <strings.h> // bzero
#include "allocore/types/al_Buffer.hpp"
#include "allocore/math/al_Interpolation.hpp"
#include "allocore/math/al_Vec.hpp"
#include "allocore/spatial/al_CoordinateFrame.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
#include "allocore/sound/al_Reverb.hpp"

namespace al{


/// Audio scene listener object

/// 
///
class Listener : public Nav {
public:

	int numSpeakers() const { return mDecoder.numSpeakers(); }

	Listener& numSpeakers(int num){
		mDecoder.numSpeakers(num); return *this;
	}

	Listener& speakerPos(int speakerNum, int deviceChannel, double az, double el=0){
		mDecoder.setSpeaker(speakerNum, deviceChannel, az, el);
		return *this;
	}

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
	std::vector<Quatd> mQuatHistory;		// buffer of interpolated orientations
	ShiftBuffer<4, Vec3d> mPosHistory;		// position in previous blocks
	Quatd mQuatPrev;						// orientation in previous block

	void zeroAmbi(){
		bzero(ambiChans(), mAmbiDomainChannels.size()*sizeof(ambiChans()[0]));
	}
};



class SoundSource : public Nav {
public:
	SoundSource(double rollOff=1, double near=0.1, double far=100, int bufSize=1024)
	:	mSound(bufSize), mRollOff(rollOff), mNearClip(near), mFarClip(far)
	{}

	/// Get far clipping distance
	double farClip() const { return mFarClip; }

	/// Get near clipping distance
	double nearClip() const { return mNearClip; }

	/// Get roll off factor
	double rollOff() const { return mRollOff; }

	/// Returns attentuation factor based on distance to listener
	double attenuation(double distance) const {
	
		double d = distance;
		if(d < nearClip()){ d=nearClip(); }
		else if(d > farClip()){ d=farClip(); }

		// inverse
		return nearClip() / (nearClip() + rollOff() * (d - nearClip()));
		
//		// exponential
//		return pow(d / nearClip(), -rollOff());
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
		float a = mSound.atRel(index0);
		float b = mSound.atRel(index0+1);
		float frac = index - index0;
		return ipl::linear(frac, a, b);
	}


	/// Set far clipping distance
	void farClip(double v){ mFarClip=v; }

	/// Set near clipping distance	
	void nearClip(double v){ mNearClip=v; }

	/// Set roll off amount
	void rollOff(double v){ mRollOff=v; }	

	/// Write sample to internal delay-line
	void writeSample(const float& v){ mSound.write(v); }

protected:
	friend class AudioScene;
	
	Buffer<float> mSound;	// spherical wave around position
	ShiftBuffer<4, Vec3d> mPosHistory; // previous positions
	
	double mRollOff;
	double mNearClip, mFarClip;
};	



class AudioScene {
public:

	AudioScene(int dim, int order, int numFrames) 
	:	mEncoder(dim, order), mNumFrames(numFrames), mSpeedOfSound(343)
	{}
	
	int dim() const { return mEncoder.dim(); }
	int order() const { return mEncoder.order(); }
	
	// TODO: setNumFrames

	Listener& createListener(int numspeakers) {
		Listener * l = new Listener(dim(), order(), numspeakers, 1, mNumFrames);
		mListeners.push_back(l);
		return *l;
	}
	
	void addSource(SoundSource& src) {
		mSources.push_back(&src);
	}
	
	void removeSource(SoundSource& src) {
		mSources.remove(&src);
	}

	/// encode sources (per listener)
	void encode(const int& numFrames, double sampleRate) {
	
		//const double invClipRange = mFarClip - mNearClip;
		
		double distanceToSample = sampleRate / mSpeedOfSound;
		distanceToSample = 16;
	
		// update source history data:
		for(Sources::iterator it = mSources.begin(); it != mSources.end(); it++) {
			SoundSource& src = *(*it);
			src.mPosHistory(src.vec());
		}
	
		//printf("%d, %d\n", (int)mListeners.size(), (int)mSources.size());
	
		// iterate through all listeners adding contribution from all sources
		for(unsigned il=0; il<mListeners.size(); ++il){
			Listener& l = *mListeners[il];
			
			// update listener history data:
			Quatd qnew = l.quat();
			Quatd::slerp_buffer(l.mQuatPrev, qnew, &l.mQuatHistory[0], numFrames);
			l.mQuatPrev = qnew;
			l.mPosHistory(l.vec());
			
			l.zeroAmbi(); // zero out existing ambi samples

			// iterate through all sound sources
			for(Sources::iterator it = mSources.begin(); it != mSources.end(); ++it){
				SoundSource& src = *(*it);
				
				// iterate time samples
				for(int i=0; i<numFrames; ++i){
					
					// compute interpolated source position relative to listener
					// TODO: this tends to warble when moving fast
					float alpha = float(i)/numFrames;
					Vec3d relpos = ipl::cubic(
						alpha, 
						src.mPosHistory[3]-l.mPosHistory[3], 
						src.mPosHistory[2]-l.mPosHistory[2], 
						src.mPosHistory[1]-l.mPosHistory[1], 
						src.mPosHistory[0]-l.mPosHistory[0]
					);

//					Vec3d relpos = ipl::linear(
//						alpha,
//						src.mPosHistory[1]-l.mPosHistory[1], 
//						src.mPosHistory[0]-l.mPosHistory[0]
//					);

					//Vec3d relpos = src.mPosHistory[0]-l.mPosHistory[0];

					//printf("%g %g %g\n", l.pos()[0], l.pos()[1], l.pos()[2]);
					//printf("%g %g %g\n", src.pos()[0], src.pos()[1], src.pos()[2]);

					double distance = relpos.mag();
					double idx = distance * distanceToSample;
					//printf("%g\n", distance);

					int idx0 = idx;
					
					// are we within range?
					if(idx0 <= src.maxIndex()-numFrames){

						idx += (numFrames-i);
						
						double gain = src.attenuation(distance);
						
						float s = src.readSample(idx) * gain;
						//printf("%g\n", s);
						//printf("%g\n", idx);

						relpos.normalize();
						mEncoder.direction(relpos[0], relpos[1], relpos[2]);
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
				
				//void encode(float ** ambiChans, const XYZ * pos, const float * input, numFrames)
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

protected:
	typedef std::vector<Listener *> Listeners;
	typedef std::list<SoundSource *> Sources;

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
