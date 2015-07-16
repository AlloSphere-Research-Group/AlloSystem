/*
Allocore Example: Bluetooth

Description:
This demonstrates how to check for available Bluetooth devices, open a 
connection to a device, and send and receive data.

Author:
Lance Putnam, 2014
*/

#include "allocore/io/al_Bluetooth.hpp"
using namespace al;

int main(){

	// First check to see if Bluetooth is available
	if(!Bluetooth::available()){
		printf("Bluetooth not available\n");
		return 0;
	}

	// Print all detected devices
	Bluetooth::printDevices();

	// Create a Bluetooth object
	Bluetooth bt;

	// Choose an address of the device to open
	const char * addr = "68:86:e7:03:09:5e"; // Sphero
	//const char * addr = "04:e4:51:fd:f4:69"; // Phone


	// Open an RFCOMM connection
	if(bt.openRFCOMM(addr)){
		printf("opened RFCOMM connection on channel %d\n", bt.channel());
	}
	else{
		printf("error opening RFCOMM connection\n");
		return 0;
	}

	// At this point, you should be able to send/recv data
	std::vector<unsigned char> buffer;

	// Send data
	/*
	buffer.push_back('H');
	buffer.push_back('e');
	buffer.push_back('l');
	buffer.push_back('l');
	buffer.push_back('o');
	bt.send(buffer);
	//*/

	// Receive data (e.g., a server response)
	/*
	if(bt.recv(buffer)){
		printf("received %d bytes: ", buffer.size());
		for(unsigned i=0; i<buffer.size(); ++i){
			printf("%x ", buffer[i]);
		}
		printf("\n");
	}
	//*/
}
