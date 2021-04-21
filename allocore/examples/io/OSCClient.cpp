/*
Allocore Example: OSC Client

Description:
This is a simple OSC client that periodically sends out a packet with the
address "/test" and containing a string and int.

You should run the OSC server example BEFORE running this program.

Author:
Lance Putnam, Oct. 2014
*/

#include <iostream>
#include <string>
#include "allocore/protocol/al_OSC.hpp"
#include "allocore/system/al_Time.hpp"
using namespace al;

int main(){

	// The port over which to send packets
	short port = 16447;

	// The IP address to send packets to
	const char * addr = "127.0.0.1";
	//const char * addr = "172.28.247.249";

	// Create an OSC client
	osc::Send client(port, addr);

	// Here, we use a loop to send a packet out every half second.
	//int i=1; while(i++){
	for(int i=0; i<10; ++i){
		std::string str = "Hello!";

		// The send function takes an OSC address pattern as the first argument
		// followed by the packet data.
		int b = client.send("/test", str, i);

		std::cout << "CLIENT: sent message (" << b << " bytes)\n";

		// Wait before sending the next packet, otherwise they will all get
		// sent at once.
		al::wait(0.5);
	}

}
