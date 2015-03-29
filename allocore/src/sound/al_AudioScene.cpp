#include "allocore/sound/al_AudioScene.hpp"

namespace al{

    Spatializer::Spatializer(const SpeakerLayout& sl) : mEnabled(true){
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



SoundSource::SoundSource(
	double nearClip, double farClip, AttenuationLaw law,
	double farBias, int delaySize
)
:	DistAtten<double>(nearClip, farClip, law, farBias),
	mSound(delaySize), mUseAtten(true), mUseDoppler(true)
{
	// initialize the position history to be VERY FAR AWAY so that we don't deafen ourselves...
	for(int i=0; i<mPosHistory.size(); ++i){
		mPosHistory(Vec3d(1e9, 0, 0));
	}
}

/*static*/
int SoundSource::bufferSize(double samplerate, double speedOfSound, double distance){
	return (int)ceil(samplerate * distance / speedOfSound);
}



AudioScene::AudioScene(int numFrames_)
:   mSpeedOfSound(344), mPerSampleProcessing(false)
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
		mBuffer.resize(v);

		Listeners::iterator it = mListeners.begin();
		while(it != mListeners.end()){
			(*it)->numFrames(v);
			++it;
		}
		mNumFrames = v;
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
    
#if !ALLOCORE_GENERIC_AUDIOSCENE
void AudioScene::render(AudioIOData& io) {
    const int numFrames = io.framesPerBuffer();
    double sampleRate = io.framesPerSecond();
#else
void AudioScene::render(float **outputBuffers, const int numFrames, const double sampleRate) {
#endif
    
	// iterate through all listeners adding contribution from all sources
	for(unsigned il=0; il<mListeners.size(); ++il){
		Listener& l = *mListeners[il];

		Spatializer* spatializer = l.mSpatializer;
		spatializer->prepare();

		// update listener history data:
		l.updateHistory(numFrames);

		// iterate through all sound sources
        //printf("audio scene has %d sound sources!!!\n", mSources.size());
		for(Sources::iterator it = mSources.begin(); it != mSources.end(); ++it){
			SoundSource& src = *(*it);

			// scalar factor to convert distances into delayline indices
			// varies per source,
			// since each source has its own buffersize and far clip
			// (not physically accurate of course)
            double distanceToSample = 0;
            if(src.useDoppler())
                distanceToSample = sampleRate / mSpeedOfSound;

            if(mPerSampleProcessing) //Original, inefficient, per sample processing
            {
                // iterate time samples
                for(int i=0; i<numFrames; ++i){

                    // per sample source processing and update source history (only for the first listener)
                    if(il == 0){
                        src.updateHistory();
                        if(src.usePerSampleProcessing())
                            src.onProcessSample(i);
                    }
                    
                    // compute interpolated source position relative to listener
                    // TODO: this tends to warble when moving fast
                    double alpha = double(i)/numFrames;

                    // moving average:
                    // cheaper & slightly less warbly than cubic,
                    // less glitchy than linear
                    Vec3d relpos = (
                        (src.posHistory()[3]-l.posHistory()[3])*(1.-alpha) +
                        (src.posHistory()[2]-l.posHistory()[2]) +
                        (src.posHistory()[1]-l.posHistory()[1]) +
                        (src.posHistory()[0]-l.posHistory()[0])*(alpha)
                    )/3.0;

					// Get distance in world-space units
                    double dist = relpos.mag();

					// Compute how many samples ago to read from buffer
					// Start with time delay due to speed of sound
                    double samplesAgo = dist * distanceToSample;

					// Add on time delay (in samples) - only needed if the source is rendered per buffer
                    if(!src.usePerSampleProcessing())
                        samplesAgo += (numFrames-i);

					// Is our delay line big enough?
					if(samplesAgo <= src.maxIndex()){
						double gain = src.attenuation(dist);
						float s = src.readSample(samplesAgo) * gain;
#if !ALLOCORE_GENERIC_AUDIOSCENE
                        spatializer->perform(io, src,relpos, numFrames, i, s);
#else
                        spatializer->perform(outputBuffers, src,relpos, numFrames, i, s);
#endif
					}

                } //end for each frame
            } //end per sample processing
        
			else //more efficient, per buffer processing
            {
                if(il == 0) //update src history (only for the first listener)
                    src.updateHistory();
                
                Vec3d relpos = src.pose().pos() - l.pose().pos();
                double distance = relpos.mag();
                double gain = src.attenuation(distance);

                for(int i = 0; i < numFrames; i++)
                {
                    double readIndex = distance * distanceToSample;
                    readIndex += (numFrames-i);
                    mBuffer[i] = gain * src.readSample(readIndex);
                }
                
#if !ALLOCORE_GENERIC_AUDIOSCENE
                spatializer->perform(io, src,relpos, numFrames, &mBuffer[0]);
#else
                spatializer->perform(outputBuffers, src, relpos, numFrames, &mBuffer[0]);
#endif
            }

		} //end for each source

#if !ALLOCORE_GENERIC_AUDIOSCENE
        spatializer->finalize(io);
#else
        spatializer->finalize(outputBuffers, numFrames);
#endif

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


