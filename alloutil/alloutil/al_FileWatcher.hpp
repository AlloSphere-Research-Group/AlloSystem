#ifndef INC_AL_UTIL_FILEWATCHER_HPP
#define INC_AL_UTIL_FILEWATCHER_HPP

/*	Allocore --
	Multimedia / virtual environment application class library
	
	Copyright (C) 2009. AlloSphere Research Group, Media Arts & Technology, UCSB.
	Copyright (C) 2012. The Regents of the University of California.
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