#ifndef AL_COMPOSITION_H
#define AL_COMPOSITION_H

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
	Composition player

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <vector>
#include <thread>

#include "allocore/ui/al_PresetSequencer.hpp"
#include "allocore/ui/al_Preset.hpp"

namespace al
{

class CompositionStep {
public:
	std::string sequenceName;
	float deltaTime;
};

class Composition : public osc::MessageConsumer
{
public:
	Composition(std::string fileName, std::string path);
	~Composition();

	void play();
	void stop();

	bool playArchive(std::string archiveName);

	std::string getName();

	/// Archive current compostion
	bool archiveComposition();

	void setSubDirectory(std::string subDir) { mSubDirectory = subDir; }

	static bool archive(std::string compositionName, std::string path = "", bool overwrite = true);
	static bool restore(std::string compositionName, std::string path = "",  bool overwrite = true);

	int size();
	const CompositionStep getStep(int index);
	void insertStep(std::string name, float deltaTime, int index);
	void deleteStep(int index);

	void write();

	void registerBeginCallback(std::function<void(Composition *sender, void *userData)> beginCallback,
	                           void *userData = nullptr);

	void enableBeginCallback(bool enable) { mBeginCallbackEnabled = enable; }

	void registerEndCallback(std::function<void(bool finished, Composition *sender, void *userData)> endCallback,
	                         void *userData = nullptr);

	void enableEndCallback(bool enable) { mEndCallbackEnabled = enable; }


	Composition &registerSequencer(PresetSequencer &sequencer) {
		mSequencer = &sequencer;
		return *this;
	}
	Composition &operator<< (PresetSequencer &sequencer) {
		return registerSequencer(sequencer);
	}

protected:
	virtual bool consumeMessage(osc::Message &m, std::string rootOSCPath) override;

private:

	std::vector<CompositionStep> mCompositionSteps;
	std::string mPath;
	std::string mSubDirectory;
	std::string mCompositionName;

	std::thread *mPlayThread;
	bool mPlaying;
	PresetSequencer *mSequencer;
	std::mutex mPlayerLock;
	std::mutex mPlayWaitLock;
	std::condition_variable mPlayWaitVariable;

	// This interface is shared with PresetSequencer, perhaps this should be abstracted?
	bool mBeginCallbackEnabled;
	std::function<void(Composition *, void *userData)> mBeginCallback;
	void *mBeginCallbackData;
	bool mEndCallbackEnabled;
	std::function<void(bool,Composition *, void *userData)> mEndCallback;
	void *mEndCallbackData;

	std::function<void(bool,PresetSequencer *, void *userData)> mSequencerEndCallbackCache;
	void *mSequencerEndCallbackDataCache;

	std::vector<CompositionStep> loadCompositionSteps(std::string compositionSteps);
	std::string getRootPath();
	std::string getCurrentPath();

	static void waitForSequencerCallback(bool finished,PresetSequencer *seq, void *userData);

	static void playbackThread(Composition *composition);
};


} // namespace al

#endif // AL_COMPOSITION_H
