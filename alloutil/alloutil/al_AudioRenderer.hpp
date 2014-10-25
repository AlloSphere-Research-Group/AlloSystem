#ifndef AL_AUDIO_RENDERER_H
#define AL_AUDIO_RENDERER_H

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_AlloSphereSpeakerLayout.hpp"

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
  void initSpatialization();

  AudioScene* scene(){ return mScene; }
  Listener* listener(){ return mListener; }

  const AudioIO& audioIO() const { return mAudioIO; }
  AudioIO& audioIO() { return mAudioIO; }

 protected:
  AudioIO mAudioIO;
  SpeakerLayout *mSpeakerLayout;
  Spatializer *mSpatializer;
  Listener *mListener;
  AudioScene *mScene;
  bool bossa, audio;
  static void AppAudioCB(AudioIOData& io);
};

inline void AudioRenderer::initAudio(double audioRate, int audioBlockSize) {
  
  if(bossa) initAudio("AF12 x5b", audioRate, audioBlockSize, 60, 60);
  else if(audio) initAudio("ECHOX5", audioRate, audioBlockSize, 60, 60);
  else{
    mAudioIO.callback = AppAudioCB;
    mAudioIO.user(this);
    mAudioIO.framesPerSecond(audioRate);
    mAudioIO.framesPerBuffer(audioBlockSize);
  }
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
  mAudioIO.callback = AppAudioCB;
  mAudioIO.user(this);
  mAudioIO.framesPerSecond(audioRate);
  mAudioIO.framesPerBuffer(audioBlockSize);
}

inline void AudioRenderer::initSpatialization(){

  if( bossa || audio ){
    // This is an AlloSphere audio machine
    mSpeakerLayout = new AlloSphereSpeakerLayout();
  } else {
    // Stereo Headphones.. XXX make a laptop layout instead
    mSpeakerLayout = new HeadsetSpeakerLayout();
  }

  // XXX select which spatializer to use via arguments
  mSpatializer = new AmbisonicsSpatializer(*mSpeakerLayout,3,3);
  
  mScene = new AudioScene(mAudioIO.framesPerBuffer());
  mListener = mScene->createListener(mSpatializer);
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

inline AudioRenderer::AudioRenderer() {
  char hostname[256];
  gethostname(hostname, 256);
  bossa = !strncmp(hostname,"bossanova",256);
  audio = !strncmp(hostname,"audio",256);
  // std::cout << "AudioRenderer at host: " << hostname << std::endl;
  initAudio();
}

}  // al
#endif
