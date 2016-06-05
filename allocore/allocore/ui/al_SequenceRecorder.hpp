#ifndef AL_SEQUENCERECORDER_H
#define AL_SEQUENCERECORDER_H

/*	Allocore --
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
	Preset sequence recorder

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>

#include "allocore/ui/al_PresetSequencer.hpp"
#include "allocore/ui/al_Preset.hpp"

namespace al
{

class SequenceRecorder
{
public:
	SequenceRecorder();

	~SequenceRecorder() {}

	void registerPresetHandler(PresetHandler &handler) {
		mPresetHandler = &handler;
		mPresetHandler->registerPresetCallback(SequenceRecorder::presetChanged, (void *) this);
	}

	void startRecord(std::string name = "");
	void stopRecord();

	SequenceRecorder & operator<< (PresetHandler &handler) { registerPresetHandler(handler); }

private:

	static void presetChanged(int index, void *sender, void *userData);
	static void recorderFunction(SequenceRecorder *recorder, std::string sequenceName);

	struct Step {
		std::string presetName;
		float delta; // The time to get to the preset
		float duration; // The time to stay in the preset before the next step
	};

	std::string mDirectory;
	PresetHandler *mPresetHandler;
	std::string mPresetName;

	std::mutex mSequenceLock;
	std::condition_variable mSequenceConditionVar;

	bool mRecording;
	std::thread *mRecorderThread;
};


} // namespace al

#endif // AL_SEQUENCERECORDER_H
