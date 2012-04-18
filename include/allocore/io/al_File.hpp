#ifndef INCLUDE_AL_FILE_HPP
#define INCLUDE_AL_FILE_HPP

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
	Utilities for file management

	File author(s):
	Graham Wakefield, 2010, grrrwaaa@gmail.com
	Lance Putnam, 2010, putnam.lance@gmail.com
*/


#include <limits.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <list>

#include "allocore/system/al_Config.h"

#ifdef AL_WIN32
	#define AL_FILE_DELIMITER		'\\'
	#define AL_FILE_DELIMITER_STR	"\\"
#else
	#define AL_FILE_DELIMITER		'/'
	#define AL_FILE_DELIMITER_STR	"/"
#endif

#define AL_PATH_MAX (4096)

namespace al{


class FilePath;


/// File
class File{
public:

	/// @param[in] path		path of file
	/// @param[in] mode		i/o mode w, r, wb, rb
	/// @param[in] open		whether to open the file
	File(const std::string& path, const std::string& mode="r", bool open=false);
	File(const FilePath& path, const std::string& mode="r", bool open=false);

	~File();

	void close();	///< Close file
	bool open();	///< Open file with specified i/o mode

	File& mode(const std::string& v){ mMode=v; return *this; }
	File& path(const std::string& v){ mPath=v; return *this; }

	/// Write memory elements to file
	int write(const std::string& v){ return write(v.data(), 1, v.length()); }
	int write(const void * v, int itemSizeInBytes, int items=1){
		int itemsWritten = fwrite(v, itemSizeInBytes, items, mFP);
		mSizeBytes += itemsWritten * itemSizeInBytes;
		return itemsWritten;
	}

	/// Read memory elements from file
	int read(void * v, int size, int items=1){ return fread(v, size, items, mFP); }

	/// Quick and dirty write memory to file
	static int write(const std::string& path, const void * v, int size, int items=1);

	/// Returns character string of file contents (read mode only)
	const char * readAll();

	/// Returns whether file is open
	bool opened() const { return 0 != mFP; }

	/// Returns file i/o mode string
	const std::string& mode() const { return mMode; }

	/// Returns path string
	const std::string& path() const { return mPath; }

	/// Returns size, in bytes, of file contents
	int size() const { return mSizeBytes; }

	/// Return modification time of file (or 0 on failure) as number of seconds since 00:00:00 january 1, 1970 UTC
	al_sec modified() const;

	/// Return last access time of file (or 0 on failure) as number of seconds since 00:00:00 january 1, 1970 UTC
	al_sec accessed() const;

	/// Return creation time of file (or 0 on failure) as number of seconds since 00:00:00 january 1, 1970 UTC
	al_sec created() const;

	/// Return size file (or 0 on failure)
	size_t sizeFile() const;

	/// Return space used on disk of file (or 0 on failure)
	size_t storage() const;

	FILE * filePointer() { return mFP; }


	/// Ensure path ends with the proper delimiter
	static std::string conformPath(const std::string& src);
	
	/// Convert relative paths to absolute paths
	static std::string absolutePath(const std::string& src);

	/// Extracts the directory-part of file name.
	
	/// The directory-part of the file name is everything up through (and 
	/// including) the last slash in it. If the file name contains no slash, 
	/// the directory part is the string ‘./’. E.g., /usr/bin/man -> /usr/bin/.
	static std::string directory(const std::string& src);

	/// Returns whether a file or directory exists
	static bool exists(const std::string& path);

	/// Returns whether a file in a directory exists
	static bool exists(const std::string& name, const std::string& path){
		return exists(path+name);
	}

	/// Search for file or directory back from current directory

	/// @param[out] prefixPath	If the file is found, this contains a series of
	///							"../" that can be prefixed to 'matchPath' to get
	///							its actual location.
	/// @param[in]  matchPath	File or directory to search for
	/// @param[in]  maxDepth	Maximum number of directories to search back
	/// \returns whether the file or directory was found
	static bool searchBack(std::string& prefixPath, const std::string& matchPath, int maxDepth=6);

	/// Search for file or directory back from current directory

	/// @param[in,out] path		Input is a file or directory to search for.
	///							If the file is found, the output contains a series of
	///							"../" prefixed to the input. Otherwise, the input
	///							path is not modified.
	/// @param[in]  maxDepth	Maximum number of directories to search back
	/// \returns whether the file or directory was found
	static bool searchBack(std::string& path, int maxDepth=6){
		std::string prefix = "";
		bool r = searchBack(prefix, path);
		if(r) path = prefix + path;
		return r;
	}

	static al_sec modified(std::string path) { File f(path); return f.modified(); }
	static al_sec accessed(std::string path) { File f(path); return f.accessed(); }
	static al_sec created(std::string path) { File f(path); return f.created(); }
	static size_t sizeFile(std::string path) { File f(path); return f.sizeFile(); }
	static size_t storage(std::string path) { File f(path); return f.storage(); }

protected:
	class Impl; Impl * mImpl;

	std::string mPath;
	std::string mMode;
	char * mContent;
	int mSizeBytes;
	FILE * mFP;

	void freeContent();
	void allocContent(int n);
	void getSize();
};



/// A pair of path (folder/directory) and file name
class FilePath {
public:
	FilePath(){}

	/// @param[in] file			File name without directory
	/// @param[in] path			Directory of file
	FilePath(const std::string& file, const std::string& path)
	:	mPath(path), mFile(file) {}

