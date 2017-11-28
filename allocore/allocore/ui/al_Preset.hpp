#ifndef AL_PRESET_H
#define AL_PRESET_H

/*	Allocore --
	Multimedia / virtual environment application class library

	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2016. The Regents of the University of California.
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
	Preset classes that encapsulates storing values for groups of parameters
	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <string>
#include <vector>
#include <mutex>
#include <map>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>

#include "allocore/protocol/al_OSC.hpp"
#include "allocore/ui/al_Parameter.hpp"
#include "allocore/system/al_Time.hpp"

namespace  al
{

/**
 * @brief The PresetHandler class handles sorting and recalling of presets.
 *
 * Presets are saved by name with the ".preset" suffix.
 */
class PresetHandler
{
public:

	typedef std::map<std::string, float> ParameterStates;
	/**
	 * @brief PresetHandler contructor
	 *
	 * @param rootDirectory sets the root directory for preset and preset map storage
	 * @param verbose if true, print diagnostic messages
	 *
	 */
	PresetHandler(std::string rootDirectory = "presets", bool verbose = false);

	PresetHandler &registerParameter(Parameter &parameter);

	~PresetHandler();

	/**
	 * @brief Stores preset
	 * @param name the name of the preset
	 *
	 * Since all presets are assinged an index, calling this function will give
	 * the preset a free index and then call storePreset(index, name). If the
	 * preset name already exists, it will overwrite the existing preset without
	 * assigning a new index.
	 */
	void storePreset(std::string name);

	/**
	 * @brief Store preset at index. The name argument specifies the preset name
	 *
	 * @param index
	 * @param name
	 * @param overwrite if false, and preset file exists it is not overwritten and a number is appended
	 *
	 * The preset name also determines the filename under which the preset is saved, so
	 * it must be unique. If name is empty, a unique name is generated. If name
	 * exists, a number is appended to the preset name. The link between preset
	 * index and preset name is store within the preset map file that is
	 * stored in the path for the PresetHandler, see getCurrentPath()
	 *
	 */
	void storePreset(int index, std::string name = "", bool overwrite = true);
	/**
	 * @brief Recall a preset by name
	 * @param name
	 *
	 * The preset should be a file on disk in the PresetHandler's root
	 * directory and should have the ".preset" extension. See also
	 * setSubDirectory().
	 */
	void recallPreset(std::string name);

	/**
	 * @brief Recall a preset by index number
	 * @param index
	 * @return the name of the preset corresponding to index. Empty if index not valid.
	 *
	 * The preset map file (by default called default.presetMap)
	 * is used to map preset names on disk to indeces. You can
	 * set alternative preset map files using setCurrentPresetMap().
	 * See also PresetMapper for handling and archiving preset maps.
	 */
	std::string recallPreset(int index);

	void recallPresetSynchronous(std::string name);

	/**
	 * @brief recall immediately (not using the morph thread)
	 * @param index
	 */
	std::string recallPresetSynchronous(int index);

	/**
	 * @brief Set parameters to values interpolated between two presets
	 * @param index1 index of the first preset
	 * @param index2 index of the second preset
	 * @param factor A value between 0-1 to determine interpolation
	 *
	 * A factor of 0 uses preset 1 and a factor of 1 uses preset 2. Values
	 * in between result in linear interpolation of the values.
	 */
	void setInterpolatedPreset(int index1, int index2, double factor);


	void morphTo(ParameterStates &parameterStates, float morphTime);
	void stopMorph();

	std::map<int, std::string> availablePresets();
	std::string getPresetName(int index);
	std::string getCurrentPresetName() {return mCurrentPresetName; }

	float getMorphTime();
	void setMorphTime(float time);

	void setSubDirectory(std::string directory);
	std::string getSubDirectory() {return mSubDir;}

	/// Path including subdirectory if any
	std::string getCurrentPath();

	/// Base path without appending sub directory
	std::string getRootPath();

	/** Display information about preset handler, including path and
	 * registered parameters.
	 */
	void print();

	/**
	 * @brief Register a callback to be notified when a preset is loaded
	 * @param cb The callback function
	 * @param userData data to be passed to the callback
	 */
	void registerPresetCallback(std::function<void(int index, void *sender, void *userData)> cb,
	                            void *userData = nullptr);

	/**
	 * @brief Register a callback to be notified when a preset is stored
	 * @param cb The callback function
	 * @param userData data to be passed to the callback
	 */
	void registerStoreCallback(std::function<void(int index, std::string name, void *userData)> cb,
	                           void *userData = nullptr);

	/**
	 * @brief Register a callback to be notified when morph time parameter is changed
	 * @param cb The callback function
	 * @param userData data to be passed to the callback
	 */
	void registerMorphTimeCallback(Parameter::ParameterChangeCallback cb, void *userData = nullptr);

