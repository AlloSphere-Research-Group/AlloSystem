/*
Allocore Example: MIDI In

Description:
The example demonstrates how to open a MIDI input port and parse some of the
more common messages.

Author:
Lance Putnam, 12/2012
*/

#include <cstdio>
#include "allocore/io/al_MIDI.hpp"
using namespace al;


void midiCallback(double deltaTime, std::vector<unsigned char> *msg, void *userData){
	unsigned numBytes = msg->size();

	if(numBytes > 0){

		// The first byte is the status byte indicating the message type
		unsigned char status = msg->at(0);

		printf("%s: ", MIDIByte::messageTypeString(status));

		// Check if we received a channel message
		if(MIDIByte::isChannelMessage(status)){
			unsigned char type = status & MIDIByte::MESSAGE_MASK;
			unsigned char chan = status & MIDIByte::CHANNEL_MASK;

			// Here we demonstrate how to parse to common channel messages
			switch(type){
			case MIDIByte::NOTE_ON:
				printf("Note %u, Vel %u", msg->at(1), msg->at(2));
				break;

			case MIDIByte::NOTE_OFF:
				printf("Note %u, Vel %u", msg->at(1), msg->at(2));
				break;

			case MIDIByte::PITCH_BEND:
				printf("Value %u", MIDIByte::convertPitchBend(msg->at(1), msg->at(2)));
				break;

			// Control messages need to be parsed again...
			case MIDIByte::CONTROL_CHANGE:
				printf("%s ", MIDIByte::controlNumberString(msg->at(1)));
				switch(msg->at(1)){
				case MIDIByte::MODULATION:
					printf("%u", msg->at(2));
					break;
				}
				break;
			default:;
			}

			printf(" (MIDI chan %u)", chan + 1);
		}

		printf("\n");



		printf("\tBytes = ");
		for(unsigned i=0; i<numBytes; ++i){
			printf("%3u ", (int)msg->at(i));
		}
		printf(", stamp = %g\n", deltaTime);
	}
}


int main(){
	MIDIIn midiIn;

	// Check available ports vs. specified
	unsigned portToOpen = 0;
	unsigned numPorts = midiIn.getPortCount();

	if(portToOpen >= numPorts){
		printf("Invalid port specifier!\n");
	}

	try {

		// Print out names of available input ports
		for(unsigned i=0; i<numPorts; ++i){
			printf("Port %u: %s\n", i, midiIn.getPortName(i).c_str());
		}

		// Open the port specified above
		midiIn.openPort(portToOpen);
	}
	catch(MIDIError &error){
		error.printMessage();
		return 1;
	}

	// Set our callback function.  This should be done immediately after
	// opening the port to avoid having incoming messages written to the
	// queue instead of sent to the callback function.
	midiIn.setCallback(&midiCallback);

	// Don't ignore sysex, timing, or active sensing messages.
	midiIn.ignoreTypes(false, false, false);

	printf("\nReading MIDI input ... press <enter> to quit.\n");
	getchar();
}
