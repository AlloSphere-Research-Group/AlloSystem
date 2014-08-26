#ifndef AL_SIMULATOR_H
#define AL_SIMULATOR_H

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_OSC.hpp"

#define PORT_TO_DEVICE_SERVER (12000)
#define PORT_FROM_DEVICE_SERVER (PORT_TO_DEVICE_SERVER + 1)
#define DEVICE_SERVER_IP_ADDRESS "BOSSANOVA"

namespace al {

class Simulator : public osc::PacketHandler, public Main::Handler {
 public:
  Simulator(std::string name = "omniapp");
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

  const std::string& name() const { return mName; }
  Simulator& name(const std::string& v) {
    mName = v;
    return *this;
  }

  osc::Recv& oscRecv() { return mOSCRecv; }
  osc::Send& oscSend() { return mOSCSend; }

  virtual void onMessage(osc::Message& m);

  void sendHandshake();
  void sendDisconnect();

 protected:
  bool started;
  double time, lastTime;
  osc::Recv mOSCRecv;
  osc::Send mOSCSend;
  std::string mName;

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

  if (false /* connected to device server */) nav().step();

  lastTime = time;
  time = Main::get().realtime();
  step(time - lastTime);
}

inline void Simulator::onExit() { exit(); }

inline void Simulator::sendHandshake() {
  oscSend().send("/handshake", name(), oscRecv().port());
}

inline void Simulator::sendDisconnect() {
  oscSend().send("/disconnectApplication", name());
}

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
  } else if (m.addressPattern() == "/pose") {
    Vec3d v;
    Quatd q;
    m >> v.x;
    m >> v.y;
    m >> v.z;
    m >> q.x;
    m >> q.y;
    m >> q.z;
    m >> q.w;
    nav().pos(v);
    nav().quat(q);
  }
}

inline void Simulator::start() {
  if (oscSend().opened()) sendHandshake();
  Main::get().interval(1 / 60.0).add(*this).start();
}

inline void Simulator::stop() { Main::get().stop(); }

inline Simulator::~Simulator() { sendDisconnect(); }

inline Simulator::Simulator(std::string name)
    : mOSCRecv(PORT_FROM_DEVICE_SERVER),
      mNavControl(mNav),
      mOSCSend(PORT_TO_DEVICE_SERVER, DEVICE_SERVER_IP_ADDRESS) {
  mName = name;
  oscRecv().bufferSize(32000);
  oscRecv().handler(*this);
  sendHandshake();
  mNavSpeed = 1;
  mNavTurnSpeed = 0.02;
  nav().smooth(0.8);
}

}  // al::

#endif
