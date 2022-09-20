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

#include <cstdio>
#include <list>
#include <string>
#include <vector>
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

/// @addtogroup allocore
/// @{

/// File information
class FileInfo{
public:

	/// File types
	enum Type{
		REG,				/**< Regular file */
		DIR,				/**< Directory */
		CHR,				/**< Character device */
		BLOCK,				/**< A block device */
		PIPE,				/**< FIFO or pipe */
		LINK,				/**< Symbolic link */
		SOCKET,				/**< Socket */
		UNKNOWN = 127		/**< Not a known type */
	};

	/// Set type
	FileInfo& type(Type v){ mType=v; return *this; }

	/// Get type
	Type type() const { return mType; }

	/// Set name
	FileInfo& name(const std::string& v){ mName=v; return *this; }

	/// Get name
	const std::string& name() const { return mName; }

	/// Returns whether entry is a regular file
	bool isReg() const { return mType==REG; }

	/// Returns whether entry is a directory
	bool isDir() const { return mType==DIR; }

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
	/// @param[in] mode		i/o mode "w", "r", "wb", "rb"
	/// @param[in] open		whether to open the file
	File(const std::string& path=".", const std::string& mode="r", bool open=false);
	File(const FilePath& path, const std::string& mode="r", bool open=false);

	~File();


	/// Open file

	/// @param[in] path		path of file
	/// @param[in] mode		i/o mode "w", "r", "wb", "rb"
	/// \returns true on success, false otherwise
	bool open(const std::string& path, const std::string& mode="r");

	/// Open file using member variables

	/// \returns true on success, false otherwise
	///
	bool open();

	/// Close file
	void close();


	/// Set i/o mode

	/// @param[in] v	A string indicating the i/o mode.
	///
	File& mode(const std::string& v){ mMode=v; return *this; }

	/// Set path of file
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


	/// Quick and dirty read of all bytes from file
	static std::vector<char> read(const std::string& path);

	/// Quick and dirty read of all bytes from text file

	/// The result is valid for text (human-readable) files or files without
	/// byte values of zero, which will exclude many binary files.
	static std::string toString(const std::string& path);

	/// Quick and dirty write memory to file
	static int write(const std::string& path, const void * v, int size, int items=1);

	/// Quick and dirty write character string to file
	static int write(const std::string& path, const std::string& data);

	/// Copy a file in srcPath to a new location. dstPath can be the new file name or the
	/// directory where the file is copied.
	/// Returns true when sucessful.
	static bool copy(const std::string& srcPath, const std::string& dstPath, unsigned int bufferSize = 1e6);

	/// Delete file from file system
	static bool remove(const std::string& path);

	/// Returns string ensured to having an ending delimiter

	/// The directory string argument is not checked to actually exist in
	/// the file system.
	static std::string conformDirectory(const std::string& dir);

	/// Conforms path

	/// This function takes a path as an argument and returns a new path with
	/// correct platform-specific directory delimiters, '/' or '\' and
	/// an extra delimiter at the end if the argument is a valid directory.
	static std::string conformPathToOS(const std::string& path);

	/// Convert relative paths to absolute paths
	static std::string absolutePath(const std::string& path);

	/// Returns the base name of path.

	/// The base name is everything following the last slash.
	/// @param[in] path		The input path
	/// @param[in] suffix	An optional suffix to strip from the end of the base name
	static std::string baseName(const std::string& path, const std::string& suffix="");

	/// Returns the directory part of path.

	/// The directory part of the path is everything up through (and including)
	/// the last slash in it. If the path contains no slash, the directory part
	/// is the string ‘./’. E.g., /usr/bin/man -> /usr/bin/.
	static std::string directory(const std::string& path);

	/// Returns extension of file name.

	/// The extension is everything including and after the last period.
	/// If there is no period, an empty string is returned.
	static std::string extension(const std::string& path);

	/// Returns string with extension replaced

	/// @param[in] path		File path
	/// @param[in] ext		New file extension with or without a leading '.'
	static std::string replaceExtension(const std::string& path, const std::string& ext);


