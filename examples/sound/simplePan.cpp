/*
 By Matt Wright 9/21/11
 Very simple equal-amplitude panner using App and gamma.

*/


#include "alloutil/al_App.hpp"
#include "Gamma/Oscillator.h"
using namespace al;


struct PanApp : App {
  gam::TableSine<> sine;
  gam::TableSine<> pansine;
  

  PanApp():
    sine(440.0f),
    pansine(1.0f)
  {}

  virtual void onSound(AudioIOData& io) {
    gam::Sync::master().spu(audioIO().fps());

    while(io()){
      float p = (pansine() + 1.)*0.5; // 0=left, 1=right
      float s = sine();

      io.out(0) = s * (1-p);
      io.out(1) = s * p;
    }
  }
};

int main() {
  PanApp app;

  app.initWindow(
    Window::Dim(600,400), // window dimensions
    "Dot", // name on window
    30.0f, // frames per second
    Window::DEFAULT_BUF | Window::MULTISAMPLE // makes things look nice
  );

  app.initAudio(
    44100, // sample rate, in Hz
    128, // block size
    2, // number of output channels to open
    0 // number of input channels to open
  );

  app.start();
}
