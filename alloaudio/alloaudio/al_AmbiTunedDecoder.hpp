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
	AmbiTunedDecoder(string configFile = "Allosphere.ambdec");

private:
	AmbisonicsConfig mAmbisonicsConfig;
};

/** @} */
}

#endif // AL_AMBITUNEDDECODER_H
