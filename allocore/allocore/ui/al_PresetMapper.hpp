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

#include "allocore/protocol/al_OSC.hpp"
#include "allocore/ui/al_Preset.hpp"
#include "allocore/io/al_File.hpp"

namespace al
{

/**
 * @brief The PresetMapper class allows archiving and recalling preset maps
 *
 * Preset maps are saved as a directory containing the presets as well as a
 * preset map file that lists the presets in the map as well as assigning
 * an index to each.
 *
 * The preset map file is a text file, where each line looks like:
 *
 * @code
 * 0:preset_name
 * @endcode
 *
 * The index of the preset is separated by the preset name by a semi-colon. The
 * preset name should correspond to a file on disk called "preset_name.preset".
 * These files are located either within a preset map archive or a PresetHandler's
 * root directory.
 *
 * To archive a preset map, it's necessary to first register a PresetHandler
 * with registerPresetHandler() and then call archive().
 */
class PresetMapper : public osc::MessageConsumer
{
public:
	PresetMapper(bool findAutomatically = true) :
	    mFindAutomatically(findAutomatically)
	{ }

	virtual ~PresetMapper();

	PresetMapper &registerPresetHandler(PresetHandler &handler);

	bool archive(std::string mapName = "default", bool overwrite = true);

	bool load(std::string mapName);

	/// Restore a preset map from a preset map archive directory
	/// This copies all the files back to the preset root directory
	/// If you don't want this copy use load()
	bool restore(std::string mapName = "default", bool overwrite = true, bool autoCreate = false);

	/// Return a list with the name of availble preset maps for use in restore() or archive().
	/// if listArchives is true, only archive directories are listed, otherwise only preset map
	/// files are.
	std::vector<std::string> listAvailableMaps(bool listArchives = true);

	virtual bool consumeMessage(osc::Message &m, std::string rootOSCPath) override;

private:

	void findPresetMaps();
	void readEntries(std::string path);

	bool mFindAutomatically;
	PresetHandler *mPresetHandler;
};


} // namespace al

#endif // AL_PRESETMAPPER_H
