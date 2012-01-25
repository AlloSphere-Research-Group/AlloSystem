/*
Allocore Example: Freenect

Description:
Demonstrating mDNS

Author:
Graham Wakefield, 2012
*/

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_mDNS.hpp"

using namespace al;

//std::string type("_http._tcp"); 
//std::string type("_ssh._tcp");
std::string type("_osc._udp");

// a Service can publish a service on this machine:
mdns::Service zservice("allocore", 4110, type);

// a Client can browse and report available services on the network for a given service type:
mdns::Client z(type);

int main() {
	MainLoop::start();
	return 0;
}
