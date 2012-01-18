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

std::string type("_http._tcp"); // "_osc._udp", "_ssh._tcp"

// a Client can browse and report available services for a given service type:
mdns::Client z(type);

// a Service can publish a service on this machine:
mdns::Service zservice("mDNS TEST", Socket::hostIP(), 4110, type);

int main(){
	printf("starting on %s\n", Socket::hostIP().c_str());
	
	while (1) {
		z.poll();
		zservice.poll();
		al_sleep(0.1);
	}
	return 0;
}
