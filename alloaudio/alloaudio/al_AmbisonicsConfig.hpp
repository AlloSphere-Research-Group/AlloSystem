#ifndef AL_AMBISONICSCONFIGURATION_H
#define AL_AMBISONICSCONFIGURATION_H

#include <string>
#include <vector>

#include "allocore/sound/al_Speaker.hpp"

namespace al {

using namespace std;

/** \addtogroup alloaudio
 *  @{
 */

///
/// \brief The AmbisonicsConfig class encapsulates the configuration for
/// ambisonics decoding
///
class AmbisonicsConfig
{
public:
	AmbisonicsConfig(int order = 3, int dimensions = 3, int flavor = 1)
	    : mOrder(order), mDimensions(dimensions) , mFlavor(flavor)
	{}

	int order() const { return mOrder; }
	int dimensions() const { return mDimensions; }
	int flavor() const { return mFlavor; }

	void setOrder(int order) { mOrder = order; }
	void setDimensions(int dimensions) { mDimensions = dimensions; }
	void setFlavor(int flavor) { mFlavor = flavor; }

	string mDescription;
	int mVersion;

	string mDecChan_mask;
	int mDecfreq_bands;
	int mDecSpeakers;
	string mDecCoeff_scale;

	string mOptInput_scale;
	string mOptNfeff_comp;
	string mOptDelay_comp;
	string mOptLevel_comp;
	double mOptXover_freq;
	double mOptXover_ratio;

	SpeakerLayout mLayout;

	vector<double> mOrderGains;
	vector<double> mDecodeMatrix;
	int mMatrixColumns;

private:
	int mOrder;
	int mDimensions;
	int mFlavor;
};

/** @} */
}

#endif // AL_AMBISONICSCONFIGURATION_H
