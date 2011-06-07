#ifndef INC_AL_UTIL_FILEWATCHER_HPP
#define INC_AL_UTIL_FILEWATCHER_HPP

#include "allocore/io/al_File.hpp"
#include "allocore/system/al_MainLoop.hpp"

#include <string>
#include <vector>

/*!
	Utilities for receiving notifications when files are updated
*/

namespace al {

/// Interface to implement for objects notified by file updates
class FileWatcher {
public:
	FileWatcher() {}
	
	/// begin filewatching:
	/// param[in] filepath: the file to watch
	/// param[in] immediate: triggers the handler immediately (if the file exists)
	FileWatcher(std::string filepath, bool immediate=true) { watch( filepath, immediate); }
	FileWatcher(File file, bool immediate=true) { watch( file.path(), immediate); }
	
	virtual ~FileWatcher();
	
	/// add a file to watch:
	/// param[in] filepath: the file to watch
	/// param[in] immediate: triggers the handler immediately (if the file exists)
	void watch(std::string filepath, bool immediate=true);
	
	/// callback when the file is modified:
	virtual void onFileWatch(File& file) = 0;
	
	/// set/get polling period (global, shared by all FileWatchers)
	static void period(al_sec p);
	static al_sec period();
};

}; // al
#endif