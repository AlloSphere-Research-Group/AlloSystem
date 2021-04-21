/*
Author: Charlie Roberts - charlie[AT]charlie[HYPHEN]roberts[DOT]com

For this demo to work, the DeviceServer (DS) needs to be able to find a folder of scripts for the application
"AllocoreTest". This folder is included in the same directory as this file. Move the folder to the
current location for default DS scripts (as specified in the DS preferences).

For more information about the DeviceServer, please see: http://www.allosphere.ucsb.edu/DeviceServer/
*/

#include <assert.h>
#include <stdio.h>
#include "allocore/protocol/al_OSC.hpp"
using namespace al;

#define DEVICE_SERVER_RECEIVE_PORT 12000
#define DEVICE_SERVER_IP_ADDRESS "127.0.0.1"
#define MY_RECEIVE_PORT 11998
#define APPLICATION_NAME "AllocoreTest"


int main(){

	osc::Send s(DEVICE_SERVER_RECEIVE_PORT, DEVICE_SERVER_IP_ADDRESS);
	osc::Recv r(MY_RECEIVE_PORT, "");

	// setup a receive handler and start it running before we connect to the DeviceServer
	{
		struct OSCHandler : public osc::PacketHandler{
			void onMessage(osc::Message& m){
				m.print();

				// after an application sends a /handshake message to the DeviceServer, the DeviceServer sends a
				// message back acknowledging reception
				if(m.addressPattern() == "/handshake") {
					printf("DeviceServer handshake received\n");
				}else{
					printf("not handshake");
					// All (well, almost all) messages sent from the DeviceServer consist of a single float
					float v;
					m >> v;

					printf("v = %f\n", v);
				}
			}
		} handler;

		r.timeout(1);
		r.handler(handler);
		r.start();	// make sure timeout > 0
	}

	/*
	/handshake message tells the device server to look in a directory with the same name as the application and read in the
	Interface.lua file found in that directory. You specify the name of your application and the port it would like to receive
	messages on as arguments.
	*/

	s.send("/handshake", APPLICATION_NAME, MY_RECEIVE_PORT );

	printf("press return to quit the application and disconnect from the Device Sever.\n");
	getchar();

	// make sure your application discconnects from the Device Server when it exits
	s.send("/disconnectApplication", APPLICATION_NAME );

	return 0;
}
