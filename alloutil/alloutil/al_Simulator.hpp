#ifndef AL_SIMULATOR_H
#define AL_SIMULATOR_H

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_OSC.hpp"
#include "alloutil/al_InterfaceServerClient.hpp"

namespace al {

class Simulator : public InterfaceServerClient, public Main::Handler {
 public:
  Simulator(const char* deviceServerAddress="127.0.0.1", int port=12001,
                     int deviceServerPort=12000);

  virtual ~Simulator();

  virtual void start();
  virtual void stop();

  virtual void init() = 0;
  virtual void step(double dt) = 0;
  virtual void exit() = 0;

  virtual void onTick();
  virtual void onExit();

  const Nav& nav() const { return mNav; }
  Nav& nav() { return mNav; }

  // virtual void onMessage(osc::Message& m);

  static bool sim(){
    char hostname[256];
    gethostname(hostname, 256);
    return (!strncmp(hostname,"gr01",256) || !strncmp(hostname,"audio.10g",256));
  }

  static const char* defaultBroadcastIP(){
    if(sim()) return "192.168.10.255";
    else return "127.0.0.1";
  }

  static const char* defaultInterfaceServerIP(){
    if(sim()) return "192.168.0.234"; //"192.168.0.74"; //interface mac mini
    else return "127.0.0.1";
  }

 protected:
  bool started;
  double time, lastTime;

  Nav mNav;
  // NavInputControl mNavControl;
  // StandardWindowKeyControls mStdControls;
  // double mNavSpeed, mNavTurnSpeed;
};

inline void Simulator::onTick() {
  if (!started) {
    started = true;
    time = Main::get().realtime();
    init();
  }

  while (oscRecv().recv()) {
  }

  lastTime = time;
  time = Main::get().realtime();
  nav().step(time - lastTime);
  step(time - lastTime);
}

inline void Simulator::onExit() { exit(); }

// inline void Simulator::onMessage(osc::Message& m) {

//   // This allows the graphics renderer (or whatever) to overwrite navigation
//   // data, so you can navigate from the graphics window when using a laptop.
//   //
//   // cout << "Simulator receive: " << m.addressPattern() << endl;
//   // if (m.addressPattern() == "/pose") {
//   //   Pose pose;
//   //   m >> pose.pos().x;
//   //   m >> pose.pos().y;
//   //   m >> pose.pos().z;
//   //   m >> pose.quat().x;
//   //   m >> pose.quat().y;
//   //   m >> pose.quat().z;
//   //   m >> pose.quat().w;
//   //   //pose.print();
//   //   nav().set(pose);
//   // }

//   InterfaceServerClient::onMessage(m);

//   // nav().print();
// }

inline void Simulator::start() {
  InterfaceServerClient::connect();
  Main::get().interval(1 / 60.0).add(*this).start();
}

inline void Simulator::stop() { cout << "Simulator stopped." <<endl; Main::get().stop(); }

inline Simulator::~Simulator() {
  InterfaceServerClient::disconnect();
  // oscSend().send("/interface/disconnectApplication", name());
}

Simulator::Simulator(const char* deviceServerAddress, int port,
                     int deviceServerPort)
    : //mNavControl(mNav),
      InterfaceServerClient(deviceServerAddress,port,deviceServerPort) {

  started = false;
  nav().smooth(0.8);
  InterfaceServerClient::nav(nav());
}


}  // al::

#endif
