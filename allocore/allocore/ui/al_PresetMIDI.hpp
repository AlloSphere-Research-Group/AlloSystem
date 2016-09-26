#ifndef AL_PRESETMIDI_H
#define AL_PRESETMIDI_H

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2015. The Regents of the University of California.
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

		Redistributions of source code must retain the above copyright notice,
		this list of conditions and the following disclaimer.

		Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in the
		documentation and/or other materials provided with the distribution.

		Neither the name of the University of California nor the names of its
		contributors may be used to endorse or promote products derived from
		this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.

	File description:
	Class to connect MIDI Input to PresetHandler objects
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include "allocore/io/al_MIDI.hpp"
#include "allocore/ui/al_Preset.hpp"

namespace al
{

/**
 * @brief The PresetMIDI class connects PresetHandler objects to MIDI messages
 *
 *
@code

@endcode
 *
 */
class PresetMIDI : public MIDIMessageHandler {
public:

	PresetMIDI() {}

	PresetMIDI(int deviceIndex) : mPresetHandler(nullptr) {
		MIDIMessageHandler::bindTo(mMidiIn);
		try {
			mMidiIn.openPort(deviceIndex);
			printf("PresetMIDI: Opened port to %s\n", mMidiIn.getPortName(deviceIndex).c_str());
		}
		catch (al::MIDIError error) {
			std::cout << "PresetMIDI Warning: Could not open MIDI port " << deviceIndex << std::endl;
		}
		mMorphBinding.channel = mMorphBinding.controlNumber = -1;
	}

	PresetMIDI(int deviceIndex, PresetHandler &presetHandler) :
	    PresetMIDI(deviceIndex)
	{
		mPresetHandler = &presetHandler;
		setPresetHandler(*mPresetHandler);
	}

	void init(int deviceIndex, PresetHandler &presetHandler) {
		MIDIMessageHandler::bindTo(mMidiIn);
		try {
			mMidiIn.openPort(deviceIndex);
			printf("PresetMIDI: Opened port to %s\n", mMidiIn.getPortName(deviceIndex).c_str());
		}
		catch (al::MIDIError error) {
			std::cout << "PresetMIDI Warning: Could not open MIDI port " << deviceIndex << std::endl;
		}
		setPresetHandler(presetHandler);
	}

	void setPresetHandler(PresetHandler &presetHandler) {
		mPresetHandler = &presetHandler;
	}

	void connectNoteToPreset(int channel,
	                         float presetLow, int noteLow,
	                         float presetHigh = -1, int noteHigh = -1);

	void setMorphControl(int controlNumber, int channel, float min, float max);

	virtual void onMIDIMessage(const MIDIMessage& m) override;

private:

	struct NoteBinding {
		int noteNumber;
		int channel;
		int presetIndex;
	};

	struct MorphBinding {
		int controlNumber;
		int channel;
		float min;
		float max;
	};

	MorphBinding mMorphBinding;

	PresetHandler *mPresetHandler;

	MIDIIn mMidiIn;
	std::vector<NoteBinding> mBindings;

};


}


#endif // AL_PRESETMIDI_H
