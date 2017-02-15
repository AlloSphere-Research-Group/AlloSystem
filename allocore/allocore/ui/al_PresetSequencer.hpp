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
#include <functional>

#include "allocore/protocol/al_OSC.hpp"
#include "allocore/ui/al_Preset.hpp"

namespace al
{

class SequenceRecorder;
class Composition;

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
class PresetSequencer : public osc::MessageConsumer
{
	friend class Composition;
public:
	PresetSequencer() :
	    mSequencerActive(true),
	    mRunning(false),
	    mSequencerThread(NULL),
	    mBeginCallbackEnabled(false),
	    mEndCallbackEnabled(false)
	{
	}

	~PresetSequencer()
	{
		mSequencerActive = false;
		stopSequence(false);
		if (mPresetHandler) {
			mPresetHandler->stopMorph();
		}
	}

	struct Step {
		std::string presetName;
		float delta; // The time to get to the preset
		float duration; // The time to stay in the preset before the next step
	};

	/**
	 * @brief Start playing the sequence specified
	 * @param sequenceName
	 *
	 * There is a single sequencer engine in the PresetSequencer class, so if
	 * a sequence is playing when this command is issued, the current playback
	 * is interrupted and the new sequence requested starts immediately.
	 */
	void playSequence(std::string sequenceName);

	void stopSequence(bool triggerCallbacks = true);

	/**
	 * @brief Stores a copy of a sequence with its associated presets
	 * @param sequenceName Name of sequence without extension. Searched for in mDirectory
	 * @param overwrite if directory exists, delete it before writing if overwrite is true.
	 * @return returns false if there was any error archiving sequence
	 *
	 * Stores a copy of the sequence and all its associated presets in a new folder.
	 * A PresetHandler must be registered for this to work as this sets the current
	 * Sequence and preset directory.
	 */
	bool archiveSequence(std::string sequenceName, bool overwrite = true);

	/**
	 * @brief getSequenceList returns a list of sequences in the current sequence directory
	 * @return a list of sequence names without path and without the '.sequence' extension
	 */
	std::vector<std::string> getSequenceList();

	/**
	 * @brief setDirectory sets the working directory for the PresetSequencer
	 * @param directory
	 *
	 * This function can be used in cases where the actual sequencer is not
	 * needed, because preset recall and morphing is done by the PresetHandler.
	 * This is useful for instance if you need to query the step of a sequence
	 * for a different purpose. If a PresetHandler is registered, this value
	 * is ignored.
	 */
	void setDirectory(std::string directory) { mDirectory = directory; }

	/**
	 * @brief Sets the sub-directory for the registered PresetHandler.
	 *
	 * Useful when you need to set the sub-dir but don't have easy access
	 * to the PresetHandler object.
	 */
	void setHandlerSubDirectory(std::string subDir);

	inline bool running() { return mRunning; }

	PresetSequencer &operator<< (PresetHandler &presetHandler)
	{
		mPresetHandler = &presetHandler;
		mDirectory = mPresetHandler->getCurrentPath();
//		std::cout << "Path set to:" << mDirectory << std::endl;
		return *this;
	}
	/**
	 * @brief Load steps from a sequence file
	 * @param sequenceName The name of the sequence
	 * @return the steps
	 *
	 * The sequence is searched in the PresetHandler current path or the
	 *  PresetSequencer's directory if PresetHandler not registered.
	 */
	std::queue<Step> loadSequence(std::string sequenceName);

	std::string currentSequence() { return mCurrentSequence; }

	void setOSCSubPath(std::string subPath) { mOSCsubPath = subPath; }

	/**
	 * @brief registerBeginCallback
	 *
	 * When registered this function is called from the playback thread as soon
	 * as it is ready to start playing before calling the first step.
	 */
	void registerBeginCallback(std::function<void(PresetSequencer *sender, void *userData)> beginCallback,
	                           void *userData = nullptr);

	void enableBeginCallback(bool enable) { mBeginCallbackEnabled = enable; }

	void toggleEnableBeginCallback() { mBeginCallbackEnabled = !mBeginCallbackEnabled; }

