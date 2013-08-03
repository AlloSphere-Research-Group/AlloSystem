#ifndef INCLUDE_AL_FILE_HPP
#define INCLUDE_AL_FILE_HPP

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
#include <list>

#include "allocore/system/al_Config.h"

#ifndef AL_FILE_DELIMITER_STR
	#ifdef AL_WINDOWS
		#define AL_FILE_DELIMITER_STR	"\\"
	#else
		#define AL_FILE_DELIMITER_STR	"/"
	#endif
#endif

#define AL_FILE_DELIMITER (AL_FILE_DELIMITER_STR[0])


#define AL_PATH_MAX (4096)

namespace al{


class FilePath;


/// File information
class FileInfo{
public:
	enum Type{
		NOFILE,				/**< No file type determined */
		REG,				/**< Regular file */
		DIR,				/**< Directory */
		CHR,				/**< Character device */
		BLOCK,				/**< A block device */
		PIPE,				/**< FIFO or pipe */
		LINK,				/**< Symbolic link */
		SOCKET,				/**< Socket */
		UNKNOWN = 127		/**< File type exists, but doesn't match any known types */
	};

	/// Set type
	FileInfo& type(Type v){ mType=v; return *this; }
	
	/// Get type
	const Type type() const { return mType; }

	/// Set name
	FileInfo& name(const std::string& v){ mName=v; return *this; }
	
	/// Get name
	const std::string& name() const { return mName; }

private:
	Type mType;
	std::string mName;
};


/// File

/// Used to retrieve data from and store data to disk.
/// The term 'path' means a file or directory.
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

	/// Write string to file
	int write(const std::string& v){ return write(v.data(), 1, v.length()); }

	/// Write memory elements to file
	int write(const void * v, int itemSizeInBytes, int items=1){
		int itemsWritten = fwrite(v, itemSizeInBytes, items, mFP);
		mSizeBytes += itemsWritten * itemSizeInBytes;
		return itemsWritten;
	}

	/// Read memory elements from file
	int read(void * v, int size, int items=1){ return fread(v, size, items, mFP); }

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

	/// Return modification time of file (or 0 on failure) as number of seconds since 00:00:00 January 1, 1970 UTC
	al_sec modified() const;

	/// Return last access time of file (or 0 on failure) as number of seconds since 00:00:00 January 1, 1970 UTC
	al_sec accessed() const;

	/// Return creation time of file (or 0 on failure) as number of seconds since 00:00:00 January 1, 1970 UTC
	al_sec created() const;

	/// Return size file (or 0 on failure)
	size_t sizeFile() const;

	/// Return space used on disk of file (or 0 on failure)
	size_t storage() const;

	FILE * filePointer() { return mFP; }


	/// Quick and dirty write memory to file
	static int write(const std::string& path, const void * v, int size, int items=1);

	/// Quick and dirty write character string to file
	static int write(const std::string& path, const std::string& data);

	/// Returns string ensured to having an ending delimiter
	
	/// The directory string argument is not checked to actually exist in
	/// the file system.
	static std::string conformDirectory(const std::string& dir);

	/// Conforms path

	/// This function takes a path as an argument and returns a new path with
	/// correct platform-specific directory delimiters, '/' or '\' and
	/// an extra delimiter at the end if the argument is a valid directory.
	static std::string conformPathToOS(const std::string& src);

	/// Convert relative paths to absolute paths
	static std::string absolutePath(const std::string& src);

	/// Returns the base name of path.

	/// The base name is everything up to the last period.
	///
	static std::string baseName(const std::string& src);

	/// Returns the directory part of path.
	
	/// The directory part of the path is everything up through (and  including)
	/// the last slash in it. If the path contains no slash, the directory part
	/// is the string ‘./’. E.g., /usr/bin/man -> /usr/bin/.
	static std::string directory(const std::string& src);

	/// Returns extension of file name.

	/// The extension is everything including and after the last period.
	/// If there is no period, an empty string is returned.
	static std::string extension(const std::string& src);


	/// Returns whether a file or directory exists
	static bool exists(const std::string& path);

	/// Returns whether a file in a directory exists
	static bool exists(const std::string& name, const std::string& path){
		return exists(path+name);
	}

	/// Returns true if path is a directory
	static bool isDirectory(const std::string& src);

	/// Search for file or directory back from current directory

	/// @param[inout] rootPath	The input should contain the path to search
	///							relative to. If the input is empty, then "./" is
	///							assumed. If a match is made, then the output is
	///							a string that can be prefixed to 'matchPath' to 
	///							get the actual location of the match.
	/// @param[in]  matchPath	File or directory to search for
	/// @param[in]  maxDepth	Maximum number of directories to search back
	/// \returns whether the file or directory was found
	static bool searchBack(std::string& rootPath, const std::string& matchPath, int maxDepth=6);

	/// Search for file or directory back from current directory

	/// @param[in,out] path		Input is a file or directory to search for.
	///							If the file is found, the output contains a series of
	///							"../" prefixed to the input. Otherwise, the input
	///							path is not modified.
	/// @param[in]  maxDepth	Maximum number of directories to search back
	/// \returns whether the file or directory was found
	static bool searchBack(std::string& path, int maxDepth=6);

	// TODO: why have these?
	static al_sec modified(const std::string& path){ return File(path).modified(); }
	static al_sec accessed(const std::string& path){ return File(path).accessed(); }
	static al_sec created (const std::string& path){ return File(path).created(); }
	static size_t sizeFile(const std::string& path){ return File(path).sizeFile(); }
	static size_t storage (const std::string& path){ return File(path).storage(); }

protected:
	class Impl; Impl * mImpl;

	std::string mPath;
	std::string mMode;
	char * mContent;
	int mSizeBytes;
	FILE * mFP;

	void dtor();
	void freeContent();
	void allocContent(int n);
	void getSize();
	
	friend class Dir;
};




/// Filesystem directory
class Dir{
public:

	/// Constructor. This does not attempt to open the directory.
	Dir();
	
	/// @param[in] dirToOpen	path to directory to open
	Dir(const std::string& dirToOpen);

	~Dir();


	/// Open a directory
	
	/// @param[in] dirPath	path to directory
	/// \returns whether the directory was successfully opened
	bool open(const std::string& dirPath);
	
	/// Close directory
	
	/// \returns whether directory was successfully closed
	///
	bool close();

	/// Read the next entry in the currently opened directory
	
	/// No ordering is guaranteed for the entries read.
	/// \returns whether there is another entry to read
	bool read();

	/// Get current directory entry (file) information
	const FileInfo& entry() const { return mEntry; }

	/// Go back to first entry in directory
	bool rewind();

private:
	class Impl; Impl * mImpl;
	FileInfo mEntry;
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
	explicit FilePath(const std::string& fullpath);


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
	
	SearchPaths(){}
	SearchPaths(const std::string& file);
	SearchPaths(int argc, char * const argv[], bool recursive=true);
	SearchPaths(const SearchPaths& cpy);

	/// find a file in the searchpaths
	FilePath find(const std::string& filename);

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
	
	void print() const;
	
	iterator begin() { return mSearchPaths.begin(); }
	iterator end() { return mSearchPaths.end(); }

protected:	
	std::list<searchpath> mSearchPaths;
	std::string mAppPath;
};


} // al::

#endif
