// MAT201B
// Fall 2015
// Author(s): Karl Yerkes
//
// Adapted from allocore/examples/audioscene.cpp by Ryan McGee
//
#include "allocore/al_Allocore.hpp"
#include "allocore/sound/al_Vbap.hpp"
#include "allocore/sound/al_Dbap.hpp"
#include "allocore/sound/al_Ambisonics.hpp"
#include "Gamma/Oscillator.h"

using namespace al;
using namespace std;
using namespace gam;

#define BLOCK_SIZE (256)
#define N_AGENTS (5)

struct Agent : SoundSource {
  Mesh body;
  float visualLoudnessMeasure;
  Accum<> timer;
  SineD<> sined;
  Color color;
  unsigned sampleCount;
  float radius;
  float rate;

  Agent() {
    sampleCount = 0;
    cout << "making agent..." << endl;
    radius = rnd::uniform(0.5, 6.0);
    cout << "  radius:" << radius << endl;
    rate = rnd::uniform(0.1, 1.0);
    cout << "  rate:" << rate << endl;
    float freq = rnd::uniform(100.0, 700.0);
    cout << "  freq:" << freq << endl;
    color = HSV((freq - 100) / 600, 1, 1);
    sined.set(freq, 0.8, 1.2);
    timer.freq(rnd::uniform(0.5, 5.0));
    cout << "  timer:" << timer.freq() << endl;
    addSphere(body);
  }

  void onDraw(Graphics& g) {
    g.pushMatrix();
    g.color(HSV(visualLoudnessMeasure, 1, visualLoudnessMeasure));
    g.translate(SoundSource::pos());
    g.scale(0.2);
    g.draw(body);
    g.popMatrix();
  }

  void onSound(AudioIOData& io) {
    cout << timer.freq() << endl;
    visualLoudnessMeasure = 0;
    for (int i = 0; i < io.framesPerBuffer(); i++) {
      float secondsCounter = ((float)sampleCount / io.fps());
      // Create an oscillating trajectory for the sound source
      //
      float x = radius * sin(rate * secondsCounter * M_PI);
      float z = radius * cos(rate * secondsCounter * M_PI);
      SoundSource::pos(x, 0, z);

      // Generate a test signal with decaying envelope
      //
      if (timer()) sined.reset();
      float sample = sined();

      // record the "loudest" sample from this block
      //
      if (abs(sample) > visualLoudnessMeasure)
        visualLoudnessMeasure = abs(sample);

      // Write sample to the source
      //
      SoundSource::writeSample(sample);

      ++sampleCount;
    }
  }
};

struct MyApp : App {
  // Create a speaker layout
  //
  SpeakerLayout speakerLayout = HeadsetSpeakerLayout();

  // Create a panner: DBAP, VBAP, Ambisonics, or Stereo
  //
  StereoPanner* panner;
  // Dbap* panner = new Dbap(speakerLayout);
  // Vbap* panner = new Vbap(speakerLayout);
  // AmbisonicsSpatializer* panner =
  //   new AmbisonicsSpatializer(speakerLayout, 2, 1); // dimension and order

  // Create listener(s) (at least one)
  //
  Listener* listener;

  // Create a Sound Source
  //
  Agent agent[N_AGENTS];

  // Create an audio scene with single argument for frames per buffer
  //
  AudioScene scene;
  MyApp() : scene(BLOCK_SIZE) {
    panner = new StereoPanner(speakerLayout);

    // Initialize the listener(s) with their individual speaker layout and
    // panner
    //
    listener = scene.createListener(panner);

    // Add the sound source to the scene
    //
    for (int i = 0; i < N_AGENTS; i++) scene.addSource(agent[i]);

    // Optionally, disable per sample processing to save CPU. Recommended to
    // disable Doppler in this case as well.
    //
    scene.usePerSampleProcessing(false);
    for (int i = 0; i < N_AGENTS; i++) agent[i].dopplerType(DOPPLER_NONE);

    // update the listener's speaker layout and panner call this to dynamically
    // change a listener's speaker layout and panner.
    //
    listener->compile();

    // Print out relevant panner info (ex. number of triplets found for VBAP)
    //
    panner->print();

    // Create an audio IO for the audio scene, specifying blocksize
    //
    initAudio(44100, BLOCK_SIZE);

    // set initial listener position based on nav()
    //
    listener->pose(nav());

    initWindow();
  }

  virtual void onDraw(Graphics& g) {
    for (int i = 0; i < N_AGENTS; i++) agent[i].onDraw(g);
  }

  // Create an audio callback function for the source and scene
  //
  virtual void onSound(AudioIOData& io) {
    gam::Sync::master().spu(audioIO().fps());
    for (int i = 0; i < N_AGENTS; i++) {
      io.frame(0);
      agent[i].onSound(io);
    }

    // render this scene buffer (renders as many frames as specified at
    // initialization)
    //
    //    listener->pose(nav());
    io.frame(0);
    scene.render(io);
  }
};

int main() { MyApp().start(); }
