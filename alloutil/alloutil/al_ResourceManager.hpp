#ifndef INCLUDE_AL_RESOURCE_MANAGER_HPP
#define INCLUDE_AL_RESOURCE_MANAGER_HPP

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
	Utility for loading & watching text files

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com


*/

#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Watcher.hpp"
#include "allocore/graphics/al_Shader.hpp"
#include "alloutil/al_Lua.hpp"

#include <map>
#include <set>
#include <list>

namespace al {



class ResourceManager {
public:
	struct FileInfo {
		std::string path;
		std::string data;
		al_sec modified;
		bool loaded;	// flag signals when file has been read

		FileInfo() : modified(0) {}
		FileInfo(const FileInfo& cpy) : path(cpy.path), modified(cpy.modified) {}
	};

	///! returns NULL if the file cannot be found
	const char * find(std::string filename);

	///! adds a file to the file map (assuming it can be found
	/// with immediate==true also calls read() directly
	bool add(std::string filename, bool immediate=true);

	/// get the FileInfo for a given fileanme
	/// (calls addFile(fileanme) if the file isn't currently in the file map)
	FileInfo& get(std::string filename);
	FileInfo& operator[](std::string filename) { return get(filename); }

	/// get the data for a given filename
	/// (calls addFile(fileanme) if the file isn't currently in the file map)
	std::string data(std::string filename);

	///! updates the modified/changed flags of all files in the filemap:
	/// returns true if any of them changed
	bool poll();


	///! list of paths to search for files:
	SearchPaths paths;

protected:
	bool read(std::string filename);

	///! map of filenames to FileInfo structures:
	typedef std::map<std::string, FileInfo> FileMap;
	FileMap mFileMap;
};


/// Example use-case with shaders:
class ManagedShader : public ShaderProgram {
public:

	ManagedShader(ResourceManager& rm, std::string vert="", std::string frag="")
	:	ShaderProgram(),
		rm(rm),
		relink(0)
	{
		vertex(vert);
		fragment(frag);
	}

	virtual ~ManagedShader() {}

	ManagedShader& vertex(std::string name) { return add(name, Shader::VERTEX); }
	ManagedShader& fragment(std::string name) { return add(name, Shader::FRAGMENT); }
	ManagedShader& geometry(std::string name) { return add(name, Shader::GEOMETRY); }

	ManagedShader& add(std::string name, Shader::Type type) {
		if (name != "") {
			shaders.push_back(ShaderFile());
			ShaderFile& last = shaders.back();
			last.name = name;
			last.type = type;
			last.modified = 0;
			relink = true;
		}
		return *this;
	}

	// overrides ShaderProgram::begin():
	void begin() {
		// check all shaders:
		for (unsigned i=0; i<shaders.size(); i++) {
			ShaderFile& sf = shaders[i];
			ResourceManager::FileInfo& info = rm[sf.name];
			if (info.loaded) {
				// mark as read:
				sf.modified = info.modified;
				info.loaded = 0;
				// remove existing:
				if (info.modified != 0) detach(sf.shader);
				// recompile & link:
				sf.shader.source(info.data, sf.type).compile();
				attach(sf.shader);
				sf.shader.printLog();
				// mark needs re-link:
				relink = true;
			}
		}
		if (relink) {
			link(false);
			printLog();
			relink = false;
		}
		ShaderProgram::begin();
	}

protected:

	virtual void onCreate() {
		ShaderProgram::onCreate();

		for (unsigned i=0; i<shaders.size(); i++) {
			ShaderFile& sf = shaders[i];
			sf.modified = 0;    // trigger reload
		}

	}

	virtual void onDestroy() {
		ShaderProgram::onDestroy();

		for (unsigned i=0; i<shaders.size(); i++) {
			ShaderFile& sf = shaders[i];
			sf.shader.invalidate();
		}
	}

	struct ShaderFile {
		std::string name;
		Shader shader;
		Shader::Type type;
		al_sec modified;
	};

	ResourceManager& rm;
	std::vector<ShaderFile> shaders;
	bool relink;
};

class ManagedLuaFile {
public:
	ManagedLuaFile(ResourceManager& rm) : rm(rm) {}
	
	void watch(const std::string& filename) {
		files.push_back(filename);
		rm.add(filename);
	}	
	
	void poll(Lua& L) {
		for (unsigned i=0; i<files.size(); i++) {
			const std::string& file = files[i];
			ResourceManager::FileInfo& info = rm[file];
			if (info.loaded) {
				info.loaded = 0;
				printf("running %s\n", info.path.c_str());
				L.dofile(info.path);
			}
		}
	}

	ResourceManager& rm;
	std::vector<std::string> files;
};

} //al::

#endif











