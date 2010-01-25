#include "system/al_mainloop.h"
#include "protocol/al_OSCAPR.hpp"
#include "stdlib.h"

void osc_parsemessage(const osc::ReceivedMessage & p, void * userdata) {
	printf("address %s tags %s args %d\n", p.AddressPattern(), p.TypeTags(), p.ArgumentCount());
	// etc.
}

int main (int argc, char * argv[]) {
	
	unsigned int port = 7007;
	osc::Recv receiver(port);
	
	for (int i=0; i<1000; i++) {
		al_sleep(0.01);
		size_t len = 0;
		do {
			len = receiver.recv(osc_parsemessage, NULL);
		} while (len > 0);
	}
	
	return 0;
}
