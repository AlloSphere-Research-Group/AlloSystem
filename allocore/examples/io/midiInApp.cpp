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

			// Bind ourself to the MIDIIn
			MIDIMessageHandler::bindTo(midiIn);
		
			// Open the first device found
			midiIn.openPort(0);
			printf("Opened port to %s\n", midiIn.getPortName(0).c_str());
		}
		else{
			printf("Error: No MIDI devices found.\n");
		}
	}

	// This gets called whenever a MIDI message is received on the port
	void onMIDIMessage(double time, const std::vector<unsigned char>& msg){

		// The first byte is the status byte indicating the message type
		unsigned char status = msg[0];
	
		printf("%s: ", MIDIByte::messageTypeString(status));

		// Check if we received a channel message
		if(MIDIByte::isChannelMessage(status)){		
			unsigned char type = status & MIDIByte::MESSAGE_MASK;
			unsigned char chan = status & MIDIByte::CHANNEL_MASK;
	
			// Here we demonstrate how to parse to common channel messages
			switch(type){
			case MIDIByte::NOTE_ON:
				printf("Note %u, Vel %u", msg[1], msg[2]);
				break;

			case MIDIByte::NOTE_OFF:
				printf("Note %u, Vel %u", msg[1], msg[2]);
				break;
			
			case MIDIByte::PITCH_WHEEL:
				printf("Value %u", MIDIByte::convertPitchWheel(msg[1], msg[2]));
				break;
	
			// Control messages need to be parsed again...
			case MIDIByte::CONTROL_CHANGE:
				printf("%s ", MIDIByte::controlNumberString(msg[1]));
				switch(msg[1]){
				case MIDIByte::MODULATION:
					printf("%u", msg[2]);
					break;
				}
				break;
			default:;
			}
			
			printf(" (MIDI chan %u)", chan + 1);
		}
		printf("\n");
	
		// Print the raw byte values and time stamp
		printf("\tBytes = ");
		for(unsigned i=0; i<msg.size(); ++i){
			printf("%3u ", (int)msg[i]);
		}
		printf(", time = %g\n", time);
	}
};


int main(){
	MyApp().start();
}
