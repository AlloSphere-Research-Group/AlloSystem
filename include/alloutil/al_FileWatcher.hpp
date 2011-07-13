#ifndef INC_AL_UTIL_FILEWATCHER_HPP
#define INC_AL_UTIL_FILEWATCHER_HPP

#include "allocore/io/al_File.hpp"
#include "allocore/system/al_MainLoop.hpp"

#include <string>
#include <vector>

/*!
	Utility for receiving notifications when files are updated
	
	Sub-class FileWatcher and implement the onFileWatch() method.
	Register for notifications of files using the watch() method(s)
*/

namespace al {

/// Interface to implement for objects notified by file updates
class FileWatcher {
public:
	FileWatcher() {}
	
	/// begin filewatching:
	/// param[in] filepath: the file to watch
	/// param[in] immediate: triggers the handler immediately (if the file exists)
	template<typename T>
	FileWatcher(T f, bool immediate=true) { watch( f, immediate); }
	
	virtual ~FileWatcher();
	
	/// add a file to watch:
	/// param[in] filepath: the file to watch
	/// param[in] immediate: triggers the handler immediately (if the file exists)
	void watch(std::string filepath, bool immediate=true);
	void watch(FilePath f, bool immediate=true) { watch(f.filepath(), immediate); }
	void watch(File f, bool immediate=true) { watch( f.path(), immediate); }
	
	/// callback when the file is modified:
	virtual void onFileWatch(File& file) = 0;
	
	/// trigger notifications from modified files:
	/// (global, affecting all FileWatchers)
	static void poll();
	
	/// start/stop automatic background polling (using MainLoop):
	/// use period <= 0 to stop polling
	static void autoPoll(al_sec period);
};

}; // al
#endif