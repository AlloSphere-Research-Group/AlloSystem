/*
Author: Charlie Roberts - charlie[AT]charlie[HYPHEN]roberts[DOT]com

This is a demo integrating Allocore with Interface.Server ( https://github.com/charlieroberts/interface.server.2 )

To setup and run Interface.Server:

1. If not already installed, download and install node.js (nodejs.org)
2. Clone the Interface.Server repo (linked above)
3. cd into the interface.server rpo
4. Run "npm install". This will install all dependencies.
5. Run "npm install interface.server.osc" This will install OSC support
6. Run "npm install interface.server.keyboard" This will install keyboard support
7. To run the server, "node ."

Then run this demo file using run.sh. If you press the 'b' key in the node.js terminal window, you should see
an OSC message received in the Allocore app. Contain your excitement.

*/

#include <assert.h>
#include <stdio.h>
#include "allocore/al_Allocore.hpp"
using namespace al;

#define INTERFACE_SERVER_RECEIVE_PORT 12000
#define INTERFACE_SERVER_IP_ADDRESS "127.0.0.1"
#define OSC_RECEIVE_PORT 10080
#define APPLICATION_NAME "simple"

int main(){

	osc::Send s( INTERFACE_SERVER_RECEIVE_PORT, INTERFACE_SERVER_IP_ADDRESS );
	osc::Recv r( OSC_RECEIVE_PORT, "" );

	// setup a receive handler and start it running before we connect to the Interface.Server
	{
		struct OSCHandler : public osc::PacketHandler{
			void onMessage(osc::Message& m){
				m.print();

                // the message we'll receive consists of a single float
                float v;
                m >> v;

                printf("v = %f\n", v);
			}
		} handler;

		r.timeout(1);
		r.handler(handler);
		r.start();	// make sure timeout > 0
	}

	/*
	* /handshake message tells Interface.Server to load a specific application description, located in a folder named 'applications'
    * found in the Interface.Server repo. In this case we use the application "simple", which is the most basic example that
    * comes with the Interface.Server repo.
	*/

	s.send("/interface/handshake", APPLICATION_NAME );

	printf("press return to quit the application and disconnect from Interface.Server.\n");
	getchar();

	// make sure your application discconnects from the Device Server when it exits
	s.send("/interface/disconnectApplication", APPLICATION_NAME );

	return 0;
}
