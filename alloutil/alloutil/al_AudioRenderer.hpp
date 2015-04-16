#ifndef AL_AUDIO_RENDERER_H
#define AL_AUDIO_RENDERER_H

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_AlloSphereAudioSpatializer.hpp"

namespace al {

class AudioRenderer : public AlloSphereAudioSpatializer {
 public:
  AudioRenderer();
  virtual ~AudioRenderer();

  virtual void onSound(AudioIOData& io) {}

  void start();
};

inline void AudioRenderer::start() {
  mAudioIO.start();
  Main::get().start();
}

inline AudioRenderer::~AudioRenderer() {}

inline AudioRenderer::AudioRenderer() {
  initAudio();
}

}  // al
#endif
