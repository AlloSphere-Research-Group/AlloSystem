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


//mdns::Client z("_osc._udp");
//mdns::Client z("_ssh._tcp");
mdns::Client z("_http._tcp");

mdns::Server zs("mDNS TEST", Socket::hostIP());

int main(){
	printf("starting on %s\n", Socket::hostIP().c_str());
	
	while (1) {
		z.poll();
		zs.poll();
		al_sleep(0.1);
	}
	return 0;
}
