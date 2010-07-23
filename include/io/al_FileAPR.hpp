#ifndef INCLUDE_AL_FILE_APR_HPP
#define INCLUDE_AL_FILE_APR_HPP

#include <stdio.h>

namespace al{

class FileAPR{
public:

	/// @param[in] path		path of file
	/// @param[in] mode		i/o mode w, r, wb, rb
	FileAPR(const char * path, const char * mode, bool open=false);

	~FileAPR();

	void close();	///< Close file
	bool open();	///< Open file with specified i/o mode

	void mode(const char * v){ mMode=v; }

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



} // al::

#endif

