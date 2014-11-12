#ifndef AL_SIMULATOR_H
#define AL_SIMULATOR_H

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_OSC.hpp"

namespace al {

class Simulator : public osc::PacketHandler, public Main::Handler {
 public:
  Simulator(const char* deviceServerAddress = "127.0.0.1", int port = 12001,
            int deviceServerPort = 12000);

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

  virtual const char* name();
  virtual const char* deviceServerConfig();

  osc::Recv& oscRecv() { return mOSCRecv; }
  osc::Send& oscSend() { return mOSCSend; }

  virtual void onMessage(osc::Message& m);

  static bool gr01(){
    char hostname[256];
    gethostname(hostname, 256);
    return !strncmp(hostname,"gr01",256);
  }

  static const char* defaultBroadcastIP(){
    if(gr01()) return "192.168.10.255";
    else return "127.0.0.1";
  }

  static const char* defaultInterfaceServerIP(){
    if(gr01()) return "192.168.0.74"; //interface mac mini
    else return "127.0.0.1";
  }

 protected:
  bool started;
  double time, lastTime;
  osc::Recv mOSCRecv;
  osc::Send mOSCSend;

  Nav mNav;
  NavInputControl mNavControl;
  StandardWindowKeyControls mStdControls;

  double mNavSpeed, mNavTurnSpeed;
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

inline void Simulator::onMessage(osc::Message& m) {

  float x;
  if (m.addressPattern() == "/mx") {
    m >> x;
    nav().moveR(-x * mNavSpeed);
  } else if (m.addressPattern() == "/my") {
    m >> x;
    nav().moveU(x * mNavSpeed);
  } else if (m.addressPattern() == "/mz") {
    m >> x;
    nav().moveF(x * mNavSpeed);
  } else if (m.addressPattern() == "/tx") {
    m >> x;
    nav().spinR(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/ty") {
    m >> x;
    nav().spinU(x * mNavTurnSpeed);
  } else if (m.addressPattern() == "/tz") {
    m >> x;
    nav().spinF(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/home") {
    nav().home();
  } else if (m.addressPattern() == "/halt") {
    nav().halt();
  } else {
  }

  // This allows the graphics renderer (or whatever) to overwrite navigation
  // data, so you can navigate from the graphics window when using a laptop.
  //
  if (m.addressPattern() == "/pose") {
    Pose pose;
    m >> pose.pos().x;
    m >> pose.pos().y;
    m >> pose.pos().z;
    m >> pose.quat().x;
    m >> pose.quat().y;
    m >> pose.quat().z;
    m >> pose.quat().w;
    //pose.print();
    nav().set(pose);
  }
}

inline void Simulator::start() {
  if (oscSend().opened())
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   deviceServerConfig());
  Main::get().interval(1 / 60.0).add(*this).start();
}

inline void Simulator::stop() { Main::get().stop(); }

inline Simulator::~Simulator() {
  oscSend().send("/interface/disconnectApplication", name());
}

Simulator::Simulator(const char* deviceServerAddress, int port,
                     int deviceServerPort)
    : mOSCRecv(port),
      mNavControl(mNav),
      mOSCSend(deviceServerPort, deviceServerAddress) {

  started = false;

  oscRecv().bufferSize(32000);
  oscRecv().handler(*this);

  mNavSpeed = 1;
  mNavTurnSpeed = 0.02;
  nav().smooth(0.8);
}

inline const char* Simulator::name() { return "default_no_name"; }

inline const char* Simulator::deviceServerConfig() {
  return R"(
      app = {
        name : 'default_no_name',
        receivers :[ {type : 'OSC', port : 12001}, ],
        inputs: {
          mx: {min: 0, max: 0.1, },
          my: {min: 0, max: 0.1, },
          mz: {min: 0, max: 0.1, },
          tx: {min: 0, max: 1, },
          ty: {min: 0, max: 1, },
          tz: {min: 0, max: 1, },
          home: {min: 0, max: 1, },
          halt: {min: 0, max: 1, },
        },
        mappings: [
          { input: { io:'keypress', name:'`' }, output:{ io:'blob', name:'home' } },
          { input: { io:'keypress', name:'w' }, output:{ io:'blob', name:'mx'}, },
        ]

      }
  )";
}

}  // al::

#endif
