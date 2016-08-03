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

	PresetMIDI(int deviceIndex) {
		mPresetHandler = NULL;
		MIDIMessageHandler::bindTo(mMidiIn);
		try {
			mMidiIn.openPort(deviceIndex);
			printf("PresetMIDI: Opened port to %s\n", mMidiIn.getPortName(deviceIndex).c_str());
		}
		catch (al::MIDIError error) {
			std::cout << "PresetMIDI Warning: Could not open MIDI port " << deviceIndex << std::endl;
		}
	}

	PresetMIDI(int deviceIndex, PresetHandler &presetHandler) {
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
	                         float presetHigh = -1, int noteHigh = -1) {
		if (noteHigh == -1) {
			presetHigh = presetLow;
			noteHigh = noteLow;
		}
		if (noteHigh - noteLow != presetHigh - presetLow) {
			std::cout << "PresetMIDI warning different ranges for preset and note ranges." << std::endl;
		}
		for (int num = noteLow; num <= noteHigh; ++num) {
			NoteBinding newBinding;
			newBinding.channel = channel - 1;
			newBinding.noteNumber = num;
			newBinding.presetIndex = presetLow - noteLow + num;
			mBindings.push_back(newBinding);
//			std::cout << channel << " " << num << " " << newBinding.presetIndex << std::endl;
		}
	}

	virtual void onMIDIMessage(const MIDIMessage& m) override {
		if (m.type() == MIDIByte::NOTE_ON
		        && m.velocity() > 0 ) {
			for(NoteBinding binding: mBindings) {
//				std::cout << binding.channel << " " << binding.noteNumber << " " << binding.presetIndex << std::endl;
//				std::cout << (int) m.channel() << std::endl;
				if (m.channel() == binding.channel
				        && m.noteNumber() == binding.noteNumber) {
//					m.print();
					mPresetHandler->recallPreset(binding.presetIndex);
				}
			}
		}
	}

private:

	struct NoteBinding {
		int noteNumber;
		int channel;
		int presetIndex;
	};

	PresetHandler *mPresetHandler;

	MIDIIn mMidiIn;
	std::vector<NoteBinding> mBindings;

};


}


#endif // AL_PRESETMIDI_H
