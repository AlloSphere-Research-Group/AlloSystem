/*
Allocore Example: MIDI In App

Description:
The example demonstrates how to open a MIDI input port and parse some of the
more common channel messages.

Author:
Lance Putnam, 9/2013
*/

#include <stdio.h>
#include "allocore/io/al_App.hpp"
#include "allocore/io/al_MIDI.hpp"
using namespace al;

class MyApp : public App, public MIDIMessageHandler{
public:

	MIDIIn midiIn;

	MyApp(){
		// Check for connected MIDI devices
		if(midiIn.getPortCount() > 0){
			//int port = midiIn.getPortCount()-1; // last port
			int port = 0; // first port

			// Bind ourself to the MIDIIn
			MIDIMessageHandler::bindTo(midiIn, port);

			// Open specified port
			midiIn.openPort(port);
			printf("Opened port to %s\n", midiIn.getPortName(port).c_str());
		}
		else{
			printf("Error: No MIDI devices found.\n");
		}
	}

	// This gets called whenever a MIDI message is received on the port
	void onMIDIMessage(const MIDIMessage& m) override {

		printf("%s: ", MIDIByte::messageTypeString(m.status()));

		// Here we demonstrate how to parse common channel messages
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

		// Control messages need to be parsed again...
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

		// If it's a channel message, print out channel number
		if(m.isChannelMessage()){
			printf(" (MIDI chan %u)", m.channel() + 1);
		}

		printf("\n");

		// Print the raw byte values and time stamp
		printf("\tBytes = ");
		for(unsigned i=0; i<3; ++i){
			printf("%3u ", (int)m.bytes[i]);
		}
		printf(", time = %g\n", m.timeStamp());

		fflush(stdout);
	}
};


int main(){
	MyApp().start();
}
