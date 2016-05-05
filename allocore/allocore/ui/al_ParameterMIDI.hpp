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
	ParameterMIDI(int deviceIndex = 0) {
		MIDIMessageHandler::bindTo(mMidiIn);
		mMidiIn.openPort(deviceIndex);
		printf("Opened port to %s\n", mMidiIn.getPortName(deviceIndex).c_str());
	}

	void connectControl(Parameter &param, int controlNumber, int channel)
	{
		connectControl(param, controlNumber, channel, (float) param.min(), (float) param.max());
	}

	void connectControl(Parameter &param, int controlNumber, int channel,
	                    float min, float max) {
		ParameterBinding newBinding;
		newBinding.controlNumber = controlNumber;
		newBinding.channel = channel - 1;
		newBinding.param = &param;
		newBinding.min = min;
		newBinding.max = max;
		mBindings.push_back(newBinding);
	}

	virtual void onMIDIMessage(const MIDIMessage& m) override {
		if (m.type() & MIDIByte::CONTROL_CHANGE ) {
			for(ParameterBinding binding: mBindings) {
				if (m.channel() == binding.channel
				        && m.controlNumber() == binding.controlNumber) {
					float newValue = binding.min + (m.controlValue() * (binding.max - binding.min));
					binding.param->set(newValue);
				}
			}
		}
	}

private:

	struct ParameterBinding {
		int controlNumber;
		int channel;
		Parameter *param;
		float min, max;
	};

	MIDIIn mMidiIn;
	std::vector<ParameterBinding> mBindings;

};


}


#endif // AL_PARAMETERMIDI_H
