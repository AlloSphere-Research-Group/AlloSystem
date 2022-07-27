/*
Allocore Example: HID

Description:
This shows how to read data from a HID (human interface device).

Author:
Lance Putnam, June 2013
*/
#include <stdio.h>
#include "allocore/system/al_Time.hpp"
#include "allocore/io/al_HID.hpp"
using namespace al;

int main(){

	// Print all recognized HID devices
	HID::printDevices();

	// Create a HID object
	HID hid;

	//int vid=0x046d, pid=0xc06c; // Logitech, USB Optical Mouse
	//int vid=0x046d, pid=0xc218; // Logitech, Logitech RumblePad 2 USB
	//int vid=0x045e, pid=0x0040; // Microsoft 3-Button Mouse with IntelliEye(TM)
	int vid=0x046d, pid=0xc626; // 3DConnexion SpaceNavigator

	//* Attempt to open HID with given vendor and product IDs
	if(!hid.open(vid, pid)){
		printf("Unable to open HID device\n");
		return -1;
	}//*/

	/* Or open by searching for product
	if(!hid.open(HID::find("Mouse"))){
		printf("Unable to open HID device\n");
		return -1;
	}//*/

	printf("Opened HID device:\n");
	printf("Manufacturer:  %ls\n", hid.manufacturer().c_str());
	printf("Product:       %ls\n", hid.product().c_str());
	printf("Serial number: %ls\n", hid.serialNumber().c_str());
	fflush(stdout);

	// Create a do-wait loop to read data from the HID
	unsigned char buf[256];
	while(true){
		// Here we just read all available bytes on the bus
		int num;
		while((num = hid.read(buf, sizeof(buf)))){
			printf("Data read: ");
			for(int i=0; i<num; ++i)
				printf("%2x ", buf[i]);
			printf("\n");
		}

		// Wait a bit since reads are non-blocking by default
		al::wait(1./20);
	}
}
