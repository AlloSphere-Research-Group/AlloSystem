/*
 *  allua.cpp
 *  allo
 *
 *  Created by Graham Wakefield on 1/24/12.
 *  Copyright 2012 UCSB. All rights reserved.
 *
 */

#include "allocore/al_Allocore.hpp"
#include "allocore/protocol/al_mDNS.hpp"

#include "alloutil/al_Lua.hpp"

const char * startup = AL_STRINGIFY(
print(hostName, hostIP);
print("allua ok");
);

using namespace al;

Lua L;
	
class Allua : public mdns::Client {
public:

	Allua() : mdns::Client(), service("allua") {
		tick(0);
	}

	///! called when a new device is added:
	virtual void onServiceNew(const std::string& name, const std::string& type, const std::string& domain) {
		printf("Zeroconf: new service '%s' of type '%s' in domain '%s'\n", name.c_str(), type.c_str(), domain.c_str());
	}

	virtual void onServiceRemove(const std::string& name, const std::string& type, const std::string& domain) {
		printf("Zeroconf: removed service '%s' of type '%s' in domain '%s'\n", name.c_str(), type.c_str(), domain.c_str());
	}

	virtual void onServiceResolved(const std::string& name, const std::string& host_name, uint16_t port, const std::string& address) {
		printf("Zeroconf: resolved service '%s' on host '%s' on port %u at address '%s'\n", name.c_str(), host_name.c_str(), port, address.c_str());
	}

	void tick(al_sec t) {
		Client::poll();
		
		// re-schedule:
		MainLoop::queue().send(0.1, this, &Allua::tick);
	}

	mdns::Service service;
};


int main(int argc, char * argv[]) {
		
	L.push(Socket::hostName().c_str()).setglobal("hostName");
	L.push(Socket::hostIP()).setglobal("hostIP");
	L.dostring(startup);
	
	Allua A;
	
	MainLoop::start();
	return 0;
}
