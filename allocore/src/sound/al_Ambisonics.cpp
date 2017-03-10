#include <string.h>
#include "allocore/sound/al_Ambisonics.hpp"

#ifdef USE_GAMMA
	#include "scl.h"
	#define COS			gam::scl::cosT8
	#define SIN			gam::scl::sinT7
	#define WRAP(phase)	phase = gam::scl::wrapPhase(phase)
#else
	#include <math.h>
	#define COS			cos
	#define SIN			sin
	#define WRAP(phase)
#endif


namespace al{

static const double c1_sqrt2	= 0.707106781186548;
static const double c8_11		= 8./11.;
static const double c40_11		= 40./11.;


//// @see http://www.ai.sri.com/ajh/ambisonics/
//
//// the three decode types:
//double k_velocity  = 1.;
//double k_energy_2d = sqrt(1/2.);
//double k_energy_3d = sqrt(1/3.);
//double k_cardioid = 1/2.;
//
//// compute speaker decode matrix
//// k is decode type (velocity, energy, cardioid)
//// dim is 2 (2D) or 3 (3D)
//// pos is direction cosines (unit vector) of speaker
//// @see http://www.ai.sri.com/ajh/ambisonics/speaker_matrix.m
//void speaker_matrix(double k, int dim, Vec3d pos, double * weights) {
//
//}

// AmbiBase

AmbiBase::AmbiBase(int dim, int order)
:	mDim(dim), mOrder(0), mWeights(0)
{	this->order(order); }

AmbiBase::~AmbiBase(){
	delete[] mWeights;
}

void AmbiBase::order(int o){
	if(o != mOrder){
		mOrder = o;
		mChannels = orderToChannels(mDim, mOrder);
		resize(mWeights, channels());
		onChannelsChange();
	}
}

int AmbiBase::channelsToUniformOrder(int channels){
	// M = floor(sqrt(N) - 1)
	return (int)(sqrt((double)channels) - 1);
}


void AmbiBase::encodeWeightsFuMa(float * ws, int dim, int order, float x, float y, float z){
	*ws++ = c1_sqrt2;								// W = 1/sqrt(2)

	if(order > 0){
		float x2 = x * x;
		float y2 = y * y;

		*ws++ = x;									// X = cos(A)cos(E)
		*ws++ = y;									// Y = sin(A)cos(E)

		if(order > 1){
			x2 = x*x;
			y2 = y*y;

			*ws++ = x2 - y2;						// U = cos(2A)cos2(E) = xx-yy
			*ws++ = 2.f * x * y;					// V = sin(2A)cos2(E) = 2xy

			if(order > 2){
				*ws++ = x * (x2 - 3.f * y2);		// P = cos(3A)cos3(E) = X(X2-3Y2)
				*ws++ = y * (y2 - 3.f * x2);		// Q = sin(3A)cos3(E) = Y(3X2-Y2)
			}
		}

		if(dim == 3){
			*ws++ = z;								// Z = sin(E)

			if(order > 1){
				float z2 = z*z;

				*ws++ = 2.f * z * x;				// S = cos(A)sin(2E) = 2zx
				*ws++ = 2.f * z * y;				// T = sin(A)sin(2E) = 2yz
				*ws++ = (1.5f * z2) - 0.5f;			// R = 1.5sin2(E)-0.5 = 1.5zz-0.5

				if(order > 2){
					float pre = c40_11 * z2 - c8_11;

					*ws++ = z * (x2-y2) * 0.5f;		// N = cos(2A)sin(E)cos2(E) = Z(X2-Y2)/2
					*ws++ = x * y * z;				// O = sin(2A)sin(E)cos2(E) = XYZ
					*ws++ = pre * x;				// L = 8cos(A)cos(E)(5sin2(E) - 1)/11 = 8X(5Z2-1)/11
					*ws++ = pre * y;				// M = 8sin(A)cos(E)(5sin2(E) - 1)/11 = 8Y(5Z2-1)/11
					*ws   = z * (2.5f * z2 - 1.5f);	// K = sin(E)(5sin2(E) - 3)/2 = Z(5Z2-3)/2
				}
			}
		}
	}
}


void AmbiBase::encodeWeightsFuMa(float * ws, int dim, int order, float az, float el){
	WRAP(az);
	WRAP(el);
	float cosel = COS(el);
	float x = COS(az) * cosel;
	float y = SIN(az) * cosel;
	float z = dim>=3 ? SIN(el) : 0;
	encodeWeightsFuMa(ws, dim, order, x,y,z);
}

// [x, y, z] is the direction unit vector
void AmbiBase::encodeWeightsFuMa16(float * ws, float x, float y, float z){
	float x2 = x * x;
	float y2 = y * y;
	float z2 = z * z;
	float pre = c40_11 * z2 - c8_11;

	ws[ 0] = c1_sqrt2;					// W channel, shouldn't it be already defined?
	ws[ 1] = x;							// X = cos(A)cos(E)
	ws[ 2] = y;							// Y = sin(A)cos(E)
	ws[ 3] = z;							// Z = sin(E)
	ws[ 4] = x2 - y2;					// U = cos(2A)cos2(E) = xx-yy
	ws[ 5] = 2.f * x * y;				// V = sin(2A)cos2(E) = 2xy
	ws[ 6] = 2.f * z * x;				// S = cos(A)sin(2E) = 2zx
	ws[ 7] = 2.f * z * y;				// T = sin(A)sin(2E) = 2yz
	ws[ 8] = 1.5f * z2 - 0.5f;			// R = 1.5sin2(E)-0.5 = 1.5zz-0.5
	ws[ 9] = x * (x2 - 3.f * y2);		// P = cos(3A)cos3(E) = X(X2-3Y2)
	ws[10] = y * (y2 - 3.f * x2);		// Q = sin(3A)cos3(E) = Y(3X2-Y2)
	ws[11] = z * (x2 - y2) * 0.5f;		// N = cos(2A)sin(E)cos2(E) = Z(X2-Y2)/2
	ws[12] = x * y * z;					// O = sin(2A)sin(E)cos2(E) = XYZ
	ws[13] = pre * x;					// L = 8cos(A)cos(E)(5sin2(E) - 1)/11 = 8X(5Z2-1)/11
	ws[14] = pre * y;					// M = 8sin(A)cos(E)(5sin2(E) - 1)/11 = 8Y(5Z2-1)/11
	ws[15] = z * (2.5f * z2 - 1.5f);	// K = sin(E)(5sin2(E) - 3)/2 = Z(5Z2-3)/2
}


void AmbiBase::encodeWeightsFuMa16(float * ws, float az, float el){
	WRAP(az);
	WRAP(el);
	float cosel = COS(el);
	float x = COS(az) * cosel;
	float y = SIN(az) * cosel;
	float z = SIN(el);
	encodeWeightsFuMa16(ws, x,y,z);
}




// AmbiDecode

float AmbiDecode::flavorWeights[4][5][5] = {
	{	// none:
		{1,		1,		1,		1,		1    }, // n = 0, M = 0, 1, 2, 3, 4
		{0,		1,		1,		1,		1    }, // n = 1, M = 0, 1, 2, 3, 4
		{0,		0,		1,		1,		1    }, // n = 2, M = 0, 1, 2, 3, 4
		{0,		0,		0,		1,		1    }, // n = 3, M = 0, 1, 2, 3, 4
		{0,		0,		0,		0,		1    }  // n = 4, M = 0, 1, 2, 3, 4
	},{	// default:
		{1,		0.707,	0.707,	0.707,	0.707},	// n = 0, M = 0, 1, 2, 3, 4
		{0,		1    ,	0.75 ,	0.75 ,	0.75 },	// n = 1, M = 0, 1, 2, 3, 4
		{0,		0    ,	0.5  ,	0.5  ,	0.5  },	// n = 2, M = 0, 1, 2, 3, 4
		{0,		0    ,	0    ,	0.3  ,	0.3  },	// n = 3, M = 0, 1, 2, 3, 4
		{0,		0    ,	0    ,	0    ,	0.1  } 	// n = 4, M = 0, 1, 2, 3, 4
	},{	// in phase
		{1,		1,		1,		1,		1    },	// n = 0, M = 0, 1, 2, 3, 4
		{0,		0.333,	0.5,	0.6,	0.667},	// n = 1, M = 0, 1, 2, 3, 4
		{0,		0,		0.1,	0.2,	0.286},	// n = 2, M = 0, 1, 2, 3, 4
		{0,		0,		0,		0.029,	0.071},	// n = 3, M = 0, 1, 2, 3, 4
		{0,		0,		0,		0,		0.008}	// n = 4, M = 0, 1, 2, 3, 4
	},{	// max-rE
		{1,		1,		1,		1,		1    },	// n = 0, M = 0, 1, 2, 3, 4
		{0,		0.577,	0.775,	0.861,	0.906},	// n = 1, M = 0, 1, 2, 3, 4
		{0,		0,		0.4,	0.612,	0.732},	// n = 2, M = 0, 1, 2, 3, 4
		{0,		0,		0,		0.305,	0.501},	// n = 3, M = 0, 1, 2, 3, 4
		{0,		0,		0,		0,		0.246}	// n = 4, M = 0, 1, 2, 3, 4
	}
};

AmbiDecode::AmbiDecode(int dim, int order, int numSpeakers, int flav)
	: AmbiBase(dim, order),
	mNumSpeakers(0), mDecodeMatrix(0), mSpeakers(NULL)
{
	resizeArrays(channels(), numSpeakers);
	flavor(flav);
}

AmbiDecode::~AmbiDecode(){
	delete[] mDecodeMatrix;
	//delete[] mSpeakers; // listener now owns speakers and will delete them
}

void AmbiDecode::decode(float * dec, const float * ambi, int numDecFrames) const {

	// iterate speakers
	for(int s=0; s<numSpeakers(); ++s){
		// skip zero-amp speakers:
		if ((*mSpeakers)[s].gain != 0.) {
			float * out = dec + (*mSpeakers)[s].deviceChannel * numDecFrames;

			// iterate ambi channels
			for(int c=0; c<channels(); ++c){
				const float * in = ambi + c * numDecFrames;
				float w = decodeWeight(s, c);
				for(int i=0; i<numDecFrames; ++i) out[i] += in[i] * w;
			}
		}
	}
}


void AmbiDecode::flavor(int type){
	if(type < 4){
		mFlavor = type;
		const int No = sizeof(mWOrder)/sizeof(mWOrder[0]);
		for(int i=0; i<No; ++i) mWOrder[i] = flavorWeights[flavor()][i][order()];
		updateChanWeights();
	}
}

void AmbiDecode::numSpeakers(int num){
	resizeArrays(channels(), num);
}

//void AmbiDecode::zero(){ memset(mFrame, 0, channels()*sizeof(float)); }

void AmbiDecode::setSpeakerRadians(int index, int deviceChannel, float az, float el, float amp){
	if(index >= numSpeakers()){
		numSpeakers(index);	// grow adaptively
	}

	(*mSpeakers)[index].azimuth = az;
	(*mSpeakers)[index].elevation = el;
	(*mSpeakers)[index].deviceChannel = deviceChannel;
	(*mSpeakers)[index].gain = amp;

	// update encoding weights
	encodeWeightsFuMa(mDecodeMatrix + index * channels(), mDim, mOrder, az, el);
	for (int i=0; i<channels(); i++) {
		mDecodeMatrix[index * channels() + i] *= amp;
	}
}

void AmbiDecode::setSpeaker(int index, int deviceChannel, float az, float el, float amp){
	setSpeakerRadians(index, deviceChannel, az * float(0.01745329252), el * float(0.01745329252), amp);
}

void AmbiDecode::setSpeakers(Speakers *spkrs) {
	mSpeakers = spkrs;
	for (unsigned i = 0; i < mSpeakers->size(); i++) {
		Speaker &spkr = mSpeakers->at(i);
		setSpeaker(i, spkr.deviceChannel, spkr.azimuth, spkr.elevation, spkr.gain);
	}
}

void AmbiDecode::updateChanWeights(){
	float * wc = mWeights;
	*wc++ = mWOrder[0];

	if(mOrder > 0){
		*wc++ = mWOrder[1];				// X
		*wc++ = mWOrder[1];				// Y
		if (mOrder > 1) {
			*wc++ = mWOrder[2];			// U
			*wc++ = mWOrder[2];			// V
			if (mOrder > 2) {
				*wc++ = mWOrder[3];		// P
				*wc++ = mWOrder[3];		// Q
			}
		}

		if(3 == mDim){
			*wc++ = mWOrder[1];			// Z
			if (mOrder > 1) {
				*wc++ = mWOrder[2];		// S
				*wc++ = mWOrder[2];		// T
				*wc++ = mWOrder[2];		// R
				if (mOrder > 2) {
					*wc++ = mWOrder[3];	// N
					*wc++ = mWOrder[3];	// O
					*wc++ = mWOrder[3];	// L
					*wc++ = mWOrder[3];	// M
					*wc   = mWOrder[3];	// K
				}
			}
		}
	}
}

void AmbiDecode::resizeArrays(int numChannels, int numSpeakers){

	int oldSize = channels() * mNumSpeakers;
	int newSize = numChannels * numSpeakers;

	if(oldSize != newSize){

		resize(mDecodeMatrix, newSize);
		//resize(mFrame, newSize);

		// resize number of speakers (?)
		if(numSpeakers != mNumSpeakers){
			mNumSpeakers = numSpeakers;

            //resize(mSpeakers, numSpeakers);
		}

//		// recompute decode matrix weights
//		for(int i=0; i<numSpeakers; i++){
//			setSpeaker(i, azimuths()[i], elevations()[i]);
//		}

		updateChanWeights();
	}

	mChannels = numChannels;
}

void AmbiDecode::onChannelsChange(){
	resizeArrays(channels(), mNumSpeakers);
}

void AmbiDecode::print(FILE * fp, const char * append) const {
//	AmbiBase::print(stdout, ", ");
//	fprintf(fp, "s:%3d%s", mNumSpeakers, append);
}


AmbisonicsSpatializer::AmbisonicsSpatializer(
	SpeakerLayout &sl, int dim, int order, int flavor
)
	:	Spatializer(sl), mDecoder(dim, order, sl.numSpeakers(), flavor), mEncoder(dim,order),
	  mListener(NULL),  mNumFrames(0)
{
	setSpeakerLayout(sl);
};

void AmbisonicsSpatializer::zeroAmbi(){
	memset(ambiChans(), 0, mAmbiDomainChannels.size()*sizeof(ambiChans()[0]));
}

void AmbisonicsSpatializer::compile(Listener& l){
    mListener = &l;
}

void AmbisonicsSpatializer::numFrames(int v){

	if(mAmbiDomainChannels.size() != (unsigned long)(mDecoder.channels() * v)){
		mAmbiDomainChannels.resize(mDecoder.channels() * v);
	}
}

void AmbisonicsSpatializer::numSpeakers(int num){
    mDecoder.numSpeakers(num);
}

void AmbisonicsSpatializer::setSpeakerLayout(const SpeakerLayout& sl){
	mDecoder.setSpeakers(&mSpeakers);

	mSpeakers.clear();
	unsigned numSpeakers = sl.speakers().size();
	for(unsigned i=0;i<numSpeakers;++i){
		mSpeakers.push_back(sl.speakers()[i]);

        mDecoder.setSpeakerRadians(
			i,
			mSpeakers[i].deviceChannel,
			mSpeakers[i].azimuth,
			mSpeakers[i].elevation,
			mSpeakers[i].gain
		);
	}
}

void AmbisonicsSpatializer::prepare(){
    zeroAmbi();
}

void AmbisonicsSpatializer::renderBuffer(AudioIOData& io,
                          const Pose& listeningPose,
                          const float *samples,
                          const int& numFrames
                          )
{
//	// compute azimuth & elevation of relative position in current listener's coordinate frame:
//	Vec3d urel(relpos);
//	urel.normalize();	// unit vector in axis listener->source

//	/* project into listener's coordinate frame:
//		Vec3d axis;
//		l.mQuatHistory[i].toVectorX(axis);
//		double rr = urel.dot(axis);
//		l.mQuatHistory[i].toVectorY(axis);
//		double ru = urel.dot(axis);
//		l.mQuatHistory[i].toVectorZ(axis);
//		double rf = urel.dot(axis);
//		//*/
	Vec3d direction = listeningPose.vec();

	//Rotate vector according to listener-rotation
	Quatd srcRot = listeningPose.quat();
	direction = srcRot.rotate(direction);
	direction = Vec4d(direction.x, direction.z, direction.y);
	for(int i = 0; i < numFrames; i++){
//		// cheaper:
//		Vec3d direction = mListener->quatHistory()[i].rotateTransposed(urel);

//		//mEncoder.direction(azimuth, elevation);
//		//mEncoder.direction(-rf, -rr, ru);
		mEncoder.direction(-direction[2], -direction[0], direction[1]);
		mEncoder.encode(ambiChans(), numFrames, i, samples[i]);
	}
}


void AmbisonicsSpatializer::renderSample(AudioIOData& io, const Pose& listeningPose,
                          const float& sample,
                          const int& frameIndex)
{
	// compute azimuth & elevation of relative position in current listener's coordinate frame:
//    Vec3d urel(relpos);
//    urel.normalize();	// unit vector in axis listener->source
//    // project into listener's coordinate frame:
//    //						Vec3d axis;
//    //						l.mQuatHistory[i].toVectorX(axis);
//    //						double rr = urel.dot(axis);
//    //						l.mQuatHistory[i].toVectorY(axis);
//    //						double ru = urel.dot(axis);
//    //						l.mQuatHistory[i].toVectorZ(axis);
//    //						double rf = urel.dot(axis);

//    // cheaper:
//    Vec3d direction = mListener->quatHistory()[frameIndex].rotateTransposed(urel);
	Vec3d direction = listeningPose.vec();

	//Rotate vector according to listener-rotation
	Quatd srcRot = listeningPose.quat();
	direction = srcRot.rotate(direction);
	direction = Vec4d(direction.x, direction.z, direction.y);

    //mEncoder.direction(azimuth, elevation);
    //mEncoder.direction(-rf, -rr, ru);
//    mEncoder.direction(-direction[2], -direction[0], direction[1]);
	mEncoder.direction(direction[0], direction[1], direction[2]);
    mEncoder.encode(ambiChans(), io.framesPerBuffer(), frameIndex, sample);
}


void AmbisonicsSpatializer::finalize(AudioIOData& io){
	//previously done in render method of audioscene

	float *outs = &io.out(0,0);//io.outBuffer();
	int numFrames = io.framesPerBuffer();

	mDecoder.decode(outs, ambiChans(), numFrames);
}

} // al::

#undef WRAP
#undef COS
#undef SIN