	/// @param[in] fullpath		Full path to file (directory + file name)
	explicit FilePath(std::string fullpath);


	/// Get file name without directory
	const std::string& file() const { return mFile; }
	
	/// Get path (directory) of file
	const std::string& path() const { return mPath; }

	/// Get file with directory
	std::string filepath() const { return path()+file(); }
	
	/// Returns whether file part is valid
	bool valid() const { return file()!=""; }


	/// Set file name without directory
	FilePath& file(const std::string& v) { mFile=v; return *this; }
	
	/// Set path (directory) of file
	FilePath& path(const std::string& v) { mPath=v; return *this; }

protected:
	std::string mPath;
	std::string mFile;
};



/// A handy way to manage several possible search paths
class SearchPaths {
public:
	typedef std::pair<std::string, bool> searchpath;
	typedef std::list<searchpath> searchpathlist;
	typedef std::list<searchpath>::iterator iterator;
	
	SearchPaths() {}
	SearchPaths(std::string file) {
		FilePath fp(file);
		addAppPaths(fp.path());
	}
	SearchPaths(int argc, char * const argv[], bool recursive=true) { addAppPaths(argc,argv,recursive); }
	SearchPaths(const SearchPaths& cpy) 
	:	mSearchPaths(cpy.mSearchPaths),
		mAppPath(cpy.mAppPath)
	{}
	~SearchPaths() {}

	/// find a file in the searchpaths
	FilePath find(const std::string& filename) ;

	/// add a path to search in; recursive searching is optional
	void addSearchPath(const std::string& path, bool recursive = true);
	void addRelativePath(std::string rel, bool recursive=true) {
		addSearchPath(appPath() + rel, recursive);
	}

	/// adds best estimate of application launch paths (cwd etc.)
	/// can pass in argv from the main() function if desired.
	void addAppPaths(int argc, char * const argv[], bool recursive = true);
	void addAppPaths(int argc, const char ** argv, bool recursive = true);
	void addAppPaths(std::string path, bool recursive = true);
	void addAppPaths(bool recursive = true);

	const std::string& appPath() const { return mAppPath; }
	
	void print();
	
	iterator begin() { return mSearchPaths.begin(); }
	iterator end() { return mSearchPaths.end(); }

protected:	
	std::list<searchpath> mSearchPaths;
	std::string mAppPath;
};







//// INLINE IMPLEMENTATION ////

inline std::string File::conformPath(const std::string& src) {
	std::string path(src);
	// paths should end with a delimiter:
	if (path[path.size()-1] != AL_FILE_DELIMITER) {
		path += AL_FILE_DELIMITER;
	}
	return path;
}

inline std::string File::absolutePath(const std::string& src) {
	char temp[PATH_MAX];
	char * result = realpath(src.c_str(), temp);
	return result ? result : "";
}

inline std::string File::directory(const std::string& src){
	size_t pos = src.find_last_of(AL_FILE_DELIMITER);
	if(std::string::npos != pos){
		return src.substr(0, pos+1);
	}
	return "."AL_FILE_DELIMITER_STR;
}

inline bool File::exists(const std::string& path){
	struct stat s;
	return ::stat(path.c_str(), &s) == 0;
}

inline bool File::searchBack(std::string& prefixPath, const std::string& matchPath, int maxDepth){
	int i=0;
	prefixPath="";

	for(; i<maxDepth; ++i){
		if(File::exists(prefixPath + matchPath)) break;
		prefixPath = ".."AL_FILE_DELIMITER_STR + prefixPath;
	}
	return i<maxDepth;
}

inline void SearchPaths::addAppPaths(std::string path, bool recursive) {
	std::string filepath = File::directory(path);
	mAppPath = filepath;
	addSearchPath(filepath, recursive);
}

inline void SearchPaths::addAppPaths(int argc, const char ** argv, bool recursive) {
	addAppPaths(recursive);
	if (argc > 0) {
		addAppPaths(File::directory(argv[0]), recursive);
	} 
}

inline void SearchPaths::addAppPaths(int argc, char * const argv[], bool recursive) {
	addAppPaths(recursive);
	if (argc > 0) {
		addAppPaths(File::directory(argv[0]), recursive);
	} 
}

inline void SearchPaths::addAppPaths(bool recursive) {
	char cwd[4096];
	if(getcwd(cwd, sizeof(cwd))){
		mAppPath = std::string(cwd) + "/";
		addSearchPath(mAppPath, recursive);
	}
}

inline void SearchPaths::addSearchPath(const std::string& src, bool recursive) {
	std::string path=File::conformPath(src);

	// check for duplicates
	std::list<searchpath>::iterator iter = mSearchPaths.begin();
	while (iter != mSearchPaths.end()) {
		//printf("path %s\n", iter->first.c_str());
		if (path == iter->first) {
			return;
		}
		iter++;
	}
//	printf("adding path %s\n", path.data());
	mSearchPaths.push_front(searchpath(path, recursive));
}

inline void SearchPaths::print() {
	printf("SearchPath %p appPath: %s\n", this, appPath().c_str());
	std::list<searchpath>::iterator it = mSearchPaths.begin();
	while (it != mSearchPaths.end()) {
		SearchPaths::searchpath sp = (*it++);
		printf("SearchPath %p path: %s (recursive: %d)\n", this, sp.first.c_str(), sp.second);
	}
}

} // al::

#endif

