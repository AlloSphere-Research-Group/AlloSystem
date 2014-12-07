#ifndef AL_ISCLIENT_H
#define AL_ISCLIENT_H

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_OSC.hpp"

namespace al {

class InterfaceServerClient : public osc::PacketHandler {
 public:
  InterfaceServerClient(const char* serverAddress = "127.0.0.1", int port = 12001,
            int serverPort = 12000);

  virtual ~InterfaceServerClient();

  // application specific name and config
  virtual const char* name();
  virtual const char* interfaceServerConfig();

  // common config used for all interface server apps
  const char* defaultConfig();

  virtual void onMessage(osc::Message& m);

  void connect();
  void disconnect();

  osc::Recv& oscRecv() { return mOSCRecv; }
  osc::Send& oscSend() { return mOSCSend; }

  // default nav and lens, be sure to set these
  // if using a different nav / lens
  Lens& lens() { return *mLens; }
  void lens(Lens& l){ mLens=&l; }

  Nav& nav() { return *mNav; }
  void nav(Nav& n){ mNav=&n; }

 protected:
  osc::Recv mOSCRecv;
  osc::Send mOSCSend;
  Lens* mLens;
  Nav* mNav;
  double mNavSpeed, mNavTurnSpeed;

};

inline void InterfaceServerClient::onMessage(osc::Message& m) {
  std::cout << "Received: " << m.addressPattern() << std::endl;

  float x;
  if (m.addressPattern() == "/pose") {
    Pose pose;
    m >> pose.pos().x;
    m >> pose.pos().y;
    m >> pose.pos().z;
    m >> pose.quat().x;
    m >> pose.quat().y;
    m >> pose.quat().z;
    m >> pose.quat().w;
    nav().set(pose);
  } else if (m.addressPattern() == "/mx") {
    m >> x;
    nav().moveR(x * mNavSpeed);
  } else if (m.addressPattern() == "/my") {
    m >> x;
    nav().moveU(x * mNavSpeed);
  } else if (m.addressPattern() == "/mz") {
    m >> x;
    nav().moveF(x * -mNavSpeed);
  } else if (m.addressPattern() == "/tx") {
    m >> x;
    nav().spinR(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/ty") {
    m >> x;
    nav().spinU(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/tz") {
    m >> x;
    nav().spinF(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/home") {
    nav().home();
  } else if (m.addressPattern() == "/halt") {
    nav().halt();
  } else if (m.addressPattern() == "/eyeSep") {
    m >> x;
    lens().eyeSep(x);
  } else if (m.addressPattern() == "/near") {
    m >> x;
    lens().near(x);
  } else if (m.addressPattern() == "/far") {
    m >> x;
    lens().far(x);
  } else if (m.addressPattern() == "/focalLength") {
    m >> x;
    lens().focalLength(x);
  } else {
  }
  nav().print();
}

InterfaceServerClient::InterfaceServerClient(const char* deviceServerAddress, int port,
                     int deviceServerPort)
    : mOSCRecv(port),
      mOSCSend(deviceServerPort, deviceServerAddress, 0, 4096) {

  oscRecv().bufferSize(32000);
  oscRecv().handler(*this);

  mNavSpeed = 0.1;
  mNavTurnSpeed = 0.02;
  mNav = new Nav();
  mLens = new Lens();
  nav().smooth(0.8);
  lens().near(0.1);
  lens().far(100);
  lens().focalLength(6.0);
  // lens().eyeSep(0.03);
  lens().eyeSepAuto();
}

inline InterfaceServerClient::~InterfaceServerClient() {
  disconnect();
}

inline void InterfaceServerClient::connect() {
  if (oscSend().opened())
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   defaultConfig());
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   interfaceServerConfig());
}

inline void InterfaceServerClient::disconnect() {
  oscSend().send("/interface/disconnectApplication", "default");
  oscSend().send("/interface/disconnectApplication", name());
}

inline const char* InterfaceServerClient::name() { return "app"; }

inline const char* InterfaceServerClient::interfaceServerConfig() {
  return R"(
      app = {
        name : 'app',
        receivers :[ {type : 'OSC', port : 12001}, ],
        inputs: {
          test: {min: 0, max: 1},
        },
        mappings: [
          { input: { io:'keypress', name:' ' }, output:{ io:'app', name:'test' } },
        ]
      }
  )";
}

inline const char* InterfaceServerClient::defaultConfig() {
  return R"(
      var eye=0.2;
      var dd = function(v){
        if(Math.abs(v) < 0.03) return 0.0;
        else return v;
      }
      app = {
        name : 'default',
        receivers :[ {type : 'OSC', port : 12001}, ],
        inputs: {
          mx: {min: -1, max: 1, },
          my: {min: -1, max: 1, },
          mz: {min: -1, max: 1, },
          tx: {min: -1, max: 1, },
          ty: {min: -1, max: 1, },
          tz: {min: -1, max: 1, },
          home: {min: 0, max: 1, },
          halt: {min: 0, max: 1, },
          eyeSep: {min: 0, max:1 },
          near: {min: 0, max:1 },
          far: {min: 0, max:1 },
          focalLength: {min: 0, max:1 },
        },
        mappings: [
          { input: { io:'keypress', name:'q' }, output:{ io:'default', name:'home'}, expression:function(v){console.log(v)} },
          { input: { io:'keypress', name:'w' }, output:{ io:'default', name:'mz'}, },
          { input: { io:'keypress', name:'a' }, output:{ io:'default', name:'mx'}, expression:function(v){return -v;} },
          { input: { io:'keypress', name:'s' }, output:{ io:'default', name:'halt'}, },
          { input: { io:'keypress', name:'x' }, output:{ io:'default', name:'mz'}, expression:function(v){return -v;} },
          { input: { io:'keypress', name:'d' }, output:{ io:'default', name:'mx'}, },
          { input: { io:'keypress', name:'i' }, output:{ io:'default', name:'eyeSep'}, expression:function(v){ eye += 0.01; console.log('eyesep: ',eye)} },
          { input: { io:'keypress', name:'k' }, output:{ io:'default', name:'eyeSep'}, expression:function(v){ eye -= 0.01; console.log('eyesep: ',eye)} },

          { input: { io:'Logitech RumblePad 2 USB #1', name:'leftX' }, output:{ io:'default', name:'mx'}, expression:dd },
          { input: { io:'Logitech RumblePad 2 USB #1', name:'leftY' }, output:{ io:'default', name:'mz'}, expression:dd },
          { input: { io:'Logitech RumblePad 2 USB #1', name:'rightX' }, output:{ io:'default', name:'ty'}, expression:dd },
          { input: { io:'Logitech RumblePad 2 USB #1', name:'rightY' }, output:{ io:'default', name:'tx'}, expression:dd },

          { input: { io:'Logitech Cordless RumblePad 2 #1', name:'leftX' }, output:{ io:'default', name:'mx' }, expression:dd },
          { input: { io:'Logitech Cordless RumblePad 2 #1', name:'leftY' }, output:{ io:'default', name:'mz' }, expression:dd },
          { input: { io:'Logitech Cordless RumblePad 2 #1', name:'rightX' }, output:{ io:'default', name:'ty' }, expression:dd },
          { input: { io:'Logitech Cordless RumblePad 2 #1', name:'rightY' }, output:{ io:'default', name:'tx' }, expression:dd },
        ]
      }
  )";
}

}  // al::

#endif
