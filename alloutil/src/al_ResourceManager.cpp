#include "allocore/system/al_Printing.hpp"
#include "alloutil/al_ResourceManager.hpp"

using namespace al;


const char * ResourceManager::find(std::string filename) {
	FilePath fp = paths.find(filename);
	std::string result = fp.filepath();
	if (result == "") {
		AL_WARN("al::ResourceManager: could not find: %s", filename.c_str());
		return NULL;
	} else {
		return result.c_str();
	}

}

ResourceManager::FileInfo& ResourceManager::get(std::string filename) {
	FileMap::iterator it =  mFileMap.find(filename);
	if (it == mFileMap.end()) {
		add(filename);
	}
	return mFileMap[filename];
}

std::string ResourceManager::data(std::string filename) {
	return get(filename).data;

}

bool ResourceManager::add(std::string filename, bool immediate) {
	//FileInfo& info = mFileMap[filename];
	if (immediate) {
		return read(filename);
	}
	return false;
}

bool ResourceManager::read(std::string filename) {
	FileInfo& info = mFileMap[filename];
	if (info.path == "") {
		info.path = find(filename);
	}
	if (info.path != "" && File::exists(info.path)) {
		al_sec modified = File::modified(info.path);
		if (modified > info.modified) {
			info.modified = modified;
			printf("loaded %s\n", info.path.c_str());
			File f(info.path, "r", true);
			info.data = f.readAll();
			info.loaded = true;
			f.close();
			return true;
		}
	}
	return false;
}

bool ResourceManager::poll() {
	bool changed = 0;
	for (FileMap::iterator it=mFileMap.begin(); it!=mFileMap.end(); it++) {
		std::string name = it->first;
		changed = read(name) || changed;
	}
	return changed;
}
