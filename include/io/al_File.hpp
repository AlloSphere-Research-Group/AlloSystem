#ifndef INCLUDE_AL_FILE_HPP
#define INCLUDE_AL_FILE_HPP

#include <unistd.h>
#include <stdio.h>
#include <string>
#include <sys/stat.h>
#include <list>

#include "system/al_Config.h"

#ifdef AL_WIN32 
	#define AL_FILE_DELIMITER '\\'
#else
	#define AL_FILE_DELIMITER '/'
#endif	

#define AL_PATH_MAX (4096)

namespace al{

/// Strips a qualified path to a file (src) into a path to the containing folder (dst)
void path2dir(char* dst, const char* src);

/// a pair of path (folder/directory) and filename
class FilePath {
public:
	FilePath() {};
	FilePath(std::string file, std::string path = "/") : mPath(path), mFile(file) {}
	
	const std::string& file() const { return mFile; }
	const std::string& path() const { return mPath; }
	
	std::string filepath() const { return path()+file(); }
	
	FilePath& file(const std::string& v) { mFile=v; return *this; }
	FilePath& path(const std::string& v) { mPath=v; return *this; }
	
protected:
	std::string mPath;
	std::string mFile;
};

/// A handy way to manage several possible search paths
class SearchPaths {
public:
	SearchPaths() {}
	SearchPaths(int argc, char * const argv[], bool recursive=true) { addAppPaths(argc,argv,recursive); }
	~SearchPaths() {}

	/// find a file in the searchpaths
	/// returns true if file found, and fills result with corresponding path & filename
	/// returns false if file not found
	FilePath find(const std::string& filename);
	
	/// add a path to search in; recursive searching is optional
	void addSearchPath(const std::string& path, bool recursive = true);
	
	/// adds best estimate of application launch paths (cwd etc.)
	/// can pass in argv from the main() function if desired.
	void addAppPaths(int argc, char * const argv[], bool recursive = true);
	void addAppPaths(bool recursive = true);
	
	const std::string& appPath() const { return mAppPath; }
	
	/// todo?
	//void addResourcePath();
	
	// strips trailing filename from a path; e.g. /usr/bin/man -> /usr/bin/
	static std::string stripFileName(const std::string& src);
	// ensure path ends with the proper delimiter
	static std::string conformPath(const std::string& src);
	// does a file at the given filepath exist?
	static bool fileExists(const std::string& name, const std::string& path);
	
protected:

	typedef std::pair<std::string, bool> searchpath;
	std::list<searchpath> mSearchPaths;
	
	std::string mAppPath;

};


/// File
class File{
public:

	/// @param[in] path		path of file
	/// @param[in] mode		i/o mode w, r, wb, rb
	File(const char * path, const char * mode, bool open=false);

	~File();

	void close();	///< Close file
	bool open();	///< Open file with specified i/o mode

	File& mode(const char * v){ mMode=v; return *this; }
	File& path(const char * v){ mPath=v; return *this; }

	/// Write memory elements to file
	int write(const void * v, int size, int items=1){ return fwrite(v, size, items, mFP); }

	/// Quick and dirty write memory to file
	static int write(const char * path, const void * v, int size, int items=1);

	/// Returns character string of file contents (read mode only)
	char * readAll();

	/// Returns whether file is open
	bool opened() const { return 0 != mFP; }
	
	/// Returns file i/o mode string
	const char * mode() const { return mMode; }
	
	/// Returns path string
	const char * path() const { return mPath; }
	
	/// Returns size (in bytes) of file contents
	int size() const { return mSizeBytes; }

	/// Returns whether file exists
	static bool exists(const char * path);

protected:
	const char * mPath;
	const char * mMode;
	char * mContent;
	int mSizeBytes;
	FILE * mFP;
	
	void freeContent();
	void allocContent(int n);
	void getSize();
};



//// INLINE IMPLEMENTATION ////

inline std::string SearchPaths::stripFileName(const std::string& src) {
	std::string filepath(src);
	size_t pos = filepath.find_last_of(AL_FILE_DELIMITER);
	if (pos !=std::string::npos) {
		filepath.erase(pos+1);
	}
	return filepath;
}

inline std::string SearchPaths::conformPath(const std::string& src) {
	std::string path(src);
	// paths should end with a delimiter:
	if (path[path.size()-1] != AL_FILE_DELIMITER) {
		path += AL_FILE_DELIMITER;
	}
	return path;
}

inline void SearchPaths::addAppPaths(int argc, char * const argv[], bool recursive) {
	addAppPaths(recursive);
	if (argc > 0) {
		//char path[4096];
		std::string filepath = stripFileName(argv[0]);
		mAppPath = filepath;
		addSearchPath(filepath, recursive);
	}
}

inline void SearchPaths::addAppPaths(bool recursive) {	
	char cwd[4096];
	getcwd(cwd, sizeof(cwd));
	addSearchPath(cwd, recursive);
}

inline void SearchPaths::addSearchPath(const std::string& src, bool recursive) {
	std::string path=conformPath(src);
	
	// check for duplicates
	std::list<searchpath>::iterator iter = mSearchPaths.begin();
	while (iter != mSearchPaths.end()) {
		//printf("path %s\n", iter->first.data());
		if (path == iter->first.data()) {
			return;
		}
		iter++;
	}
	printf("adding path %s\n", path.data());
	mSearchPaths.push_front(searchpath(path, recursive));
}

inline bool SearchPaths::fileExists(const std::string& path, const std::string& name) {
	struct stat stFileInfo; 
	std::string filename(path+name);
	return (stat((path + name).c_str(),&stFileInfo) == 0);
}

} // al::

#endif