	/**
	 * @brief registerEndCallback
	 *
	 * The callback provides a finished argument. If the sequence reached the end naturally,
	 * this argument will be passed as true. If it was stopped by the user prematurely, it
	 * will send false.
	 */
	void registerEndCallback(std::function<void(bool finished, PresetSequencer *sender, void *userData)> endCallback,
	                         void *userData = nullptr);

	void enableEndCallback(bool enable) { mEndCallbackEnabled = enable; }
	void toggleEnableEndCallback() { mEndCallbackEnabled = !mEndCallbackEnabled; }

	float getSequenceTotalDuration(std::string sequenceName);

	void clearSteps();

	void appendStep(Step &newStep);

protected:
	virtual bool consumeMessage(osc::Message &m, std::string rootOSCPath) override;

private:
	static void sequencerFunction(PresetSequencer *sequencer);

	std::string buildFullPath(std::string sequenceName);

	std::queue<Step> mSteps;
	std::string mDirectory;
	PresetHandler *mPresetHandler;
	std::string mOSCsubPath;
	std::string mCurrentSequence;

	std::mutex mSequenceLock;
	std::mutex mPlayWaitLock;
	std::condition_variable mPlayWaitVariable;

	bool mSequencerActive;
	bool mRunning;
	std::thread *mSequencerThread;
	bool mBeginCallbackEnabled;
	std::function<void(PresetSequencer *, void *userData)> mBeginCallback;
	void *mBeginCallbackData;
	bool mEndCallbackEnabled;
	std::function<void(bool,PresetSequencer *, void *userData)> mEndCallback;
	void *mEndCallbackData;
};


class SequenceServer : public osc::PacketHandler, public OSCNotifier<>
{
public:
	/**
	 * @brief SequenceServer constructor
	 *
	 * @param oscAddress The network address on which to listen to. If empty use all available network interfaces. Defaults to "127.0.0.1".
	 * @param oscPort The network port on which to listen. Defaults to 9012.
	 *
	 * The sequencer server triggers sequences when it receives a valid sequence
	 * name on OSC path /sequence.
	 */

	SequenceServer(std::string oscAddress = "127.0.0.1",
	             int oscPort = 9012);
	/**
	 * @brief using this constructor reuses the existing osc::Recv server from the
	 * ParameterServer object
	 * @param paramServer an existing ParameterServer object
	 *
	 * You will want to reuse an osc::Recv server when you want to expose the
	 * interface thorugh the same network port. Since network ports are exclusive,
	 * once a port is bound, it can't be used. You might need to expose the
	 * parameters on the same network port when using things like interface.simpleserver.js
	 * That must connect all interfaces to the same network port.
	 */
	SequenceServer(ParameterServer &paramServer);
	~SequenceServer();

	virtual void onMessage(osc::Message& m);

	// Special cases of objects that are handled in specific ways
	SequenceServer &registerSequencer(PresetSequencer &sequencer);
	SequenceServer &registerRecorder(SequenceRecorder &recorder);
	SequenceServer &registerMessageConsumer(osc::MessageConsumer &consumer);

	/**
	 * @brief print prints information about the server to std::out
	 */
	void print();

	/**
	 * @brief stopServer stops the OSC server thread. Calling this function
	 * is sometimes required when this object is destroyed abruptly and the
	 * destructor is not called.
	 */
	void stopServer();

	SequenceServer &operator <<(PresetSequencer &sequencer) {return registerSequencer(sequencer);}
	SequenceServer &operator <<(SequenceRecorder &recorder) {return registerRecorder(recorder);}
	SequenceServer &operator <<(osc::MessageConsumer &consumer) {return registerMessageConsumer(consumer);}

	void setAddress(std::string address);
	std::string getAddress();

protected:
//	void attachPacketHandler(osc::PacketHandler *handler);
	static void changeCallback(int value, void *sender, void *userData);

private:
	osc::Recv *mServer;
	PresetSequencer *mSequencer;
	SequenceRecorder *mRecorder;
	ParameterServer *mParamServer;
	std::vector<Composition *> mCompositions;
//	std::mutex mServerLock;
	std::string mOSCpath;
	std::string mOSCQueryPath;
//	std::mutex mHandlerLock;
//	std::vector<osc::PacketHandler *> mHandlers;
	std::vector<osc::MessageConsumer *> mConsumers;
};


} // namespace al

#endif // AL_PRESETSEQUENCER_H
