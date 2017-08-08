
#include <allosphere/al_AlloSphereApp.hpp>

using namespace al;


AudioRendererBaseNoState::~AudioRendererBaseNoState()
{
	io.stop();
}

void AudioRendererBaseNoState::initAudio()
{
	io.start();
}