	PresetHandler &operator << (Parameter &param) { return this->registerParameter(param); }

	std::string buildMapPath(std::string mapName, bool useSubDirectory = false);

	std::map<int, std::string> readPresetMap(std::string mapName = "default");

	void setCurrentPresetMap(std::string mapName = "default", bool autoCreate = false);

	/**
	 * @brief useCallbacks determines whether to call the internal callbacks
	 * @param use
	 *
	 * The callbacks set by registerStoreCallback() and registerPresetCallback()
	 * are only called if this is set to true. The value is true by default.
	 */
	void useCallbacks(bool use) { mUseCallbacks = use; }

	void changeParameterValue(std::string presetName, std::string parameterPath,
	                          float newValue);

private:
	void storeCurrentPresetMap();

	ParameterStates loadPresetValues(std::string name);
	bool savePresetValues(const ParameterStates &values, std::string presetName,
	                       bool overwrite = true);

	static void morphingFunction(PresetHandler *handler);

	bool mVerbose;
	bool mUseCallbacks;
	std::string mRootDir;
	std::string mSubDir; // Optional sub directory, e.g. for preset map archives
	std::string mFileName;
	std::string mCurrentMapName;
	std::vector<Parameter *> mParameters;
	std::mutex mFileLock;
	bool mRunning; // To keep the morphing thread alive
	bool mMorph; // To be able to trip and stop morphing at any time.
	std::atomic<int> mMorphRemainingSteps;
	float mMorphInterval;
	Parameter mMorphTime;

	std::mutex mMorphLock;
	std::mutex mTargetLock;
	std::condition_variable mMorphConditionVar;
	std::map<std::string, float> mTargetValues;

	std::thread mMorphingThread;

	std::vector<std::function<void(int index, void *sender, void *userData)>> mCallbacks;
	std::vector<void *> mCallbackUdata;
	std::vector<std::function<void(int index, std::string name, void *userData)>> mStoreCallbacks;
	std::vector<void *> mStoreCallbackUdata;

	std::map<int, std::string> mPresetsMap;
	std::string mCurrentPresetName;
};

class PresetServer : public osc::PacketHandler, public OSCNotifier
{
public:
	/**
	 * @brief PresetServer constructor
	 *
	 * @param oscAddress The network address on which to listen to. If empty use all available network interfaces. Defaults to "127.0.0.1".
	 * @param oscPort The network port on which to listen. Defaults to 9011.
	 *
	 */

	PresetServer(std::string oscAddress = "127.0.0.1",
	             int oscPort = 9011);
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
	PresetServer(ParameterServer &paramServer);
	~PresetServer();

	/**
	 * @brief print prints information about the server to std::out
	 */
	void print();

	/**
	 * @brief stopServer stops the OSC server thread. Calling this function
	 * is sometimes required when this object is destroyed abruptly and the
	 * destructor is not called
	 */
	void stopServer();

	bool serverRunning();

	void allowStore(bool allow) {mAllowStore = allow;}
	bool allowStore() {return mAllowStore;}

	virtual void onMessage(osc::Message& m);

	PresetServer &registerPresetHandler(PresetHandler &presetHandler) {
		mPresetHandlers.push_back(&presetHandler);
		presetHandler.registerPresetCallback(PresetServer::changeCallback,
		                                       (void *) this);

		presetHandler.registerMorphTimeCallback(
		            [](float value, void *sender,
		            void *userData, void * blockSender) {
			static_cast<PresetServer *>(userData)->notifyListeners(
			            static_cast<PresetServer *>(userData)->mOSCpath + "/morphTime", value);
		}, this);
		return *this;
	}

	PresetServer &operator <<(PresetHandler &presetHandler) {return registerPresetHandler(presetHandler);}

	void setAddress(std::string address);
	std::string getAddress();

	void notifyPresetChange(bool notify) {mNotifyPresetChange = notify;}

	void attachPacketHandler(osc::PacketHandler *handler);

protected:
	static void changeCallback(int value, void *sender, void *userData);

private:
	osc::Recv *mServer;
	std::vector<PresetHandler *> mPresetHandlers;
//	std::mutex mServerLock;
	std::string mOSCpath;
	std::mutex mHandlerLock;
	std::vector<osc::PacketHandler *> mHandlers;
	bool mAllowStore;
	bool mStoreMode;
	bool mNotifyPresetChange;

	std::mutex mPresetChangeLock;
	std::string mPresetChangeSenderAddr;

	std::vector<std::string> mDisabledListeners;

	ParameterServer *mParameterServer;
};


}

#endif // AL_PRESET_H
