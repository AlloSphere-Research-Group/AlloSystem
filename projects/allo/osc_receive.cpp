/* allocore */
#include "system/al_mainloop.h"
#include "system/al_time.h"
#include "types/al_types.h"

#include "protocol/al_OSCAPR.hpp"

/* oscpack */
#include "protocol/oscpack/osc/OscOutboundPacketStream.h"
#include "protocol/oscpack/osc/OscReceivedElements.h"

#include "stdlib.h"

#define MAX_MESSAGE_LEN (4096)

/* APR utility */
static apr_status_t check_apr(apr_status_t err) {
	char errstr[1024];
	if (err != APR_SUCCESS) {
		apr_strerror(err, errstr, 1024);
		fprintf(stderr, "apr error: %s\n", errstr);
	}
	return err;
}

/* OSC packet parser */
void osc_parsemessage(const osc::ReceivedMessage & p, void * userdata) {
	printf("address %s tags %s args %d\n", p.AddressPattern(), p.TypeTags(), p.ArgumentCount());
	// etc.
}

int main (int argc, char * argv[]) {
	
	// init APR:
	apr_initialize();
	atexit(apr_terminate);	// ensure apr tear-down
	
	unsigned int port = 7007;
	osc::Recv * receiver = new osc::Recv(port);

	// receive data:
	for (int i=0; i<1000; i++) {
		apr_size_t len;
		char data[MAX_MESSAGE_LEN];
		do {
			len = receiver->recv(osc_parsemessage, NULL);
		} while (len > 0);
		al_sleep(0.01);
	}
	
	delete receiver;
	return 0;
}
