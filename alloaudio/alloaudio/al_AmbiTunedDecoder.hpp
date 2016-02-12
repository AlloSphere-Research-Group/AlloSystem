#ifndef AL_AMBITUNEDDECODER_H
#define AL_AMBITUNEDDECODER_H

#include "allocore/sound/al_Ambisonics.hpp"
#include "alloaudio/al_AmbisonicsConfig.hpp"

namespace al {

using namespace std;

/** \addtogroup alloaudio
 *  @{
 */

///
/// \brief The AmbiTunedDecoder class can do ambisonics decoding according to an optimized set of coefficients
///
/// The coefficients and speaker data are read from  an ambdec configuration
/// file provided in the constructor
class AmbiTunedDecoder : public AmbiDecode
{
public:
	AmbiTunedDecoder(int dim = 3, int order = 3, int numSpeakers = 8, int flavor=1) :
	    AmbiDecode(dim, order, numSpeakers, flavor) { setConfiguration();}
	void setConfiguration(string configFile = "Allosphere.ambdec");

private:
	AmbisonicsConfig mAmbisonicsConfig;
};


class AmbisonicsTunedSpatializer : public AmbisonicsBaseSpatializer<AmbiTunedDecoder>
{
public:
	AmbisonicsTunedSpatializer(SpeakerLayout sl = StereoSpeakerLayout(), int dim = 3, int order = 3, int flavor=1)
	    :AmbisonicsBaseSpatializer(sl, dim, order, flavor) {}
};

/** @} */
}

#endif // AL_AMBITUNEDDECODER_H
