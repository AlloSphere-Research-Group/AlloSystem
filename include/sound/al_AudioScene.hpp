#ifndef INCLUDE_AL_AUDIOSCENE_HPP
#define INCLUDE_AL_AUDIOSCENE_HPP

#include <vector>
#include "types/al_Buffer.hpp"
#include "math/al_Vec.hpp"
#include "spatial/al_CoordinateFrame.hpp"
#include "sound/al_Ambisonics.hpp"
#include "sound/al_Reverb.hpp"


namespace al{



class Listener : public NavRef {
public:

	int numSpeakers() const { return mDecoder.numSpeakers(); }

	Listener& numSpeakers(int num){ mDecoder.numSpeakers(num); return *this; }

	Listener& speakerPos(int ind, T az, T el=0){ mDecoder.setSpeakerDegrees(ind,az,el); return *this; }

protected:
	friend class AudioScene;
	AmbiDecode<float> mDecoder;
};



class SoundSource : public NavRef {
public:
	SoundSource(int bufSize=1024): mSound(bufSize)
	{}

	void writeSample(const float& v){ mSound.write(v); }

protected:
	Buffer<float> mSound;	// spherical wave around position
};



class AudioScene {
public:

	/// Encode sources
	
	/// 
	///
	void encode(float ** ambiChans, const int& numFrames){
		
	}

	void addListener(){
		//mListeners.push_back
	}

	void render(float ** outs, const float ** ambiChans, const int& numFrames){

		for(int il=0; il<mListeners.size(); ++il){

			Listener& l = mListeners[il];

			// loop speakers
			for(int is=0; is<l.numSpeakers(); ++is){
				float * out = outs[is]; // may have to do a channel mapping  //io.out(chanMap[s]);
				
				// loop ambi channels
				for(unsigned int ic=0; ic<l.mDecoder.numChannels(); ++ic){
					const float * in = ambiChans[ic]; // io.aux(c + AMBI_NUM_SOURCES);
					float w = l.mDecoder.decodeWeight(is, ic);
					
					for(unsigned int i=0; i<numFrames; ++i) out[i] += in[i] * w * 8.f;
				}		
			}
		}
	}

protected:
	std::vector<Listener> mListeners;
	std::vector<Source> mSources;
	AmbiEncode<float> mEncoder;
	
	int numAmbiChans(){ return 16; }
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
