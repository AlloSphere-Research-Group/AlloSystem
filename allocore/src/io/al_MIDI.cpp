#include <cmath>
#include "allocore/system/al_Config.h"
#include "allocore/io/al_MIDI.hpp"
using namespace al;

#define AL_MIDI_RTMIDI // the only backend...

#ifdef AL_MIDI_RTMIDI

#if defined(AL_OSX)
	#define __MACOSX_CORE__
#elif defined(AL_WINDOWS)
	#define __WINDOWS_MM__
#elif defined(AL_LINUX)
	#define __LINUX_ALSA__
#endif

#include "rtmidi/RtMidi.cpp"

struct MIDIIn::Impl{

	RtMidiIn midi;

	Impl(){}
	~Impl(){}

	int portCount(){ return midi.getPortCount(); }
	std::string portName(int i){ return midi.getPortName(i); }

	bool open(int port){
		try {
			midi.openPort(port);
			return true;
		} catch (RtMidiError& error){
		}

		return false;
	}

	void close(){
		midi.closePort();
	}

	void bindCallback(MIDIIn * outer){
		midi.setCallback([](double t, std::vector<unsigned char> * msgPtr, void *user){
			auto& o = *static_cast<MIDIIn *>(user);
			if(!o.mOnMessage) return;

			auto& m = *msgPtr;

			switch(m.size()){
			case 3:
				o.mOnMessage(MIDIMessage(t, m[0], m[1], m[2]));
				break;
			case 2:
				o.mOnMessage(MIDIMessage(t, m[0], m[1]));
				break;
			case 1:
				o.mOnMessage(MIDIMessage(t, m[0]));
				break;
			case 0:;
			default: // sysex
				o.mOnMessage(MIDIMessage(t, m[0], m[1], m[2], &m[3]));
			}
		}, outer);
	}

	void ignoreTypes(bool sysex, bool midiTime, bool sense){
		midi.ignoreTypes(sysex, midiTime, sense);
	}
};

#endif // AL_MIDI_RTMIDI


double noteToHz(double noteNumber){
	return std::pow(2., (noteNumber - 69.)/12.) * 440.;
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
	double timeStamp,
	unsigned char b1, unsigned char b2, unsigned char b3,
	unsigned char * data
)
:	mTimeStamp(timeStamp), mData(data)
{
	bytes[0] = b1;
	bytes[1] = b2;
	bytes[2] = b3;
}

void MIDIMessage::print() const {
	printf("%s", MIDIByte::messageTypeString(status()));

	if(type() == MIDIByte::CONTROL_CHANGE){
		printf(" %2d", controlNumber());
		const char * s = MIDIByte::controlNumberString(controlNumber());
		if(s[0]) printf(" (%s)", s);
	}

	if(isChannelMessage()){
		printf(" chan:%u", channel() + 1);
	}

	printf(" bytes:");
	for(unsigned i=0; i<3; ++i)
		printf("%3u ", (int)bytes[i]);

	printf(" time:%g\n", timeStamp());
}


MIDIIn::MIDIIn(){
	mImpl->bindCallback(this);
}

std::string MIDIIn::name() const {
	return opened() ? const_cast<MIDIIn*>(this)->portName(mPort) : "";
}

int MIDIIn::portCount(){
	return mImpl->portCount();
}

std::string MIDIIn::portName(int i){
	return mImpl->portName(i);
}

bool MIDIIn::open(int port){
	if(!opened() && mImpl->open(port)){
		mPort = port;
		return true;
	}
	return false;
}

bool MIDIIn::open(const std::string& keyword){
	int numPorts = portCount();
	for(int i=0; i<numPorts; ++i){
		auto name = portName(i);
		if(name.find(keyword) != std::string::npos) return open(i);
	}
	return false;
}

void MIDIIn::close(){
	if(opened()){
		mImpl->close();
		mPort = -1;
	}
}

bool MIDIIn::opened() const {
	return mPort>=0;
}

MIDIIn& MIDIIn::ignoreTypes(bool sysex, bool midiTime, bool sense){
	mImpl->ignoreTypes(sysex, midiTime, sense);
	return *this;
}
