#ifndef AL_AUDIO_RENDERER_H
#define AL_AUDIO_RENDERER_H

#include "allocore/al_Allocore.hpp"

namespace al {

class AudioRenderer {
 public:
  AudioRenderer();
  virtual ~AudioRenderer();

  virtual void onSound(AudioIOData& io) {}

  void start();
  void initAudio(double audioRate = 44100, int audioBlockSize = 256);
  void initAudio(std::string devicename, double audioRate, int audioBlockSize,
                 int audioInputs, int audioOutputs);

  const AudioIO& audioIO() const { return mAudioIO; }
  AudioIO& audioIO() { return mAudioIO; }

 protected:
  AudioIO mAudioIO;
  static void AppAudioCB(AudioIOData& io);
};

inline void AudioRenderer::initAudio(double audioRate, int audioBlockSize) {
  mAudioIO.callback = AppAudioCB;
  mAudioIO.user(this);
  mAudioIO.framesPerSecond(audioRate);
  mAudioIO.framesPerBuffer(audioBlockSize);
}

inline void AudioRenderer::initAudio(std::string devicename, double audioRate,
                                     int audioBlockSize, int audioInputs,
                                     int audioOutputs) {
  AudioDevice indev(devicename, AudioDevice::INPUT);
  AudioDevice outdev(devicename, AudioDevice::OUTPUT);
  indev.print();
  outdev.print();
  mAudioIO.deviceIn(indev);
  mAudioIO.deviceOut(outdev);
  mAudioIO.channelsOut(audioOutputs);
  mAudioIO.channelsIn(audioInputs);
  initAudio(audioRate, audioBlockSize);
}

inline void AudioRenderer::AppAudioCB(AudioIOData& io) {
  AudioRenderer& app = io.user<AudioRenderer>();
  io.frame(0);
  app.onSound(io);
}

inline void AudioRenderer::start() {
  mAudioIO.start();
  Main::get().start();
}

inline AudioRenderer::~AudioRenderer() {}

inline AudioRenderer::AudioRenderer() { initAudio(); }

}  // al
#endif
