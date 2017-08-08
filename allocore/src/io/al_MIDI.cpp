#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include "allocore/system/al_Config.h"
#include "allocore/io/al_MIDI.hpp"


using namespace al;

namespace al{
double noteToHz(double noteNumber){
	return ::pow(2., (noteNumber - 69.)/12.) * 440.;
}
}

const char * MIDIByte::messageTypeString(unsigned char statusByte){
	switch(statusByte & MESSAGE_MASK){
	case NOTE_OFF:			return "NOTE_OFF";
	case NOTE_ON:			return "NOTE_ON";
	case CONTROL_CHANGE:	return "CONTROL_CHANGE";
	case PROGRAM_CHANGE:	return "PROGRAM_CHANGE";
	case PRESSURE_POLY:		return "PRESSURE_POLY";
	case PRESSURE_CHAN:		return "PRESSURE_CHAN";
	case PITCH_BEND:		return "PITCH_BEND";
	case SYSTEM_MSG:
		switch(statusByte){
		case SYS_EX:			return "SYS_EX";
		case SYS_EX_END:		return "SYS_EX_END";
		case TIME_CODE:			return "TIME_CODE";
		case SONG_POSITION:		return "SONG_POSITION";
		case SONG_SELECT:		return "SONG_SELECT";
		case TUNE_REQUEST:		return "TUNE_REQUEST";
		case TIMING_CLOCK:		return "TIMING_CLOCK";
		case SEQ_START:			return "SEQ_START";
		case SEQ_CONTINUE:		return "SEQ_CONTINUE";
		case SEQ_STOP:			return "SEQ_STOP";
		case ACTIVE_SENSING:	return "ACTIVE_SENSING";
		case RESET:				return "RESET";
		}
	default: return "";
	}
}

const char * MIDIByte::controlNumberString(unsigned char controlNumber){
	switch(controlNumber){
	case MODULATION:	return "MODULATION";
	case EXPRESSION:	return "EXPRESSION";
	default: return "";
	}
}

unsigned short MIDIByte::convertPitchBend(unsigned char byte2, unsigned char byte3){
	unsigned short r = byte3;
	return (r<<7) | byte2;
}


MIDIMessage::MIDIMessage(
	double timeStamp, unsigned port,
	unsigned char b1, unsigned char b2, unsigned char b3,
	unsigned char * data
)
:	mTimeStamp(timeStamp), mPort(port), mData(data)
{
	bytes[0] = b1;
	bytes[1] = b2;
	bytes[2] = b3;
}

void MIDIMessage::print() const{
	printf("%s", MIDIByte::messageTypeString(status()));

	if(type() == MIDIByte::CONTROL_CHANGE){
		printf(" (%s)", MIDIByte::controlNumberString(controlNumber()));
	}

	printf(", port %d", port());

	if(isChannelMessage()){
		printf(", chan %u", channel() + 1);
	}

	printf(", bytes ");
	for(unsigned i=0; i<3; ++i)
		printf("%3u ", (int)bytes[i]);

	printf(", time %g\n", timeStamp());
}

void MIDIMessageHandler::bindTo(RtMidiIn& midiIn, unsigned port){
	struct F{
	static void callback(double t, std::vector<unsigned char> * msgPtr, void *user){
		Binding& b = *static_cast<Binding *>(user);
		std::vector<unsigned char>& m = *msgPtr;

		switch(m.size()){
		case 3:
			b.handler->onMIDIMessage(MIDIMessage(t, b.port, m[0], m[1], m[2]));
			break;
		case 2:
			b.handler->onMIDIMessage(MIDIMessage(t, b.port, m[0], m[1]));
			break;
		case 1:
			b.handler->onMIDIMessage(MIDIMessage(t, b.port, m[0]));
			break;
		case 0:;
		default: // sysex
			b.handler->onMIDIMessage(MIDIMessage(t, b.port, m[0], m[1], m[2], &m[3]));
		}
	}
	};

	Binding b = { &midiIn, this, port };
	mBindings.push_back(b);

	midiIn.setCallback(F::callback, &mBindings.back());
}


