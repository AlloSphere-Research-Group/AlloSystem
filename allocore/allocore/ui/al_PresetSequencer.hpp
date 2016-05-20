#ifndef AL_PRESETSEQUENCER_H
#define AL_PRESETSEQUENCER_H

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
	Preset sequencing

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <utility>

#include "allocore/ui/al_Preset.hpp"

namespace al
{

/**
 * @brief The PresetSequencer class allows triggering presets from a PresetHandler over time.
 *
 * In order to use a preset sequencer you must register a preset handler with
 * the sequencer:
 *
 * @code
 * PresetHandler presetHandler;
 * PresetSequencer sequencer;
 *
 * sequencer << presetHandler;
 * @endcode
 *
 * Sequences that trigger presets are stored in text files with the format:
 *
 * @code
 * preset1:0.0:3.0
 * preset2:4.0:2.0
 * preset3:1.0:1.0
 * ::
 * @endcode
 *
 * The first element in each line specifies the preset name that the PresetHandler
 * should load (i.e. a file called "preset1.preset" in the current directory).
 * The second element determines the time to get to the preset from the current state,
 * i.e. the morph time to reach the preset. The third element determines the time
 * the preset should be held after reaching it.
 *
 * The file should end with two colons (::).
 *
 * The directory where sequences are loaded is taken from the PresetHandler
 * object registered with the sequencer.
 *
 */
class PresetSequencer
{
public:
	PresetSequencer() :
	    mSequencerActive(true),
	    mRunning(false),
	    mSequencerThread(PresetSequencer::sequencerFunction, this)
	{ }

	~PresetSequencer()
	{
		mSequencerActive = false;
		mRunning = false;
		mSequenceConditionVar.notify_one();
		mSequencerThread.join();
	}

	void playSequence(std::string sequenceName);

	void stopSequence();

	inline bool running() { return mRunning; }

	PresetSequencer &operator<< (PresetHandler &presetHandler)
	{
		mPresetHandler = &presetHandler;
		mDirectory = mPresetHandler->getCurrentPath();
//		std::cout << "Path set to:" << mDirectory << std::endl;
		return *this;
	}

private:

	static void sequencerFunction(PresetSequencer *sequencer);
	struct Step {
		std::string presetName;
		float delta; // The time to get to the preset
		float duration; // The time to stay in the preset before the next step
	};

	std::queue<Step> mSteps;
	std::string mDirectory;
	PresetHandler *mPresetHandler;

	std::mutex mSequenceLock;
	std::condition_variable mSequenceConditionVar;

	bool mSequencerActive;
	bool mRunning;
	std::thread mSequencerThread;
};

} // namespace al

#endif // AL_PRESETSEQUENCER_H
