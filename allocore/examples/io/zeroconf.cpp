/*
Allocore Example: Freenect

Description:
Demonstrating mDNS

Author:
Graham Wakefield, 2012
*/

#include "allocore/protocol/al_Zeroconf.hpp"
#include "allocore/io/al_Socket.hpp"
using namespace al;

class ZeroconfNotifier : public zero::Client {
public:

	ZeroconfNotifier(std::string type) : zero::Client(type) {}
	virtual ~ZeroconfNotifier() {}

	///! called when a new service name is added:
	virtual void onServiceNew(const std::string& name) {
		printf("Zeroconf: new service '%s' of type '%s' in domain '%s'\n",
			name.c_str(), type.c_str(), domain.c_str());
	}

	///! usually called after onServiceNew
	/// identifies the host/port/address(es) associated with the service name
	virtual void onServiceResolved(const std::string& name, const std::string& host_name, uint16_t port, const std::string& address) {
		printf("Zeroconf: resolved service '%s' on host '%s' on port %u at address '%s'\n",
			name.c_str(), host_name.c_str(), port, address.c_str());
	}

	///! called when existing service name is removed:
	virtual void onServiceRemove(const std::string& name) {
		printf("Zeroconf: removed service '%s' of type '%s' in domain '%s'\n",
			name.c_str(), type.c_str(), domain.c_str());
	}
};


int main() {
	//std::string type("_http._tcp");
	//std::string type("_ssh._tcp");
	std::string type("_osc._udp");

	// a Client can browse and report available services on the network for a given service type:
	ZeroconfNotifier z(type);

	// a Service can publish a service on this machine:
	zero::Service zservice("allocore:" + Socket::hostName(), 4110, type);

	//Window win;
	//win.create();
	//MainLoop::start();

	// if the MainLoop::start() was not called, this alternative works:
	while (1) { z.poll(); }

	return 0;
}
