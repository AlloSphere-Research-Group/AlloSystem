#ifndef INCLUDE_AL_IO_MIDI_HPP
#define INCLUDE_AL_IO_MIDI_HPP

#include <exception>
#include <iostream>
#include <string>
#include <vector>
#include <queue>

/**********************************************************************/
/* \class MIDI
    \brief An abstract base class for realtime MIDI input/output.

    This class implements some common functionality for the realtime
    MIDI input/output subclasses RtMidiIn and RtMidiOut.

    RtMidi WWW site: http://music.mcgill.ca/~gary/rtmidi/
*/
/**********************************************************************/


#include "RtMidi.h"

namespace al{

// Internal classes to maintain backward compatibility
class MIDIIn : public RtMidiIn {};

class MIDIOut : public RtMidiOut {};

class MIDIError : public RtMidiError {};


/// Convert note number to Hz value
///
/// @ingroup allocore
double noteToHz(double noteNumber);


/// Utilities for parsing MIDI bytes
///
/// @ingroup allocore
class MIDIByte{
public:

	#define BITS_(a,b,c,d,e,f,g,h)\
		(a<<7 | b<<6 | c<<5 | d<<4 | e<<3 | f<<2 | g<<1 | h)

	// Constants for checking the first message byte
	// http://www.midi.org/techspecs/midimessages.php
	static const unsigned char
		CHANNEL_MASK	= BITS_(0,0,0,0, 1,1,1,1), ///< Channel message status byte channel mask
		MESSAGE_MASK	= BITS_(1,1,1,1, 0,0,0,0), ///< Message status byte type mask

		NOTE_OFF		= BITS_(1,0,0,0, 0,0,0,0), ///< Note off channel message type
		NOTE_ON			= BITS_(1,0,0,1, 0,0,0,0), ///< Note on channel message type
		CONTROL_CHANGE	= BITS_(1,0,1,1, 0,0,0,0), ///< Control change channel message type
		 BANK_SELECT	= 0x00, ///< Bank select control number
		 MODULATION		= 0x01, ///< Modulation wheel/stick control number
		 BREATH			= 0x02, ///< Breath controller control number
		 FOOT			= 0x04, ///< Foot controller control number
		 PORTAMENTO_TIME= 0x05, ///< Portamento time control number
		 VOLUME			= 0x07, ///< Channel volume control number
		 BALANCE		= 0x08, ///< Balance control number
		 PAN			= 0x0A, ///< Pan control number
		 EXPRESSION		= 0x0B, ///< Expression controller control number
		 DAMPER_PEDAL	= 0x40, ///< Damper pedal control number
		 PORTAMENTO_ON	= 0x41, ///< Portamento on/off control number
		 SOSTENUTO_ON	= 0x42, ///< Sostenuto on/off control number
		 SOFT_PEDAL		= 0x43, ///< Soft pedal control number
		 LEGATO_ON		= 0x44, ///< Legato on/off control number

		PROGRAM_CHANGE	= BITS_(1,1,0,0, 0,0,0,0), ///< Program change channel message type
		PRESSURE_POLY	= BITS_(1,0,1,0, 0,0,0,0), ///< Polyphonic pressure (aftertouch) channel message type
		PRESSURE_CHAN	= BITS_(1,1,0,1, 0,0,0,0), ///< Channel pressure (aftertouch) channel message type
		PITCH_BEND		= BITS_(1,1,1,0, 0,0,0,0), ///< Pitch bend channel message type

		SYSTEM_MSG		= BITS_(1,1,1,1, 0,0,0,0), ///< System message type

