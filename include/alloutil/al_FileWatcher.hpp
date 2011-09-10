#ifndef INC_AL_UTIL_FILEWATCHER_HPP
#define INC_AL_UTIL_FILEWATCHER_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2006-2008. The Regents of the University of California (REGENTS). 
	All Rights Reserved.

	Permission to use, copy, modify, distribute, and distribute modified versions
	of this software and its documentation without fee and without a signed
	licensing agreement, is hereby granted, provided that the above copyright
	notice, the list of contributors, this paragraph and the following two paragraphs 
	appear in all copies, modifications, and distributions.

	IN NO EVENT SHALL REGENTS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
	SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS, ARISING
	OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF REGENTS HAS
	BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	REGENTS SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
	PURPOSE. THE SOFTWARE AND ACCOMPANYING DOCUMENTATION, IF ANY, PROVIDED
	HEREUNDER IS PROVIDED "AS IS". REGENTS HAS  NO OBLIGATION TO PROVIDE
	MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


	File description:
	Utility for receiving notifications when files are updated

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
*/

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
	/// (affects only this FileWatcher):
	void poll();
	
	/// trigger notifications from modified files:
	/// (global, affecting all FileWatchers)
	static void pollAll();
	
	/// start/stop automatic background polling (using MainLoop):
	/// use period <= 0 to stop polling
	static void autoPoll(al_sec period);
};

}; // al
#endif