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
	void poll();
	
	
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
	
	ManagedShader(ResourceManager& rm, std::string vert, std::string frag) 
	:	ShaderProgram(), 
		rm(rm), 
		vertName(vert), 
		fragName(frag), 
		vertModified(0), 
		fragModified(0)
	{
		rm.add(vert, false);
		rm.add(frag, false);
	}
	
	virtual ~ManagedShader() {}
	
	// overrides ShaderProgram::begin():
	void begin() {
		// update shader sources:
		ResourceManager::FileInfo& v = rm[vertName];
		ResourceManager::FileInfo& f = rm[fragName];
		if (v.modified > vertModified || f.modified > fragModified) {
			// mark as read:
			vertModified = v.modified;
			fragModified = f.modified;
			// remove existing:
			detach(vert).detach(frag);
			// recompile & link:
			vert.source(v.data, Shader::VERTEX).compile();
			frag.source(f.data, Shader::FRAGMENT).compile();
			attach(vert).attach(frag).link();
			vert.printLog();
			frag.printLog();
			printLog();			
		}
		ShaderProgram::begin();
	}
	
	ResourceManager& rm;
	al_sec vertModified, fragModified;
	std::string vertName, fragName;
	Shader vert, frag;
};


} //al::

#endif











