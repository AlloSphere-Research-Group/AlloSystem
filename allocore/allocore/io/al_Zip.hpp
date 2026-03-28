#ifndef INC_AL_ZIP_HPP
#define INC_AL_ZIP_HPP

/*	Allocore -- Multimedia / virtual environment application class library

	Description:
	Zip file reader/writer

	Author(s):
	Lance Putnam, 2021, putnam.lance@gmail.com
*/

#include <functional>
#include <string>
#include <vector>
#include "allocore/system/al_Pimpl.hpp"

namespace al{

/// @addtogroup allocore
/// @{

/// Read and extract contents of zip archive
class ZipReader {
public:

	ZipReader();

	/// Open archive file
	bool open(const std::string& path);

	/// Open archive from memory source

	/// \param[in] mem	Pointer to memory containing full archive
	/// \param[in] size	Size in bytes of archive
	bool open(const void * mem, int size);

	/// Extract file in archive to heap memory
	bool extract(		
		const std::string& fileName,
		const std::function<void(const void * data, int size)>& onData
	);

	/// Extract all files in archive to heap memory

	/// \returns number of files extracted
	///
	int extractAll(
		const std::function<void(const std::string& fileName, const void * data, int size)>& onData
	);


	/// Extract file in archive file to heap memory

	/// This is the simplest way to extract a single file from an archive.
	/// If extracting multiple files from the same archive, create an object
	/// and call extract for each file.
	///
	/// \param[in] zipPath	Path to archive file to extract from
	/// \param[in] fileName	Name of file in archive to extract
	/// \param[in] onData	Function called upon successful discovery and 
	///						extraction of specified file in archive.
	/// \returns whether file was extracted successfully
	static bool extract(
		const std::string& zipPath,
		const std::string& fileName,
		const std::function<void(const void * data, int size)>& onData
	);

	/// Extract all files in archive file to heap memory

	/// \param[in] zipPath	Path to archive file to extract from
	/// \param[in] onData	Function called for each file successfully extracted
	///						from archive.
	/// \returns number of files extracted
	static int extractAll(
		const std::string& zipPath,
		const std::function<void(const std::string& fileName, const void * data, int size)>& onData
	);

	/// Extract all files in archive residing in memory to heap memory

	/// \param[in] mem		Pointer to memory containing full archive
	/// \param[in] size		Size in bytes of archive
	/// \param[in] onData	Function called for each file successfully extracted
	///						from archive.
	/// \returns number of files extracted
	static int extractAll(
		const void * mem, int size,
		const std::function<void(const std::string& fileName, const void * data, int size)>& onData
	);

	/// Get file paths contained in archive
	const std::vector<std::string>& filePaths() const { return mFilePaths; }

private:
	class Impl;
	Pimpl<Impl> mImpl;

	std::vector<std::string> mFilePaths;

	bool open(const void * mem, int len, const std::string& path);
};


/// Write contents to zip archive
class ZipWriter {
public:

	ZipWriter();
	~ZipWriter();

	/// Open archive file
	bool open(const std::string& path);

	/// Finalize and close the archive
	void close();

	/// Add file to archive

	/// @param[in] filePath		Path of file on disk
	/// @param[in] zipPath		Destination file path in archive
	bool addFile(const std::string& filePath, const std::string& zipPath="");

	/// Add directory to archive
	bool addDir(const std::string& zipDir);

	/// Add memory buffer to archive
	bool addMem(const void * buf, int bytes, const std::string& zipPath="");

	/// Set compression level, in 0-10

	/// A value of 0 means no compression (fastest), a value of 10 means max 
	/// compression (slowest) and a value of -1 means default compression.
	ZipWriter& compression(int level = -1);

private:
	class Impl;
	Pimpl<Impl> mImpl;

	int mCompression = -1;
};

/// @} // end allocore group

} // al::
#endif
