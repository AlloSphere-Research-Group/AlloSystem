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
  // Lens& lens() { return *mLens; }
  void setLens(Lens& l){ mLens=&l; }

  // Nav& nav() { return *mNav; }
  void setNav(Nav& n){ mNav=&n; }

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
    (*mNav).set(pose);
  } else if (m.addressPattern() == "/mx") {
    m >> x;
    (*mNav).moveR(x * mNavSpeed);
  } else if (m.addressPattern() == "/my") {
    m >> x;
    (*mNav).moveU(x * mNavSpeed);
  } else if (m.addressPattern() == "/mz") {
    m >> x;
    (*mNav).moveF(x * -mNavSpeed);
  } else if (m.addressPattern() == "/tx") {
    m >> x;
    (*mNav).spinR(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/ty") {
    m >> x;
    (*mNav).spinU(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/tz") {
    m >> x;
    (*mNav).spinF(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/home") {
    (*mNav).home();
  } else if (m.addressPattern() == "/halt") {
    (*mNav).halt();
  } else if (m.addressPattern() == "/eyeSep") {
    m >> x;
    (*mLens).eyeSep(x);
  } else if (m.addressPattern() == "/near") {
    m >> x;
    (*mLens).near(x);
  } else if (m.addressPattern() == "/far") {
    m >> x;
    (*mLens).far(x);
  } else if (m.addressPattern() == "/focalLength") {
    m >> x;
    (*mLens).focalLength(x);
  } else {
  }
  (*mNav).print();
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
  (*mNav).smooth(0.8);
  (*mLens).near(0.1);
  (*mLens).far(100);
  (*mLens).focalLength(6.0);
  // (*mLens).eyeSep(0.03);
  (*mLens).eyeSepAuto();
}

inline InterfaceServerClient::~InterfaceServerClient() {
  disconnect();
}

inline void InterfaceServerClient::connect() {
  if (oscSend().opened()){
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   defaultConfig());
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   interfaceServerConfig());
  }
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
        transports :[ {type : 'osc', port : 12001}, ],
        inputs: {
          test: {min: 0, max: 1},
        },
        mappings: [
          { input: { io:'keyboard', name:' ' }, output:{ io:'app', name:'test' } },
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
        transports :[ {type : 'osc', port : 12001}, ],
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
          { input: { io:'keyboard', name:'q' }, output:{ io:'default', name:'home'}, expression:function(v){console.log(v)} },
          { input: { io:'keyboard', name:'w' }, output:{ io:'default', name:'mz'}, },
          { input: { io:'keyboard', name:'a' }, output:{ io:'default', name:'mx'}, expression:function(v){return -v;} },
          { input: { io:'keyboard', name:'s' }, output:{ io:'default', name:'halt'}, },
          { input: { io:'keyboard', name:'x' }, output:{ io:'default', name:'mz'}, expression:function(v){return -v;} },
          { input: { io:'keyboard', name:'d' }, output:{ io:'default', name:'mx'}, },
          { input: { io:'keyboard', name:'i' }, output:{ io:'default', name:'eyeSep'}, expression:function(v){ eye += 0.01; console.log('eyesep: ',eye)} },
          { input: { io:'keyboard', name:'k' }, output:{ io:'default', name:'eyeSep'}, expression:function(v){ eye -= 0.01; console.log('eyesep: ',eye)} },

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