	/// Returns whether a file or directory exists
	static bool exists(const std::string& path);

	/// Returns whether a file in a directory exists
	static bool exists(const std::string& name, const std::string& path){
		return exists(path+name);
	}

	/// Returns true if path is a directory
	static bool isDirectory(const std::string& path);

	/// Search for file or directory back from current directory

	/// @param[in,out] rootPath	The input should contain the path to search
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

protected:
	std::string mPath;
	std::string mMode;
	char * mContent;
	int mSizeBytes;
	FILE * mFP;

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

	/// Whether to include all hidden entries in read
	Dir& all(bool v){ mHiddenPolicy=v?2:0; return *this; }

	/// Whether to include all hidden entries, except . and .. in read
	Dir& almostAll(bool v){ mHiddenPolicy=v?1:0; return *this; }


	/// Go back to first entry in directory
	bool rewind();


	struct iterator {
		typedef FileInfo value_type;
		typedef const value_type& reference;
		typedef const value_type* pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef std::ptrdiff_t difference_type;

		iterator(Dir& dir, bool more=true): mDir(dir), mMore(more){
			if(more){
				mDir.rewind();
				++(*this);
			}
		}
		iterator& operator++(){ mMore=mDir.read(); return *this;} //++it
		iterator operator++(int){ auto t=*this; ++(*this); return t;} // it++
		bool operator==(iterator other) const { return mMore == other.mMore;}
		bool operator!=(iterator other) const { return !(*this == other);}
		reference operator*(){ return mDir.entry(); }
		pointer operator->(){ return &*(*this); }
	private:
		Dir& mDir; // use ref to avoid copy
		bool mMore;
	};

	iterator begin(){ return iterator(*this); }
	iterator end(){ return iterator(*this, false); }


	/// Make a directory
	static bool make(const std::string& path, bool recursive=true);

	/// Remove a directory
	static bool remove(const std::string& path);

	/// Remove a directory recursively
	static bool removeRecursively(const std::string& path);

	/// Get the current working directory
	static std::string cwd();

	/// Get OS-specific application data directory
	static std::string appData();

	/// Get OS-specific per-user application data directory
	static std::string userAppData();

	/// Get current user home directory
	static std::string home();

private:
	class Impl; Impl * mImpl;
	FileInfo mEntry;
	int mHiddenPolicy = 0;
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



/// Keeps a list of files
class FileList {
public:
	typedef std::vector<FilePath>::iterator iterator;

	FileList():indx(0){}

	/// return currently selected file in list
	FilePath& operator()(){ return mFiles[indx]; }

	/// find a file in list
	// FilePath& select(const std::string& filename);

	FilePath& select(int i){ indx=i%count(); return (*this)(); }
	FilePath& next(){ ++indx %= count(); return (*this)(); }
	FilePath& prev(){ --indx; if(indx < 0) indx = count()-1; return (*this)(); }

	int count(){ return mFiles.size(); }

	void print() const;

	FilePath& operator[](int i){ return mFiles[i]; }
	iterator begin() { return mFiles.begin(); }
	iterator end() { return mFiles.end(); }

	void add(const FilePath& fp){ mFiles.push_back(fp); }
	void sort(bool (*f)(FilePath,FilePath));

protected:
	int indx;
	std::vector<FilePath> mFiles;
};



/// A handy way to manage several possible search paths
class SearchPaths {
public:
	typedef std::pair<std::string, bool> SearchPath;
	typedef std::list<SearchPath> SearchPathList;
	typedef std::list<SearchPath>::iterator iterator;

	SearchPaths(){}
	SearchPaths(const std::string& file);
	SearchPaths(int argc, char * const argv[], bool recursive=true);
	SearchPaths(const SearchPaths& cpy);

	/// find a file in the searchpaths
	FilePath find(const std::string& filename) const;
	FileList glob(const std::string& regex) const;

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
	std::list<SearchPath> mSearchPaths;
	std::string mAppPath;
};

/// @} // end allocore group

} // al::

#endif
