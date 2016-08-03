#ifndef AL_PARAMETERMIDI_H
#define AL_PARAMETERMIDI_H

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
	Class to connect MIDI Input to Parameter objects
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include "allocore/io/al_MIDI.hpp"
#include "allocore/ui/al_Parameter.hpp"

namespace al
{

/**
 * @brief The ParameterMIDI class connects Parameter objects to MIDI messages
 *
 *
@code
    Parameter Size("Size", "", 1.0, "", 0, 1.0);
    Parameter Speed("Speed", "", 0.05, "", 0.01, 0.3);

    ParameterMIDI parameterMIDI;

    parameterMIDI.connectControl(Size, 1, 1);
    parameterMIDI.connectControl(Speed, 10, 1);
@endcode
 *
 */
class ParameterMIDI : public MIDIMessageHandler {
public:

	ParameterMIDI() {}

	ParameterMIDI(int deviceIndex, bool verbose = false) {
		MIDIMessageHandler::bindTo(mMidiIn);
		mVerbose = verbose;
		try {
			mMidiIn.openPort(deviceIndex);
			printf("ParameterMIDI: Opened port to %s\n", mMidiIn.getPortName(deviceIndex).c_str());
		}
		catch (al::MIDIError error) {
			std::cout << "ParameterMIDI Warning: Could not open MIDI port " << deviceIndex << std::endl;
		}
	}

	void init(int deviceIndex = 0, bool verbose = false) {
		MIDIMessageHandler::bindTo(mMidiIn);
		mVerbose = verbose;
		try {
			mMidiIn.openPort(deviceIndex);
			printf("ParameterMIDI: Opened port to %s\n", mMidiIn.getPortName(deviceIndex).c_str());
		}
		catch (al::MIDIError error) {
			std::cout << "ParameterMIDI Warning: Could not open MIDI port " << deviceIndex << std::endl;
		}
	}

	void connectControl(Parameter &param, int controlNumber, int channel)
	{
		connectControl(param, controlNumber, channel, (float) param.min(), (float) param.max());
	}

	void connectControl(Parameter &param, int controlNumber, int channel,
	                    float min, float max) {
		ControlBinding newBinding;
		newBinding.controlNumber = controlNumber;
		newBinding.channel = channel - 1;
		newBinding.param = &param;
		newBinding.min = min;
		newBinding.max = max;
		mBindings.push_back(newBinding);
	}

	/**
	 * @brief connectNoteToValue
	 * @param param the parameter to bind
	 * @param channel MIDI channel (1-16)
	 * @param min The parameter value to map the lowest note
	 * @param low The MIDI note number for the lowest (or only) note to map
	 * @param max The value unto which the highest note is mapped
	 * @param high The highest MIDI note number to map
	 */
	void connectNoteToValue(Parameter &param, int channel,
	                        float min, int low,
	                        float max = -1, int high = -1) {
		if (high == -1) {
			max = min;
			high = low;
		}
		for (int num = low; num <= high; ++num) {
			NoteBinding newBinding;
			newBinding.noteNumber = num;
			if (num != high) {
				newBinding.value = min + (max - min) * (num - low)/(high - low);
			} else {
				newBinding.value = max;
			}
			newBinding.channel = channel - 1;
			newBinding.param = &param;
			mNoteBindings.push_back(newBinding);
		}
	}

	void connectNoteToToggle(ParameterBool &param, int channel, int note) {
		ToggleBinding newBinding;
		newBinding.noteNumber = note;
		newBinding.toggle = true;
		newBinding.channel = channel - 1;
		newBinding.param = &param;
		mToggleBindings.push_back(newBinding);
	}

	void connectNoteToIncrement(Parameter &param, int channel, int note,
	                            float increment) {
		IncrementBinding newBinding;
		newBinding.channel = channel - 1;
		newBinding.noteNumber = note;
		newBinding.increment = increment;
		newBinding.param = &param;
		mIncrementBindings.push_back(newBinding);
	}

	virtual void onMIDIMessage(const MIDIMessage& m) override {
		if (m.type() & MIDIByte::CONTROL_CHANGE ) {
			for(ControlBinding binding: mBindings) {
				if (m.channel() == binding.channel
				        && m.controlNumber() == binding.controlNumber) {
					float newValue = binding.min + (m.controlValue() * (binding.max - binding.min));
					binding.param->set(newValue);
				}
			}
		}
		if (m.type() & MIDIByte::NOTE_ON && m.velocity() > 0) {
			for(NoteBinding binding: mNoteBindings) {
				if (m.channel() == binding.channel
				        && m.noteNumber() == binding.noteNumber) {
					binding.param->set(binding.value);
				}
			}
			for(IncrementBinding binding: mIncrementBindings) {
				if (m.channel() == binding.channel
				        && m.noteNumber() == binding.noteNumber) {
					binding.param->set(binding.param->get() + binding.increment);
				}
			}
			for(ToggleBinding binding: mToggleBindings) {
				if (m.channel() == binding.channel
						&& m.noteNumber() == binding.noteNumber) {
					if (binding.toggle == true) {
						binding.param->set(
									binding.param->get() == binding.param->max() ?
										binding.param->min() : binding.param->max() );
					} else {
						binding.param->set(binding.param->max());
					}
				}
			}
		} else if (m.type() & MIDIByte::NOTE_OFF
				   || (m.type() & MIDIByte::NOTE_ON && m.velocity() == 0)) {

			for(ToggleBinding binding: mToggleBindings) {
				if (m.channel() == binding.channel
						&& m.noteNumber() == binding.noteNumber) {
					if (binding.toggle != true) {
						binding.param->set( binding.param->min() );
					}
				}
			}
		}
		if (mVerbose) {
			m.print();
		}
	}

private:

	struct ControlBinding {
		int controlNumber;
		int channel;
		Parameter *param;
		float min, max;
	};

	struct NoteBinding {
		int noteNumber;
		int channel;
		float value;
		Parameter *param;
	};

	struct ToggleBinding {
		int noteNumber;
		int channel;
		bool toggle;
		ParameterBool *param;
	};

	struct IncrementBinding {
		int noteNumber;
		int channel;
		float increment;
		Parameter *param;
	};

	MIDIIn mMidiIn;
	bool mVerbose;
	std::vector<ControlBinding> mBindings;
	std::vector<NoteBinding> mNoteBindings;
	std::vector<ToggleBinding> mToggleBindings;
	std::vector<IncrementBinding> mIncrementBindings;
};


}


#endif // AL_PARAMETERMIDI_H
