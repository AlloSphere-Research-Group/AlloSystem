#include "system/al_mainloop.h"
#include "protocol/al_OSCAPR.hpp"
#include "stdlib.h"

int main (int argc, char * argv[]) {
	
	osc::Send sender("localhost", 7009);
	
	char data[OSC_DEFAULT_MAX_MESSAGE_LEN];
	osc::OutboundPacketStream packet(data, OSC_DEFAULT_MAX_MESSAGE_LEN);
	for (int i=0; i<10; i++) {
	
		packet << osc::BeginMessage("/foo");
		packet << i;
		packet << osc::EndMessage;
		
		printf("sent %d bytes\n", (int)sender.send(packet));
		
		packet.Clear();
		al_sleep(0.1);
	}
	
	return 0;
}
