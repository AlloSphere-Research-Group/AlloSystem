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


int main(){
	MIDIIn midiIn;

	unsigned numPorts = midiIn.portCount();

	// First check if there are any MIDI devices
	if(!numPorts){
		printf("No MIDI devices detected!\n");
		return 0;
	}

	// Print out names of available input ports
	for(unsigned i=0; i<numPorts; ++i){
		printf("Port %u: %s\n", i, midiIn.portName(i).c_str());
	}

	unsigned portToOpen = 0;

	// Check available ports vs. specified
	if(portToOpen >= numPorts){
		printf("Invalid port specifier!\n");
		return 0;
	}

	// Set our callback function. This should be done before opening the port.
	midiIn.onMessage([](const MIDIMessage& m){

		printf("%s: ", MIDIByte::messageTypeString(m.status()));

		// Here we check for the more common channel messages
		switch(m.type()){
		case MIDIByte::NOTE_ON:
			printf("Note %u, Vel %f", m.noteNumber(), m.velocity());
			break;

		case MIDIByte::NOTE_OFF:
			printf("Note %u, Vel %f", m.noteNumber(), m.velocity());
			break;

		case MIDIByte::PITCH_BEND:
			printf("Value %f", m.pitchBend());
			break;

		case MIDIByte::CONTROL_CHANGE:
			printf("%s ", MIDIByte::controlNumberString(m.controlNumber()));
			switch(m.controlNumber()){
			case MIDIByte::MODULATION:
				printf("%f", m.controlValue());
				break;

			case MIDIByte::EXPRESSION:
				printf("%f", m.controlValue());
				break;
			}
			break;
		default:;
		}

		if(m.isChannelMessage()){
			printf(" (MIDI chan %u)", m.channel() + 1);
		}

		printf("\n");

		printf("\tBytes = ");
		for(unsigned i=0; i<3; ++i){
			printf("%3u ", (int)m.bytes[i]);
		}
		printf(", time = %g\n", m.timeStamp());

		fflush(stdout);
	});

	// Optional: Don't ignore sysex, timing, or active sensing messages
	//midiIn.ignoreTypes(false, false, false);

	// Open the port specified above
	if(midiIn.open(portToOpen)){
		printf("\nReading MIDI input from \"%s\" ... press <enter> to quit.\n", midiIn.name().c_str());
		fflush(stdout);
		getchar();
	} else {
		printf("Failed to open port to device.\n");
	}
}
