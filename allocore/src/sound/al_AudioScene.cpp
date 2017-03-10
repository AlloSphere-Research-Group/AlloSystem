#include "allocore/sound/al_AudioScene.hpp"
#include "allocore/math/al_Constants.hpp"


#include <iostream>

namespace al{

Spatializer::Spatializer(const SpeakerLayout& sl) :
    mEnabled(true)
{
	unsigned numSpeakers = sl.speakers().size();
	for(unsigned i=0;i<numSpeakers;++i){
		mSpeakers.push_back(sl.speakers()[i]);
	}
};



void AudioSceneObject::updateHistory(){
	mPosHistory(mPose.pos());
}



Listener::Listener(int numFrames_, Spatializer *spatializer)
	:	mSpatializer(spatializer), mIsCompiled(false)
{
	numFrames(numFrames_);
}

void Listener::numFrames(unsigned v){
	if(mQuatHistory.size() != v) mQuatHistory.resize(v);
	mSpatializer->numFrames(v);
}

void Listener::updateHistory(int numFrames){
	AudioSceneObject::updateHistory();

	// Create a buffer of spherically interpolated quaternions from the previous
	// quat to the current quat:
	Quatd qnew = pose().quat();
	Quatd::slerpBuffer(mQuatPrev, qnew, &mQuatHistory[0], numFrames);
	mQuatPrev = qnew;
}

void Listener::compile(){
	mIsCompiled = true;
	mSpatializer->compile(*(this));
}



SoundSource::SoundSource(double nearClip, double farClip,
                         AttenuationLaw law, DopplerType dopplerType,
                         double sampleRate, double farBias, int delaySize
                         )
    :	DistAtten<double>(nearClip, farClip, law, farBias),
      mSound(delaySize), mUseAtten(true), mDopplerType(dopplerType), mUsePerSampleProcessing(false),
      mCachedIndex(0), mSampleRate(sampleRate), mSpeedOfSound(340), mFrameCounter(0)
{
	// initialize the position history to be VERY FAR AWAY so that we don't deafen ourselves...
	for(int i=0; i<mPosHistory.size(); ++i){
		mPosHistory(Vec3d(1000, 0, 0));
	}

	presenceFilter.set(2700);
}

int SoundSource::bufferSize(double samplerate, double speedOfSound, double distance){
	return (int)ceil(samplerate * distance / speedOfSound);
}



AudioScene::AudioScene(int numFrames_)
	:   mNumFrames(0), mPerSampleProcessing(false)
{
	numFrames(numFrames_);
}

AudioScene::~AudioScene(){
	for(
		Listeners::iterator it = mListeners.begin();
		it != mListeners.end();
		++it
		){
		delete (*it);
	}
}

void AudioScene::addSource(SoundSource& src){
	mSources.push_back(&src);
}

void AudioScene::removeSource(SoundSource& src){
	mSources.remove(&src);
}

void AudioScene::numFrames(int v){
	if(mNumFrames != v){

		Listeners::iterator it = mListeners.begin();
		while(it != mListeners.end()){
			(*it)->numFrames(v);
			++it;
		}
		mNumFrames = v;
		mBuffer.reserve(mNumFrames);
	}
}

Listener * AudioScene::createListener(Spatializer* spatializer){
	Listener * l = new Listener(mNumFrames, spatializer);
	l->compile();
	mListeners.push_back(l);
	return l;
}

/*
	So, for large numbers of sources it quickly gets too expensive.
	Having one delayline per soundsource (for doppler) is itself quite taxing.
	e.g., at 44100kHz sampling rate, a speed of sound of 343m/s and an audible distance of 50m implies a delay of at least 6428 samples (need to add blocksize to that too).
	The actual buffersize sets the effective doppler far-clip; beyond this it always uses max-delay size (no doppler)
	The head-size sets the effective doppler near-clip.
*/

void AudioScene::render(AudioIOData& io) {
	const int numFrames = io.framesPerBuffer();
	double sampleRate = io.framesPerSecond();
	io.zeroOut();

	// iterate through all listeners adding contribution from all sources
	for(unsigned il=0; il<mListeners.size(); ++il){
		Listener& l = *mListeners[il];

		Spatializer* spatializer = l.mSpatializer;
		spatializer->prepare();

		// update listener history data:
		l.updateHistory(numFrames);

		// iterate through all sound sources
		for(Sources::iterator it = mSources.begin(); it != mSources.end(); ++it){
			SoundSource& src = *(*it);
			if(mPerSampleProcessing) { //audioscene per sample processing
				src.frame(0);
				for(int i=0; i < numFrames; ++i){
					Vec3d relpos = src.pose().pos() - l.pose().pos();
					spatializer->renderSample(io, relpos, src.getNextSample(l), i);
				}
			} else { //more efficient, per buffer processing for audioscene (does not work well with doppler)
				src.getBuffer(l, mBuffer.data(), numFrames);
				Vec3d relpos = src.pose().pos() - l.pose().pos();
//				std::cout << l.pose().x() << "," << l.pose().z() << " ---- ";
//				std::cout << src.pos().x << "," << src.pos().z << " ----- ";
//				std::cout << relpos.x << "," << relpos.z << std::endl;
				spatializer->renderBuffer(io, relpos, mBuffer.data(), numFrames);
			}
		} //end for each source

		spatializer->finalize(io);

	} // end for each listener
}

} // al::


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
