#ifndef INCLUDE_AL_RESOURCE_MANAGER_HPP
#define INCLUDE_AL_RESOURCE_MANAGER_HPP

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
	Utility for loading & watching text files

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com


*/

#include "allocore/io/al_File.hpp"
#include "allocore/system/al_Watcher.hpp"
#include "allocore/graphics/al_Shader.hpp"

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


} //al::

#endif











