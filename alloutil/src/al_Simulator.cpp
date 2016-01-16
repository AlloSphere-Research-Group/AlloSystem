#include "alloutil/al_Simulator.hpp"

using namespace al;

Simulator::Simulator(const char* deviceServerAddress, int port,
                     int deviceServerPort)
    : //mNavControl(mNav),
      InterfaceServerClient(deviceServerAddress,port,deviceServerPort) {

  started = false;
  nav().smooth(0.8);
  InterfaceServerClient::setNav(nav());
  InterfaceServerClient::setLens(lens());
}
