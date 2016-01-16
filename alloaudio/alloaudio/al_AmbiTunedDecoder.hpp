#ifndef AL_AMBITUNEDDECODER_H
#define AL_AMBITUNEDDECODER_H

#include "allocore/sound/al_Ambisonics.hpp"


namespace al {

using namespace std;

class AmbiTunedDecoder : public AmbiDecode
{
public:
	AmbiTunedDecoder(string configFile);

private:
};

/** @} */
}

#endif // AL_AMBITUNEDDECODER_H
