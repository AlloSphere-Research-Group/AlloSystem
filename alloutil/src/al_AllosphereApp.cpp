
#include <alloutil/al_AllosphereApp.hpp>

using namespace al;


AudioRendererBaseNoState::~AudioRendererBaseNoState()
{
	io.stop();
}

void AudioRendererBaseNoState::initAudio()
{
	io.start();
}