		SYS_EX			= BITS_(1,1,1,1, 0,0,0,0), ///< System exclusive system message type
		SYS_EX_END		= BITS_(1,1,1,1, 0,1,1,1), ///< End of system exclusive system message type
		TIME_CODE		= BITS_(1,1,1,1, 0,0,0,1), ///< Time code system message type
		SONG_POSITION	= BITS_(1,1,1,1, 0,0,1,0), ///< Song position system message type
		SONG_SELECT		= BITS_(1,1,1,1, 0,0,1,1), ///< Song select system message type
		TUNE_REQUEST	= BITS_(1,1,1,1, 0,1,1,0), ///< Tune request system message type
		TIMING_CLOCK	= BITS_(1,1,1,1, 1,0,0,0), ///< Timing clock system message type
		SEQ_START		= BITS_(1,1,1,1, 1,0,1,0), ///< Start sequence system message type
		SEQ_CONTINUE	= BITS_(1,1,1,1, 1,0,1,1), ///< Continue sequence system message type
		SEQ_STOP		= BITS_(1,1,1,1, 1,1,0,0), ///< Stop sequence system message type
		ACTIVE_SENSING	= BITS_(1,1,1,1, 1,1,1,0), ///< Active sensing system message type
		RESET			= BITS_(1,1,1,1, 1,1,1,1)  ///< Reset all receivers system message type
	;
	#undef BITS_

	/// Check status byte to see if the message is a channel message
	static bool isChannelMessage(unsigned char statusByte){
		return (statusByte & MESSAGE_MASK) != SYSTEM_MSG;
	}

	/// Get string with message type from status byte
	static const char * messageTypeString(unsigned char statusByte);

	/// Get string with control type from control number
	static const char * controlNumberString(unsigned char controlNumber);

	/// Convert pitch bend message bytes into a 14-bit value in [0, 16384)
	static unsigned short convertPitchBend(unsigned char byte2, unsigned char byte3);
};


/// MIDI message
///
/// @ingroup allocore
class MIDIMessage {
public:
	unsigned char bytes[3];

	MIDIMessage(double timeStamp, unsigned port,
		unsigned char b1, unsigned char b2=0, unsigned char b3=0,
		unsigned char * data = NULL
	);

	/// Get the MIDI device port
	unsigned port() const { return mPort; }

	/// Get time stamp of message
	double timeStamp() const { return mTimeStamp; }


	/// Get the status byte
	unsigned char status() const { return bytes[0]; }

	/// Returns whether this is a channel (versus system) message
	bool isChannelMessage() const { return MIDIByte::isChannelMessage(status()); }

	/// Get the channel number (0-15)
	unsigned char channel() const { return bytes[0] & MIDIByte::CHANNEL_MASK; }

	/// Get the message type (see MIDIByte)
	unsigned char type() const { return bytes[0] & MIDIByte::MESSAGE_MASK; }


	/// Get note number (type must be NOTE_ON or NOTE_OFF)
	unsigned char noteNumber() const { return bytes[1]; }

	/// Get mapped note velocity (type must be NOTE_ON or NOTE_OFF)
	double velocity(double mul = 1./127.) const { return bytes[2]*mul; }

	/// Get mapped pitch bend amount in [-1,1] (type must be PITCH_BEND)
	double pitchBend() const {
		int v = int(MIDIByte::convertPitchBend(bytes[1], bytes[2]));
		v += 1 - bool(v); // clip interval to [1, 16383]
		return double(v - 8192) / 8191.;
	}

	/// Get controller number (type must be CONTROL_CHANGE)
	unsigned char controlNumber() const { return bytes[1]; }

	/// Get mapped controller value (type must be CONTROL_CHANGE)
	double controlValue(double mul = 1./127.) const { return bytes[2]*mul; }


	/// Get sysex message data
	unsigned char * data() const { return mData; }


	/// Print general information about message
	void print() const;

protected:
	double mTimeStamp;
	unsigned mPort;
	unsigned char * mData;
};


/// Handles receipt of MIDI messages
///
/// @ingroup allocore
class MIDIMessageHandler{
public:
	virtual ~MIDIMessageHandler(){}

	/// Called when a MIDI message is received
	virtual void onMIDIMessage(const MIDIMessage& m) = 0;

	/// Bind handler to a MIDI input
	void bindTo(RtMidiIn& midiIn, unsigned port=0);

protected:
	struct Binding{
		RtMidiIn * midiIn;
		MIDIMessageHandler * handler;
		unsigned port;
	};

	std::vector<Binding> mBindings;
};


} // al::

#endif
