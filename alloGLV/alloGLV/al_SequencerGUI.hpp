#ifndef AL_SEQUENCERGUI_HPP
#define AL_SEQUENCERGUI_HPP
/*	AlloSystem --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012-2016. The Regents of the University of California.
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
	GUI generation for sequencer

	File author(s):
	Andres Cabrera 2016, andres@mat.ucsb.edu
*/

#include "allocore/ui/al_SequenceRecorder.hpp"
#include "allocore/ui/al_PresetSequencer.hpp"
#include "allocore/ui/al_PresetMapper.hpp"
#include "alloGLV/al_ParameterGUI.hpp"
#include "GLV/glv.h"

namespace al {

class SequencerGUI
{
public:

	static glv::View *makePresetHandlerView(PresetHandler &handler, float buttonScale = 1.0f, int numColumns = 10, int numRows = 4);

	static glv::View *makeSequencerPlayerView(PresetSequencer &sequencer);

	static glv::View *makeSequencerView(PresetSequencer &sequencer);

	static glv::View *makeRecorderView(SequenceRecorder &recorder);

	static glv::View *makePresetMapperView(PresetMapper &presetMapper, bool showArchives = true);

private:
	struct PresetViewData {
		PresetButtons *presetButtons;
		PresetHandler *presetHandler;
	};

	struct SequencePlayerData {
		PresetSequencer *sequencer;
		glv::DropDown *sequenceSelector;
	};

	struct RecordViewData {
		SequenceRecorder *recorder;
		glv::TextView *recordNameTextView;
	};

	static void presetChangeCallback(int index, void *sender,void *userData)
	{
		PresetViewData *viewData = static_cast<PresetViewData *>(userData);
		glv::Data &d = viewData->presetButtons->data();
		const int *shape = d.shape();
		for (int x = 0; x < shape[0]; ++x) {
			for (int y = 0; y < shape[1]; ++y) {
				d.assign<int>(false, x, y); // Must manually set them all off...
			}
		}
		viewData->presetButtons->presetName = viewData->presetHandler->availablePresets()[index];
		d.assign<int>(true, index); // Do it this way to avoid the GUI triggering callbacks.

		//		std::cout << "preset Change callback" << std::endl;
	}

	static void presetSavedInButton(const glv::Notification &n) {
		glv::Button *button = static_cast<glv::Button *>(n.receiver());
		button->setValue(false);
	}
};

}

#endif // AL_SEQUENCERGUI_HPP
