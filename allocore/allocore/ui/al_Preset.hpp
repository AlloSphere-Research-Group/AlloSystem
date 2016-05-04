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

#include "allocore/system/al_Parameter.hpp"

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
	/**
	 * @brief PresetHandler contructor
	 * @param verbose if true, print diagnostic messages
	 */
	PresetHandler(std::string rootDirectory = "presets", bool verbose = false);

	PresetHandler &registerParameter(Parameter &parameter);

	void storePreset(std::string name);

	void storePreset(int index, std::string name);

	void recallPreset(std::string name);

	std::string recallPreset(int index);

	std::map<int, std::string> availablePresets();

	void setSubDirectory(std::string directory);

	std::vector<std::string> availableSubDirectories();

	typedef void (*PresetChangeCallback)(int index, void *sender,
	                                        void *userData);
	/**
	 * @brief Register a callback to be notified when a preset is loaded
	 * @param cb The callback function
	 * @param userData data to be passed to the callback
	 */
	void registerPresetCallback(PresetChangeCallback cb, void *userData = nullptr);

	PresetHandler &operator << (Parameter &param) { return this->registerParameter(param); }

private:
	std::string getCurrentPath();
	void loadPresetMap();
	void storePresetMap();

	bool mVerbose;
	std::string mRootDir;
	std::string mSubDir;
	std::string mFileName;
	std::vector<Parameter *> mParameters;
	std::mutex mFileLock;

	std::vector<PresetChangeCallback> mCallbacks;
	std::vector<void *> mCallbackUdata;

	std::map<int, std::string> mPresetsMap;
};

class PresetServer : public osc::PacketHandler, public OSCNotifier<>
{
public:
	/**
	 * @brief PresetServer constructor
	 *
	 * @param oscAddress The network address on which to listen to. If empty use all available network interfaces. Defaults to "127.0.0.1".
	 * @param oscPort The network port on which to listen. Defaults to 9010.
	 *
	 */

	PresetServer(std::string oscAddress = "127.0.0.1",
	             int oscPort = 9011);
	~PresetServer();

	virtual void onMessage(osc::Message& m);

	PresetServer &registerPresetHandler(PresetHandler &presetHandler) {
		mPresetHandler = &presetHandler;
		mPresetHandler->registerPresetCallback(PresetServer::changeCallback,
		                                       (void *) this);
	}

	/**
	 * @brief print prints information about the server to std::out
	 */
	void print();

	PresetServer &operator <<(PresetHandler &presetHandler) {return registerPresetHandler(presetHandler);}

protected:
	static void changeCallback(int value, void *sender, void *userData);

private:
	osc::Recv *mServer;
	PresetHandler *mPresetHandler;
//	std::mutex mServerLock;
	std::string mOSCpath;

};


}

#endif // AL_PRESET_H
