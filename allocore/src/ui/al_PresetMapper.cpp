
#include "allocore/ui/al_PresetMapper.hpp"

using namespace al;

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

PresetMapper::~PresetMapper() { }

PresetMapper &PresetMapper::registerPresetHandler(PresetHandler &handler) {
	mPresetHandler = &handler;
	if (mFindAutomatically) {
		findPresetMaps();
	}
	return *this;
}

bool PresetMapper::archive(std::string mapName, bool overwrite)
{
	bool ok = true;
	std::string fullPath = mPresetHandler->buildMapPath(mapName) + "_archive";
	if (overwrite) {
		if(File::isDirectory(fullPath)) {
			if (!Dir::removeRecursively(fullPath)) {
				std::cout << "Error removing directory: " << fullPath << " aborting preset map archiving." << std::endl;
				return false;
			}
		} else {
			if (File::remove(fullPath) != 0) {
				std::cout << "Error removing file: " << fullPath << " aborting preset map archiving." << std::endl;
				return false;
			}
		}
		if (!Dir::make(fullPath)) {
			std::cout << "Error creating preset map archive directory " << fullPath << std::endl;
			return false;
		}
	} else {
		int counter = 0;
		while (File::isDirectory(fullPath)) {
			std::string newName = mapName + "_" + std::to_string(counter++);
			fullPath = mPresetHandler->buildMapPath(newName) + "_archive";
			if (counter == 0) { // We've wrapped and run out of names...
				std::cout << "Out of names for preset map archive." << std::endl;
				return false;
			}
		}
		if (!Dir::make(fullPath)) {
			std::cout << "Error creating preset map archive directory " << fullPath << std::endl;
			return false;
		}
	}
	std::map<int, std::string> presetMap = mPresetHandler->readPresetMap(mapName);
	for(auto const &preset : presetMap) {
		std::string presetFilename = mPresetHandler->getCurrentPath() + preset.second + ".preset";
		if (!File::copy(presetFilename, fullPath)) {
			std::cout << "Error copying preset " << presetFilename << " when archiving preset map." << std::endl;
			ok = false;
		}
	}
	if (!File::copy(mPresetHandler->buildMapPath(mapName), fullPath + "default.presetMap")) {
		std::cout << "Error copying map: " << mapName << " when archiving preset map." << std::endl;
		ok = false;
	}

	return ok;
}

bool PresetMapper::load(std::string archiveName)
{
	if (! (archiveName.size() > 18 && archiveName.substr(archiveName.size() - 18) == ".presetMap_archive") ) {
		archiveName +=".presetMap_archive";
	}
	std::string mapPath = mPresetHandler->getRootPath() + archiveName;
	if (File::isDirectory(mapPath)) {
		mPresetHandler->setSubDirectory(archiveName);
		mPresetHandler->setCurrentPresetMap("default");
	} else {
		std::cout << "Error loading archive: " << archiveName << std::endl;
		return false;
	}
	return true;
}

std::vector<std::string> PresetMapper::listAvailableMaps(bool showArchives)
{
	std::vector<std::string> mapList;
	Dir mapperDir(mPresetHandler->getRootPath());

	// TODO it makes more sense to sort entries alphabetically
	while(mapperDir.read()) {
		const FileInfo &info = mapperDir.entry();
		if (info.type() == FileInfo::DIR && showArchives) {
			std::string name = info.name();
			if ( (name.size() > 18 && name.substr(name.size() - 18) == ".presetMap_archive")) {
				mapList.push_back(name);
			}
		} else if (info.type() == FileInfo::REG && !showArchives) {
			std::string name = info.name();
			if ( (name.size() > 4 && name.substr(name.size() - 4) == ".txt")
			     || (name.size() > 10 && name.substr(name.size() - 10) == ".presetMap")) {
				mapList.push_back(name);
			}
		}
	}
	return mapList;
}

bool PresetMapper::consumeMessage(osc::Message &m, std::string rootOSCPath)
{
	if(m.addressPattern() == rootOSCPath + "/map" && m.typeTags() == "s"){
		std::string val;
		m >> val;
		std::cout << "OSC: Recall preset map: " << val << std::endl;
		this->load(val);
		return true;
	}
	return false;
}

bool PresetMapper::restore(std::string mapName, bool overwrite, bool autoCreate)
{
	bool ok = true;
	if (mapName.size() > 18 && mapName.substr(mapName.size() - 18) == ".presetMap_archive") { // restore from archive
		std::cout << "Restoring archive " << mapName << std::endl;
		Dir mapDirectory(mPresetHandler->getCurrentPath() + mapName);
		std::string mapNewName = mapName.substr(0, mapName.size() - sizeof("_archive") + 1);
		while (mapDirectory.read()) {
			const FileInfo &entry = mapDirectory.entry();
			if (entry.type() == FileInfo::REG) {
				if (entry.name().substr(entry.name().size() - 7) == ".preset") {
					if (overwrite && File::exists(mPresetHandler->getCurrentPath() + "/" + entry.name())) {
						File::remove(mPresetHandler->getCurrentPath() + entry.name());
					}
					if (!File::copy(mPresetHandler->getCurrentPath() + mapName + "/" + entry.name(),
					                mPresetHandler->getCurrentPath() + entry.name())) {
						std::cout << "Error restoring preset " << entry.name() << " for " << mapName << std::endl;
						ok = false;
					}
				} else if (entry.name() == "default.presetMap") {

					if (overwrite && File::exists(mPresetHandler->getCurrentPath() + "/" + mapNewName)) {
						File::remove(mPresetHandler->getCurrentPath() + "/" + entry.name());
					}

					if (!File::copy(mPresetHandler->getCurrentPath() + "/" + mapName + "/default.presetMap",
					                mPresetHandler->getCurrentPath() + "/" + mapNewName)) {
						std::cout << "Error restoring preset map " << mapNewName << " for " << mapName << std::endl;
						ok = false;
					}
				} else {
					std::cout << "PresetMapper::restore() invalid file: " << entry.name() << std::endl;
				}
			}
		}
		mPresetHandler->setCurrentPresetMap(mapNewName, autoCreate);
	} else { // set preset map directly
		mPresetHandler->setCurrentPresetMap(mapName, autoCreate);
	}
	return ok;
}

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
