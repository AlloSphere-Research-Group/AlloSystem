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

  virtual const char* name();
  virtual const char* deviceServerConfig();
  const char* basicConfig();

  virtual void onMessage(osc::Message& m);

  void connect();
  void disconnect();

  osc::Recv& oscRecv() { return mOSCRecv; }
  osc::Send& oscSend() { return mOSCSend; }


 protected:
  osc::Recv mOSCRecv;
  osc::Send mOSCSend;
  Nav mNav;
  double mNavSpeed, mNavTurnSpeed;

};

inline void InterfaceServerClient::onMessage(osc::Message& m) {
  std::cout << "Received: " << m.addressPattern() << std::endl;

  float x;
  if (m.addressPattern() == "/mx") {
    m >> x;
    mNav.moveR(x * mNavSpeed);
  } else if (m.addressPattern() == "/my") {
    m >> x;
    mNav.moveU(x * mNavSpeed);
  } else if (m.addressPattern() == "/mz") {
    m >> x;
    mNav.moveF(x * mNavSpeed);
  } else if (m.addressPattern() == "/tx") {
    m >> x;
    mNav.spinR(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/ty") {
    m >> x;
    mNav.spinU(x * mNavTurnSpeed);
  } else if (m.addressPattern() == "/tz") {
    m >> x;
    mNav.spinF(x * -mNavTurnSpeed);
  } else if (m.addressPattern() == "/home") {
    mNav.home();
  } else if (m.addressPattern() == "/halt") {
    mNav.halt();
  } else {
  }
}

inline void InterfaceServerClient::connect() {
  if (oscSend().opened())
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   basicConfig());
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   deviceServerConfig());
}

inline void InterfaceServerClient::disconnect() {
  oscSend().send("/interface/disconnectApplication", "app");
  oscSend().send("/interface/disconnectApplication", name());
}

inline InterfaceServerClient::~InterfaceServerClient() {
  disconnect();
}

InterfaceServerClient::InterfaceServerClient(const char* deviceServerAddress, int port,
                     int deviceServerPort)
    : mOSCRecv(port),
      mOSCSend(deviceServerPort, deviceServerAddress) {

  oscRecv().bufferSize(32000);
  oscRecv().handler(*this);

  mNavSpeed = 1;
  mNavTurnSpeed = 0.02;
  mNav.smooth(0.8);
}

inline const char* InterfaceServerClient::name() { return "default"; }

inline const char* InterfaceServerClient::deviceServerConfig() {
  return R"(
      app = {
        name : 'default',
        receivers :[ {type : 'OSC', port : 12001}, ],
        inputs: {
          test: {min: 0, max: 1},
        },
        mappings: [
          { input: { io:'keypress', name:' ' }, output:{ io:'default', name:'test' } },
        ]
      }
  )";
}

inline const char* InterfaceServerClient::basicConfig() {
  return R"(
      app = {
        name : 'app',
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
          { input: { io:'keypress', name:'`' }, output:{ io:'app', name:'home' } },
          { input: { io:'keypress', name:'w' }, output:{ io:'app', name:'mx'}, },
        ]

      }
  )";
}

}  // al::

#endif
