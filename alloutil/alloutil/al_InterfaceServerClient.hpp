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

  virtual void onMessage(osc::Message& m);

  void connect();
  void disconnect();

  osc::Recv& oscRecv() { return mOSCRecv; }
  osc::Send& oscSend() { return mOSCSend; }

  static const char* getDefaultDevice

 protected:
  osc::Recv mOSCRecv;
  osc::Send mOSCSend;
};

inline void InterfaceServerClient::onMessage(osc::Message& m) {
  std::cout << "Received: " << m.addressPattern() << std::endl;
}

inline void InterfaceServerClient::connect() {
  if (oscSend().opened())
    oscSend().send("/interface/applicationManager/createApplicationWithText",
                   deviceServerConfig());
}

inline void InterfaceServerClient::disconnect() {
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
}

inline const char* InterfaceServerClient::name() { return "default_no_name"; }

inline const char* InterfaceServerClient::deviceServerConfig() {
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
