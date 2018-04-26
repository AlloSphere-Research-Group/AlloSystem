
#include "allocore/ui/al_PresetMIDI.hpp"

using namespace al;


void PresetMIDI::connectNoteToPreset(int channel, float presetLow, int noteLow, float presetHigh, int noteHigh) {
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

void PresetMIDI::connectProgramToPreset(int channel, float presetLow, int programLow, float presetHigh, int programHigh)
{
	if (programHigh == -1) {
		presetHigh = presetLow;
		programHigh = programLow;
	}
	if (programHigh - programLow != presetHigh - presetLow) {
		std::cout << "PresetMIDI warning different ranges for preset and program ranges." << std::endl;
	}
	for (int num = programLow; num <= programHigh; ++num) {
		ProgramBinding newBinding;
		newBinding.channel = channel - 1;
		newBinding.programNumber = num;
		newBinding.presetIndex = presetLow - programLow + num;
		mProgramBindings.push_back(newBinding);
		//			std::cout << channel << " " << num << " " << newBinding.presetIndex << std::endl;
	}
}


void PresetMIDI::setMorphControl(int controlNumber, int channel, float min, float max)
{
	mMorphBinding.controlNumber = controlNumber;
	mMorphBinding.channel = channel - 1;
	mMorphBinding.min = min;
	mMorphBinding.max = max;
}

void PresetMIDI::onMIDIMessage(const MIDIMessage &m) {
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
	} else if (m.type() == MIDIByte::PROGRAM_CHANGE
	           && m.velocity() > 0 ) {
		for(ProgramBinding binding: mProgramBindings) {
			//				std::cout << binding.channel << " " << binding.noteNumber << " " << binding.presetIndex << std::endl;
			//				std::cout << (int) m.channel() << std::endl;
			if (m.channel() == binding.channel
			        && m.noteNumber() == binding.programNumber) {
				//					m.print();
				mPresetHandler->recallPreset(binding.presetIndex);
			}
		}
	} else if (m.type() == MIDIByte::CONTROL_CHANGE) {
		if (m.controlNumber() == mMorphBinding.controlNumber
		        && m.channel() == mMorphBinding.channel) {
			mPresetHandler->setMorphTime(mMorphBinding.min + m.controlValue() * (mMorphBinding.max - mMorphBinding.min));
		}
	}
}

