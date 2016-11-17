#ifndef AL_PRESETMAPPER_H
#define AL_PRESETMAPPER_H

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
	Preset mapping: Organizing preset groups

	File author(s):
	Andrés Cabrera mantaraya36@gmail.com
*/

#include <vector>
#include <string>

#include "allocore/ui/al_Preset.hpp"
#include "allocore/io/al_File.hpp"

namespace al
{

class PresetMapper
{
public:
	PresetMapper(bool findAutomatically = true) :
	    mFindAutomatically(findAutomatically)
	{
	}

	virtual ~PresetMapper()
	{

	}

	PresetMapper &registerPresetHandler(PresetHandler &handler) {
		mPresetHandler = &handler;
		if (mFindAutomatically) {
			findPresetMaps();
		}
	}


	std::vector<std::string> listAvailableMaps();

private:

	void findPresetMaps();
	void readEntries(std::string path);

	bool mFindAutomatically;
	PresetHandler *mPresetHandler;
	std::vector<std::string> mPresetMapPaths; // full path to the preset map file (including preset map name e.g. _presetMap.txt)
};

void PresetMapper::findPresetMaps() {
	std::string presetsPath = mPresetHandler->getCurrentPath();

	Dir presetDir(presetsPath);
	while (presetDir.read()) {
		FileInfo entry = presetDir.entry();
		if (entry.name().size() > 4 && entry.name().substr(entry.name().size() - 4) == ".txt") {
			std::cout << "Found map: " << entry.name() << std::endl;
		}
	}
}

void PresetMapper::readEntries(std::string path)
{
	Dir presetDir(path);
	while (presetDir.read()) {
		FileInfo entry = presetDir.entry();
		if (entry.type() == FileInfo::DIR) {
			readEntries(entry.name());
		} else if (entry.type() == FileInfo::REG) {
			if (entry.name().size() > 4 && entry.name().substr(entry.name().size() - 4) == ".txt") {
				std::cout << "Found map: " << entry.name();
			}
		}
	}
}




} // namespace al

#endif // AL_PRESETMAPPER_H
