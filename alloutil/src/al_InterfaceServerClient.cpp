#include "alloutil/al_InterfaceServerClient.hpp"

using namespace al;

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

