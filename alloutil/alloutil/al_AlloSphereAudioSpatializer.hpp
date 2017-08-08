#ifndef AL_SPHERE_AUDIO_SPATIALIZER_H
#define AL_SPHERE_AUDIO_SPATIALIZER_H

#include "allocore/al_Allocore.hpp"
#include "alloutil/al_AlloSphereSpeakerLayout.hpp"

namespace al {

class AlloSphereAudioSpatializer {
 public:
  AlloSphereAudioSpatializer();
  virtual ~AlloSphereAudioSpatializer();

  virtual void onSound(AudioIOData& io) {}

  // void start();
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

inline void AlloSphereAudioSpatializer::initAudio(double audioRate, int audioBlockSize) {

  if(bossa) initAudio("AF12 x5b", audioRate, audioBlockSize, 60, 60);
  else if(audio) initAudio("ECHO X5", audioRate, audioBlockSize, 60, 60);
  else{
    // laptop..
    mAudioIO.callback = AppAudioCB;
    mAudioIO.user(this);
    mAudioIO.framesPerSecond(audioRate);
    mAudioIO.framesPerBuffer(audioBlockSize);
    mAudioIO.channelsOut(2);
    mAudioIO.channelsIn(1);
  }
}

inline void AlloSphereAudioSpatializer::initAudio(std::string devicename, double audioRate,
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

inline void AlloSphereAudioSpatializer::initSpatialization(){

  if( bossa || audio ){
    // This is an AlloSphere audio machine
    mSpeakerLayout = new AlloSphereSpeakerLayout();
  } else {
    // mSpeakerLayout = new HeadsetSpeakerLayout();
    mSpeakerLayout = new SpeakerLayout();
    mSpeakerLayout->addSpeaker(Speaker(0, 45, 0, 1.0, 1.0));
    mSpeakerLayout->addSpeaker(Speaker(1, -45, 0, 1.0, 1.0));
  }

  // XXX select which spatializer to use via arguments
  mSpatializer = new AmbisonicsSpatializer(*mSpeakerLayout,3,3);
  // mSpatializer = new Dbap(*mSpeakerLayout);
  // mSpatializer = new Vbap(*mSpeakerLayout);

  mScene = new AudioScene(mAudioIO.framesPerBuffer());
  mListener = mScene->createListener(mSpatializer);
}

inline void AlloSphereAudioSpatializer::AppAudioCB(AudioIOData& io) {
  AlloSphereAudioSpatializer& app = io.user<AlloSphereAudioSpatializer>();
  io.frame(0);
  app.onSound(io);
}

// inline void AlloSphereAudioSpatializer::start() {
//   mAudioIO.start();
//   Main::get().start();
// }

inline AlloSphereAudioSpatializer::~AlloSphereAudioSpatializer() {}

inline AlloSphereAudioSpatializer::AlloSphereAudioSpatializer() {
	std::string hostname = Socket::hostName();
	bossa = hostname == "bossanova";
	audio = hostname == "audio.10g";
	std::cout << "AlloSphereAudioSpatializer at host: " << hostname << std::endl;
	// initAudio();
}

}  // al
#endif
