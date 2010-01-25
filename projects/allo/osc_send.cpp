#include "system/al_mainloop.h"
#include "protocol/al_OSCAPR.hpp"
#include "stdlib.h"

int main (int argc, char * argv[]) {
	
	// init APR:
	apr_initialize();
	atexit(apr_terminate);	// ensure apr tear-down
	
	osc::Send * sender = new osc::Send("localhost", 7009);
	
	char data[OSC_DEFAULT_MAX_MESSAGE_LEN];
	osc::OutboundPacketStream packet(data, OSC_DEFAULT_MAX_MESSAGE_LEN);
	for (int i=0; i<10; i++) {
	
		packet << osc::BeginMessage("/foo");
		packet << i;
		packet << osc::EndMessage;
		
		printf("sent %d bytes\n", sender->send(packet));
		packet.Clear();
		
		al_sleep(0.1);
	}
	
	delete sender;
	return 0;
}