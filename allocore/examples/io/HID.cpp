#include <stdio.h>
#include "allocore/system/al_Time.hpp"
#include "allocore/io/al_HID.hpp"

int main(){

	al::HID::printDevices();

	al::HID hid;

	//int vid=0x046d, pid=0xc06c; // Logitech, USB Optical Mouse
	//int vid=0x046d, pid=0xc218; // Logitech, Logitech RumblePad 2 USB
	int vid=0x045e, pid=0x0040; // Microsoft 3-Button Mouse with IntelliEye(TM)
	//int vid=0x045e, pid=0x00dd;	

	if(!hid.open(vid, pid)){
		printf("Unable to open HID device\n");
		return -1;
	}

	printf("Manufacturer:  %ls\n", hid.manufacturer().c_str());
	printf("Product:       %ls\n", hid.product().c_str());
	printf("Serial number: %ls\n", hid.serialNumber().c_str());
	
	unsigned char buf[256];
	while(1){
		int num = hid.read(buf, sizeof(buf));
		if(num){
			printf("Data read: ");
			for(int i=0; i<num; ++i)
				printf("%2x ", buf[i]);
			printf("\n");
		}
		al::wait(1./10);
	}
}
